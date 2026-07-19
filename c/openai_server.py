#!/usr/bin/env python3
"""Dependency-free OpenAI-compatible HTTP gateway for the colibri engine."""

import argparse
import codecs
import collections
import contextlib
import json
import os
import select
import signal
import socket
import subprocess
import sys
import threading
import time
import uuid
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from urllib.parse import unquote, urlsplit

from resource_plan import detect_model_family


HERE = Path(__file__).resolve().parent
END = b"\x01\x01END\x01\x01\n"
READY = b"\x01\x01READY\x01\x01\n"
MAX_BODY = 4 << 20


def detect_engine_for_model(model_path, requested_engine=None):
    if requested_engine not in (None, ""):
        return Path(requested_engine)
    config_path = Path(model_path) / "config.json"
    if not config_path.is_file():
        return HERE / "glm"
    try:
        payload = json.loads(config_path.read_text())
    except (OSError, json.JSONDecodeError, TypeError):
        return HERE / "glm"
    if not isinstance(payload, dict):
        return HERE / "glm"
    if detect_model_family(payload) == "qwen35":
        return HERE / "qwen35_moe"
    return HERE / "glm"
DEFAULT_CORS_ORIGINS = (
    "http://127.0.0.1:5173",
    "http://localhost:5173",
    "http://tauri.localhost",
    "tauri://localhost",
)


class APIError(Exception):
    def __init__(self, status, message, param=None, code=None, error_type="invalid_request_error",
                 headers=None):
        super().__init__(message)
        self.status = status
        self.message = message
        self.param = param
        self.code = code
        self.error_type = error_type
        self.headers = headers or {}


class ClientCancelled(Exception):
    pass


def error_object(error):
    return {"error": {"message": error.message, "type": error.error_type,
                      "param": error.param, "code": error.code}}


class GenerationScheduler:
    """Bounded FIFO admission for the engine's single mutable KV context."""

    def __init__(self, max_queue=8, queue_timeout=300):
        if max_queue < 0:
            raise ValueError("max_queue cannot be negative")
        if queue_timeout <= 0:
            raise ValueError("queue_timeout must be positive")
        self.max_queue = max_queue
        self.queue_timeout = queue_timeout
        self.condition = threading.Condition()
        self.queue = collections.deque()
        self.active = False
        self.closed = False
        self.admitted = 0
        self.completed = 0
        self.rejected = 0
        self.timed_out = 0
        self.cancelled = 0

    @contextlib.contextmanager
    def admit(self, cancelled=None):
        ticket = object()
        queued_at = time.monotonic()
        with self.condition:
            if self.closed:
                raise APIError(503, "The inference scheduler is shutting down.", None,
                               "scheduler_closed", "server_error")
            if (self.active or self.queue) and len(self.queue) >= self.max_queue:
                self.rejected += 1
                raise APIError(429, "The inference queue is full.", None, "queue_full",
                               "rate_limit_error", {"Retry-After": "1"})
            self.queue.append(ticket)
            deadline = queued_at + self.queue_timeout
            while True:
                if self.closed:
                    self.queue.remove(ticket)
                    self.condition.notify_all()
                    raise APIError(503, "The inference scheduler is shutting down.", None,
                                   "scheduler_closed", "server_error")
                if not self.active and self.queue[0] is ticket:
                    break
                if cancelled and cancelled():
                    self.queue.remove(ticket)
                    self.cancelled += 1
                    self.condition.notify_all()
                    raise ClientCancelled()
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    self.queue.remove(ticket)
                    self.timed_out += 1
                    self.condition.notify_all()
                    raise APIError(429, "Timed out waiting for the inference engine.", None,
                                   "queue_timeout", "rate_limit_error", {"Retry-After": "1"})
                self.condition.wait(min(remaining, 0.25))
            self.queue.popleft()
            self.active = True
            self.admitted += 1
            wait_seconds = time.monotonic() - queued_at
        try:
            yield wait_seconds
        finally:
            with self.condition:
                self.active = False
                self.completed += 1
                self.condition.notify_all()

    def snapshot(self):
        with self.condition:
            return {"active": self.active, "queued": len(self.queue),
                    "max_queue": self.max_queue, "queue_timeout_seconds": self.queue_timeout,
                    "admitted": self.admitted, "completed": self.completed,
                    "rejected": self.rejected, "timed_out": self.timed_out,
                    "cancelled": self.cancelled}

    def close(self):
        with self.condition:
            self.closed = True
            self.condition.notify_all()


DEFAULT_LOGPROBS_COUNT = 5
MOCK_LOGPROB = -0.6931471805599453


def content_text(content, param):
    if isinstance(content, str):
        return content
    if not isinstance(content, list):
        raise APIError(400, "Message content must be a string or an array of text parts.", param)
    parts = []
    for index, part in enumerate(content):
        if not isinstance(part, dict):
            raise APIError(400, "Content parts must be objects.", f"{param}.{index}")
        part_type = part.get("type")
        if part_type in ("text", "input_text"):
            text = part.get("text")
            if not isinstance(text, str):
                raise APIError(400, "Text content parts require a string `text` field.",
                               f"{param}.{index}.text")
            parts.append(text)
            continue
        if part_type in ("image_url", "image"):
            url = part.get("image_url")
            if isinstance(url, dict):
                url = url.get("url")
            elif not isinstance(url, str):
                url = part.get("url")
                if isinstance(url, dict):
                    url = url.get("url")
            if not isinstance(url, str):
                raise APIError(400, "Image content parts require a string `image_url` or `url` field.",
                               f"{param}.{index}.image_url")
            parts.append("[image]")
            continue
        if part_type in ("video_url", "video"):
            url = part.get("video_url")
            if isinstance(url, dict):
                url = url.get("url")
            elif not isinstance(url, str):
                url = part.get("url")
                if isinstance(url, dict):
                    url = url.get("url")
            if not isinstance(url, str):
                raise APIError(400, "Video content parts require a string `video_url` or `url` field.",
                               f"{param}.{index}.video_url")
            parts.append("[video]")
            continue
        if part_type in ("audio_url", "audio", "input_audio"):
            url = part.get("audio_url")
            if isinstance(url, dict):
                url = url.get("url")
            elif not isinstance(url, str):
                url = part.get("url")
                if isinstance(url, dict):
                    url = url.get("url")
            if not isinstance(url, str):
                raise APIError(400, "Audio content parts require a string `audio_url` or `url` field.",
                               f"{param}.{index}.audio_url")
            parts.append("[audio]")
            continue
        raise APIError(400, "Unsupported content part type.", f"{param}.{index}.type",
                       "unsupported_content_type")
    return "".join(parts)


def response_format_hint(response_format):
    if not response_format or not isinstance(response_format, dict):
        return ""
    fmt_type = response_format.get("type")
    if fmt_type == "json_object":
        return "Respond with a valid JSON object."
    if fmt_type == "json_schema":
        schema = response_format.get("json_schema") or {}
        name = schema.get("name") if isinstance(schema, dict) else None
        if name:
            return f"Respond with a valid JSON object matching the `{name}` schema."
        return "Respond with a valid JSON object matching the supplied schema."
    if fmt_type == "text":
        return ""
    return f"Respond using the requested `{fmt_type}` format."


def prompt_with_response_format(prompt, response_format):
    hint = response_format_hint(response_format)
    if not hint:
        return prompt
    return f"{prompt}\n\n{hint}" if prompt else hint


def build_logprobs(text, top_n):
    """Return placeholder logprobs for generated text.

    The current engine does not expose token-level logits, so this is a best-effort
    placeholder used to keep the OpenAI-compatible surface working without misrepresenting
    actual model confidence.
    """
    if not text or top_n is None or top_n <= 0:
        return None
    tokens = list(text)
    return {"tokens": tokens, "token_logprobs": [MOCK_LOGPROB] * len(tokens),
            "top_logprobs": [None] * len(tokens), "text_offset": 0}


# ---- GLM-5.2 tool calling -----------------------------------------------------------------
# The model expresses tool calls as ordinary text (from chat_template.jinja):
#   <tool_call>{name}<arg_key>{k}</arg_key><arg_value>{v}</arg_value>...</tool_call>
# and tool results come back as <|observation|><tool_response>{content}</tool_response>.
# We render those markers into the prompt and parse them back into OpenAI `tool_calls`.
import re

BOX_START, BOX_END = "<tool_call>", "</tool_call>"
TR_OPEN,  TR_CLOSE = "<tool_response>", "</tool_response>"
THINK_OPEN, THINK_CLOSE = "<think>", "</think>"

_BOX_RE  = re.compile(re.escape(BOX_START) + r"(.*?)" + re.escape(BOX_END), re.DOTALL)
_ARG_RE  = re.compile(r"<arg_key>([^<]*)</arg_key><arg_value>(.*?)</arg_value>", re.DOTALL)
_NAME_RE = re.compile(r"\s*([A-Za-z0-9_.\-]+)")
_TAG_RE  = re.compile(r"</?arg_key>|</?arg_value>")

# De-mangler: opt-in recovery for heavily-quantized models that drop the
# <arg_key>K</arg_key><arg_value> structure. Default OFF (never rewrites well-formed output).
_SALVAGE = os.environ.get("COLI_TOOL_SALVAGE", "0") == "1"


def _tool_param_order(tools):
    """name -> ordered param names (required first) from the request schema, for de-mangling."""
    out = {}
    for tool in (tools or []):
        fn = tool.get("function", tool) if isinstance(tool, dict) else {}
        name = fn.get("name")
        if not name:
            continue
        params = ((fn.get("parameters") or {}).get("properties") or {})
        required = list((fn.get("parameters") or {}).get("required") or [])
        out[name] = required + [p for p in params if p not in required]
    return out


def parse_tool_calls(reply, tools=None):
    """Return (content, tool_calls). Strict GLM parse; optional de-mangler (COLI_TOOL_SALVAGE=1)
    rescues malformed int4 output by mapping a lone payload onto the tool's primary parameter."""
    param_order = _tool_param_order(tools)
    calls, salvaged = [], []
    for match in _BOX_RE.finditer(reply):
        inner = match.group(1)
        name_match = _NAME_RE.match(inner)
        name = name_match.group(1) if name_match else inner.strip()
        args = {}
        for arg in _ARG_RE.finditer(inner):
            key, value = arg.group(1), arg.group(2)
            try:
                value = json.loads(value)
            except (json.JSONDecodeError, TypeError):
                pass
            args[key] = value
        if not args and _SALVAGE:
            rest = inner[name_match.end():] if name_match else ""
            payload = _TAG_RE.sub("", rest).strip()
            if payload.startswith("(") and payload.endswith(")"):
                payload = payload[1:-1].strip()
            if payload:
                key = (param_order.get(name) or ["input"])[0]
                try:
                    payload = json.loads(payload)
                except (json.JSONDecodeError, TypeError, ValueError):
                    pass
                args = {key: payload}
                salvaged.append(name)
        calls.append({"id": "call_" + uuid.uuid4().hex[:24], "type": "function",
                      "function": {"name": name, "arguments": json.dumps(args, ensure_ascii=False)}})
    text = _BOX_RE.sub("", reply)
    if THINK_CLOSE in text:
        text = text.split(THINK_CLOSE, 1)[1]
    text = text.replace(THINK_OPEN, "").replace(THINK_CLOSE, "")
    if calls:
        dm = len(salvaged)
        sys.stderr.write("[api] tool-calls: %d total, %d strict, %d de-mangled [%s]%s\n"
                         % (len(calls), len(calls) - dm, dm, "CLEAN" if dm == 0 else "DE-MANGLED",
                            (" -> " + ", ".join(salvaged)) if dm else ""))
        sys.stderr.flush()
    return text.strip(), calls


def render_chat(messages, enable_thinking=False, reasoning_effort=None, tools=None, response_format=None):
    """Render the text-only subset of the official GLM-5.2 chat template."""
    if not isinstance(messages, list) or not messages:
        raise APIError(400, "`messages` must be a non-empty array.", "messages")
    prompt = ["[gMASK]<sop>"]
    if response_format is not None:
        hint = response_format_hint(response_format)
        if hint:
            prompt.append(f"<|system|>{hint}")
    if enable_thinking:
        effort = "High" if reasoning_effort == "high" else "Max"
        prompt.append(f"<|system|>Reasoning Effort: {effort}")
    if tools:
        # AUTHORITATIVE GLM-5.2 tool-declaration block (byte-matches chat_template.jinja): the
        # `# Tools` + <tools></tools> XML structure is what the model was trained on. A made-up
        # preamble makes it hallucinate other frameworks' syntax (e.g. `end_action`).
        prompt.append("<|system|>\n# Tools\n\nYou may call one or more functions to assist with the "
                      "user query.\n\nYou are provided with function signatures within <tools></tools> "
                      "XML tags:\n<tools>\n")
        for tool in tools:
            fn = tool.get("function", tool) if isinstance(tool, dict) else {}
            clean = {k: v for k, v in fn.items() if k not in ("defer_loading", "strict")}
            prompt.append(json.dumps(clean, ensure_ascii=False) + "\n")
        prompt.append("</tools>\n\nFor each function call, output the function name and arguments "
                      "within the following XML format:\n<tool_call>{function-name}"
                      "<arg_key>{arg-key-1}</arg_key><arg_value>{arg-value-1}</arg_value>"
                      "<arg_key>{arg-key-2}</arg_key><arg_value>{arg-value-2}</arg_value>...</tool_call>")
    prev_tool = False
    for index, message in enumerate(messages):
        if not isinstance(message, dict):
            raise APIError(400, "Each message must be an object.", f"messages.{index}")
        role = message.get("role")
        if role in ("system", "developer"):
            prompt.append(f"<|system|>{content_text(message.get('content'), f'messages.{index}.content')}")
        elif role == "user":
            prompt.append(f"<|user|>{content_text(message.get('content'), f'messages.{index}.content')}")
        elif role == "assistant":
            # content may be null when the message is purely tool_calls
            raw = message.get("content")
            text = content_text(raw, f"messages.{index}.content") if raw is not None else ""
            prompt.append(f"<|assistant|><think></think>{text.strip()}")
            for tc in (message.get("tool_calls") or []):
                fn = tc.get("function", tc) if isinstance(tc, dict) else {}
                args = fn.get("arguments", "{}")
                if isinstance(args, str):
                    try:
                        args = json.loads(args)
                    except (json.JSONDecodeError, TypeError):
                        args = {}
                prompt.append(BOX_START + (fn.get("name") or ""))
                for key, value in (args or {}).items():
                    prompt.append(f"<arg_key>{key}</arg_key><arg_value>"
                                  + (value if isinstance(value, str)
                                     else json.dumps(value, ensure_ascii=False)) + "</arg_value>")
                prompt.append(BOX_END)
        elif role == "tool":
            if not prev_tool:                       # one <|observation|> per consecutive tool run
                prompt.append("<|observation|>")
            prompt.append(TR_OPEN + content_text(message.get("content"), f"messages.{index}.content") + TR_CLOSE)
        else:
            raise APIError(400, f"Unsupported message role: {role!r}.",
                           f"messages.{index}.role", "unsupported_role")
        prev_tool = (role == "tool")
    prompt.append("<|assistant|><think>" if enable_thinking else
                  "<|assistant|><think></think>")
    return "".join(prompt)


# ---- Qwen3.5 / Ornith tool calling -------------------------------------------------------
# Qwen3.5 uses a different tool-call format from GLM-5.2:
#   <tool_call>
#   <function=function_name>
#   <parameter=param_name>
#   param_value
#   </parameter>
#   </function>
#   </tool_call>
# Tool responses come back wrapped in <tool_response>...</tool_response> inside a user turn.
_QWEN_TC_RE = re.compile(r"<tool_call>\s*<function=([^>]+)>(.*?)</function>\s*</tool_call>",
                          re.DOTALL)
_QWEN_PARAM_RE = re.compile(r"<parameter=([^>]+)>\s*(.*?)\s*</parameter>", re.DOTALL)


def parse_tool_calls_qwen(reply, tools=None):
    """Return (content, tool_calls) for Qwen3.5 <function=>/<parameter=> format."""
    calls = []
    for match in _QWEN_TC_RE.finditer(reply):
        name = match.group(1).strip()
        body_text = match.group(2)
        args = {}
        for pmatch in _QWEN_PARAM_RE.finditer(body_text):
            key = pmatch.group(1).strip()
            value = pmatch.group(2).strip()
            try:
                value = json.loads(value)
            except (json.JSONDecodeError, TypeError):
                pass
            args[key] = value
        calls.append({"id": "call_" + uuid.uuid4().hex[:24], "type": "function",
                       "function": {"name": name, "arguments": json.dumps(args, ensure_ascii=False)}})
    text = _QWEN_TC_RE.sub("", reply)
    if THINK_CLOSE in text:
        text = text.split(THINK_CLOSE, 1)[1]
    text = text.replace(THINK_OPEN, "").replace(THINK_CLOSE, "")
    if calls:
        sys.stderr.write("[api] qwen tool-calls: %d\n" % len(calls))
        sys.stderr.flush()
    return text.strip(), calls


def render_chat_qwen(messages, enable_thinking=False, reasoning_effort=None, tools=None, response_format=None):
    """Render the Qwen3.5/Ornith <|im_start|>/<|im_end|> chat template (text-only subset)."""
    if not isinstance(messages, list) or not messages:
        raise APIError(400, "`messages` must be a non-empty array.", "messages")
    prompt = []
    if response_format is not None:
        hint = response_format_hint(response_format)
        if hint:
            prompt.append(f"<|im_start|>system\n{hint}<|im_end|>\n")
    first = messages[0] if messages else {}
    has_system_first = isinstance(first, dict) and first.get("role") in ("system", "developer")

    if tools:
        # Qwen3.5 tool declaration format matching chat_template.jinja
        tool_block = "<|im_start|>system\n# Tools\n\nYou have access to the following functions:\n\n<tools>"
        for tool in tools:
            fn = tool.get("function", tool) if isinstance(tool, dict) else {}
            clean = {k: v for k, v in fn.items() if k not in ("defer_loading", "strict")}
            tool_block += "\n" + json.dumps(clean, ensure_ascii=False)
        tool_block += "\n</tools>"
        tool_block += ("\n\nIf you choose to call a function ONLY reply in the following format "
                       "with NO suffix:\n\n<tool_call>\n<function=example_function_name>\n"
                       "<parameter=example_parameter_1>\nvalue_1\n</parameter>\n"
                       "<parameter=example_parameter_2>\nvalue_2\n</parameter>\n"
                       "</function>\n</tool_call>")
        if has_system_first:
            sys_content = content_text(messages[0].get("content"), "messages.0.content").strip()
            if sys_content:
                tool_block += "\n\n" + sys_content
        prompt.append(tool_block + "<|im_end|>\n")
        messages = messages[1:] if has_system_first else messages
    elif has_system_first:
        sys_content = content_text(messages[0].get("content"), "messages.0.content").strip()
        prompt.append(f"<|im_start|>system\n{sys_content}<|im_end|>\n")
        messages = messages[1:]

    prev_tool = False
    for index, message in enumerate(messages):
        if not isinstance(message, dict):
            raise APIError(400, "Each message must be an object.", f"messages.{index}")
        role = message.get("role")
        if role in ("system", "developer"):
            sys_content = content_text(message.get("content"), f"messages.{index}.content").strip()
            prompt.append(f"<|im_start|>system\n{sys_content}<|im_end|>\n")
        elif role == "user":
            text = content_text(message.get("content"), f"messages.{index}.content")
            prompt.append(f"<|im_start|>user\n{text}<|im_end|>\n")
        elif role == "assistant":
            raw = message.get("content")
            text = content_text(raw, f"messages.{index}.content").strip() if raw is not None else ""
            # Preserve reasoning_content if present, else strip any embedded <think> block
            reasoning = (message.get("reasoning_content") or "").strip()
            if not reasoning and THINK_CLOSE in text:
                parts = text.split(THINK_CLOSE, 1)
                reasoning = parts[0].replace(THINK_OPEN, "").strip()
                text = parts[1].lstrip("\n")
            think_block = f"<think>\n{reasoning}\n</think>\n\n" if reasoning else "<think>\n\n</think>\n\n"
            prompt.append(f"<|im_start|>assistant\n{think_block}{text}")
            for tc in (message.get("tool_calls") or []):
                fn = tc.get("function", tc) if isinstance(tc, dict) else {}
                args = fn.get("arguments", "{}")
                if isinstance(args, str):
                    try:
                        args = json.loads(args)
                    except (json.JSONDecodeError, TypeError):
                        args = {}
                prompt.append("\n<tool_call>\n<function=" + (fn.get("name") or "") + ">\n")
                for key, value in (args or {}).items():
                    prompt.append(f"<parameter={key}>\n"
                                  + (value if isinstance(value, str)
                                     else json.dumps(value, ensure_ascii=False))
                                  + "\n</parameter>\n")
                prompt.append("</function>\n</tool_call>")
            prompt.append("<|im_end|>\n")
        elif role == "tool":
            if not prev_tool:
                prompt.append("<|im_start|>user")
            prompt.append("\n<tool_response>\n"
                          + content_text(message.get("content"), f"messages.{index}.content")
                          + "\n</tool_response>")
            next_role = messages[index + 1].get("role") if index + 1 < len(messages) else None
            if next_role != "tool":
                prompt.append("<|im_end|>\n")
        else:
            raise APIError(400, f"Unsupported message role: {role!r}.",
                           f"messages.{index}.role", "unsupported_role")
        prev_tool = (role == "tool")

    if enable_thinking:
        prompt.append("<|im_start|>assistant\n<think>\n")
    else:
        prompt.append("<|im_start|>assistant\n<think>\n\n</think>\n\n")
    return "".join(prompt)


def load_jinja_chat_template(model_dir):
    """Return the source of a model's `chat_template.jinja`, or None if absent.

    Qwen3.5/GLM checkpoints ship a HuggingFace-style Jinja chat template. When
    present (and jinja2 is installed) the server can render prompts with the
    model's own template instead of the built-in text-subset renderers, which is
    important for exact fidelity with agent harnesses such as Hermes that rely on
    the canonical formatting.
    """
    if not model_dir:
        return None
    try:
        path = Path(model_dir) / "chat_template.jinja"
        if path.is_file():
            return path.read_text()
    except OSError:
        return None
    return None


def render_chat_jinja(template_source, messages, enable_thinking=False,
                      reasoning_effort=None, tools=None, response_format=None):
    """Render a chat prompt using the model's own Jinja chat template.

    jinja2 is an optional dependency: it is imported lazily so the server stays
    dependency-free unless a Jinja template is actually configured. The variables
    exposed (messages, tools, add_generation_prompt, enable_thinking) mirror the
    HuggingFace `apply_chat_template` contract so upstream templates render
    unchanged.
    """
    try:
        from jinja2 import Environment
        from jinja2.exceptions import TemplateError
    except ImportError as exc:  # pragma: no cover - exercised only without jinja2
        raise APIError(500, "Jinja chat templates require the optional `jinja2` package. "
                       "Install it with: pip install jinja2",
                       None, "jinja2_missing", "server_error") from exc
    if not isinstance(messages, list):
        raise APIError(400, "`messages` must be an array.", "messages")
    env = Environment(trim_blocks=True, lstrip_blocks=True, autoescape=False)
    env.filters.setdefault("tojson", lambda value, **_: json.dumps(value))
    try:
        template = env.from_string(template_source)
        rendered = template.render(
            messages=messages,
            tools=tools,
            add_generation_prompt=True,
            enable_thinking=enable_thinking,
            reasoning_effort=reasoning_effort,
            response_format=response_format,
        )
    except TemplateError as exc:
        raise APIError(400, f"Chat template rendering failed: {exc}", "messages",
                       "template_error") from exc
    return prompt_with_response_format(rendered, response_format)


def split_thinking(text):
    """Split model output into (reasoning_content, content) by stripping <think>...</think>."""
    if THINK_CLOSE in text:
        before, after = text.split(THINK_CLOSE, 1)
        reasoning = before.replace(THINK_OPEN, "").lstrip("\n")
        content = after.lstrip("\n")
        return reasoning, content
    # No closing tag: the model was told not to think, strip any stray open tag
    return "", text.replace(THINK_OPEN, "").replace(THINK_CLOSE, "")


def generation_options(body, limit):
    if body.get("n", 1) != 1:
        raise APIError(400, "Colibri currently supports `n=1` only.", "n", "unsupported_value")
    # `tools`/`functions` are handled by render_chat (declaration) + parse_tool_calls (output).
    if body.get("stop") is not None:
        raise APIError(400, "Custom stop sequences are not supported yet.", "stop", "unsupported_parameter")
    logprobs = body.get("logprobs")
    if logprobs is not None:
        if isinstance(logprobs, bool):
            logprobs = DEFAULT_LOGPROBS_COUNT if logprobs else None
        elif isinstance(logprobs, int) and logprobs >= 0:
            pass
        else:
            raise APIError(400, "`logprobs` must be a boolean or a non-negative integer.", "logprobs")
    if body.get("frequency_penalty", 0) or body.get("presence_penalty", 0):
        raise APIError(400, "Token penalties are not supported yet.", None, "unsupported_parameter")
    response_format = body.get("response_format")
    if response_format is not None and response_format != {"type": "text"}:
        if not isinstance(response_format, dict):
            raise APIError(400, "`response_format` must be an object or null.", "response_format")
        fmt_type = response_format.get("type")
        if fmt_type not in ("text", "json_object", "json_schema"):
            raise APIError(400, "Only text, json_object, and json_schema response formats are supported.",
                           "response_format", "unsupported_parameter")

    maximum = body.get("max_completion_tokens")
    maximum_param = "max_completion_tokens"
    if maximum is None:
        maximum = body.get("max_tokens")
        maximum_param = "max_tokens"
    if maximum is None:
        maximum = min(256, limit)
    temperature = body.get("temperature")
    top_p = body.get("top_p")
    top_k = body.get("top_k")
    seed = body.get("seed")
    min_p = body.get("min_p")
    temperature = 0.7 if temperature is None else temperature
    top_p = 0.9 if top_p is None else top_p
    top_k = 0 if top_k is None else top_k
    seed = 0 if seed is None else seed
    min_p = 0.0 if min_p is None else min_p
    # The backend uses integer-valued sampling controls for `top_k` and `seed`,
    # while `temperature`, `top_p`, and `min_p` can be passed as floats.
    if isinstance(maximum, bool) or not isinstance(maximum, int) or not 1 <= maximum <= limit:
        raise APIError(400, f"`{maximum_param}` must be an integer between 1 and {limit}.", maximum_param)
    if isinstance(temperature, bool) or not isinstance(temperature, (int, float)) or not 0 <= temperature <= 2:
        raise APIError(400, "`temperature` must be between 0 and 2.", "temperature")
    if isinstance(top_p, bool) or not isinstance(top_p, (int, float)) or not 0 < top_p <= 1:
        raise APIError(400, "`top_p` must be greater than 0 and at most 1.", "top_p")
    if isinstance(min_p, bool) or not isinstance(min_p, (int, float)) or not 0 <= min_p <= 1:
        raise APIError(400, "`min_p` must be between 0 and 1.", "min_p")
    if isinstance(top_k, bool) or not isinstance(top_k, int) or top_k < 0:
        raise APIError(400, "`top_k` must be a non-negative integer.", "top_k")
    if isinstance(seed, bool) or not isinstance(seed, int) or seed < 0:
        raise APIError(400, "`seed` must be a non-negative integer.", "seed")
    return maximum, float(temperature), float(top_p), top_k, seed, logprobs, float(min_p)


def read_engine_turn(stream, sentinel, on_bytes):
    pending = b""
    while True:
        byte = stream.read(1)
        if byte == b"":
            raise RuntimeError("colibri engine exited unexpectedly")
        pending += byte
        if pending.endswith(sentinel):
            data = pending[:-len(sentinel)]
            if data:
                on_bytes(data)
            break
        if len(pending) > len(sentinel):
            on_bytes(pending[:-len(sentinel)])
            pending = pending[-len(sentinel):]

    fields = stream.readline().decode("utf-8", "replace").strip().split()
    if len(fields) < 5 or fields[0] != "STAT":
        raise RuntimeError(f"invalid engine status: {' '.join(fields)}")
    return {
        "completion_tokens": int(fields[1]),
        "tokens_per_second": float(fields[2]),
        "cache_hit_percent": float(fields[3]),
        "rss_gb": float(fields[4]),
        "prompt_tokens": int(fields[5]) if len(fields) > 5 else 0,
        "length_limited": bool(int(fields[6])) if len(fields) > 6 else False,
    }


class Engine:
    def __init__(self, executable, model, cap=8, max_tokens=1024, env=None, kv_slots=1):
        child_env = dict(env or os.environ, SNAP=str(model), SERVE="1", NGEN=str(max_tokens),
                         KV_SLOTS=str(kv_slots))
        self.process = subprocess.Popen(
            [str(executable), "--model", str(model)], env=child_env, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, bufsize=0,
        )
        self.lock = threading.Lock()
        self.kv_slots = kv_slots
        read_engine_turn(self.process.stdout, READY, lambda _: None)

    def generate(self, prompt, max_tokens, temperature, top_p, on_text, cache_slot=0, top_k=0, seed=0, min_p=0.0):
        if isinstance(cache_slot, bool) or not isinstance(cache_slot, int) or not 0 <= cache_slot < self.kv_slots:
            raise APIError(400, "Invalid cache slot.", "cache_slot")
        payload = prompt.encode("utf-8")
        if b"\0" in payload:
            raise APIError(400, "NUL bytes are not supported in prompts.", "messages")
        decoder = codecs.getincrementaldecoder("utf-8")("replace")

        def decode(data):
            text = decoder.decode(data)
            if text:
                on_text(text)

        with self.lock:
            if self.process.poll() is not None:
                raise RuntimeError("colibri engine is not running")
            header = (f"\x02PROMPT {len(payload)} {max_tokens} {temperature:.8g} "
                      f"{top_p:.8g} {cache_slot} {top_k} {seed} {min_p:.8g}\n").encode()
            self.process.stdin.write(header + payload + b"\n")
            self.process.stdin.flush()
            stats = read_engine_turn(self.process.stdout, END, decode)
            tail = decoder.decode(b"", final=True)
            if tail:
                on_text(tail)
            return stats

    def close(self):
        if self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()


def model_object(model_id, created):
    return {"id": model_id, "object": "model", "created": created, "owned_by": "colibri"}


class APIServer(ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self, address, engine, model_id, api_key=None, max_tokens=1024,
                 cors_origins=DEFAULT_CORS_ORIGINS, max_queue=8, queue_timeout=300,
                 kv_slots=1, model_family="glm", chat_template=None):
        super().__init__(address, APIHandler)
        self.engine = engine
        self.model_id = model_id
        self.api_key = api_key
        self.max_tokens = max_tokens
        self.scheduler = GenerationScheduler(max_queue, queue_timeout)
        self.kv_slots = kv_slots
        self.cors_origins = tuple(cors_origins)
        self.created = int(time.time())
        self.model_family = model_family
        self.chat_template = chat_template


class APIHandler(BaseHTTPRequestHandler):
    protocol_version = "HTTP/1.1"
    server_version = "colibri"

    def log_message(self, fmt, *args):
        sys.stderr.write("[api] %s - %s\n" % (self.address_string(), fmt % args))

    def send_json(self, status, body, request_id=None, headers=None):
        data = json.dumps(body, ensure_ascii=False, separators=(",", ":")).encode()
        self.send_response(status)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(data)))
        if request_id:
            self.send_header("x-request-id", request_id)
        for name, value in (headers or {}).items():
            self.send_header(name, value)
        self.send_cors_headers()
        self.end_headers()
        self.wfile.write(data)

    def send_cors_headers(self):
        origin = self.headers.get("Origin")
        if not origin or ("*" not in self.server.cors_origins and origin not in self.server.cors_origins):
            return
        safe_origin = "".join(c for c in origin if c.isprintable() and c not in "\r\n\0")
        self.send_header("Access-Control-Allow-Origin", "*" if "*" in self.server.cors_origins else safe_origin)
        self.send_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS")
        self.send_header("Access-Control-Allow-Headers", "Authorization, Content-Type")
        self.send_header("Access-Control-Expose-Headers",
                         "x-request-id, x-colibri-queue-wait-ms, Retry-After")
        self.send_header("Access-Control-Max-Age", "600")
        if "*" not in self.server.cors_origins:
            self.send_header("Vary", "Origin")

    def require_auth(self):
        if self.server.api_key and self.headers.get("Authorization") != f"Bearer {self.server.api_key}":
            raise APIError(401, "Invalid or missing API key.", None, "invalid_api_key",
                           "authentication_error")

    def read_json(self):
        try:
            length = int(self.headers.get("Content-Length", "0"))
        except ValueError:
            raise APIError(400, "Invalid Content-Length header.")
        if length < 1 or length > MAX_BODY:
            raise APIError(400, f"Request body must be between 1 and {MAX_BODY} bytes.")
        try:
            body = json.loads(self.rfile.read(length))
        except (json.JSONDecodeError, UnicodeDecodeError):
            raise APIError(400, "Request body must be valid JSON.")
        if not isinstance(body, dict):
            raise APIError(400, "Request body must be a JSON object.")
        return body

    def check_model(self, body):
        model = body.get("model")
        if model != self.server.model_id:
            raise APIError(404, f"The model `{model}` does not exist.", "model", "model_not_found")

    def do_GET(self):
        request_id = "req_" + uuid.uuid4().hex
        try:
            path = urlsplit(self.path).path
            if path == "/health":
                self.send_json(200, {"status": "ok", "scheduler": self.server.scheduler.snapshot(),
                                     "kv_slots": self.server.kv_slots}, request_id)
                return
            self.require_auth()
            if path == "/v1/models":
                self.send_json(200, {"object": "list", "data": [model_object(
                    self.server.model_id, self.server.created)]}, request_id)
            elif path.startswith("/v1/models/") and unquote(path[11:]) == self.server.model_id:
                self.send_json(200, model_object(self.server.model_id, self.server.created), request_id)
            else:
                raise APIError(404, "Not found.", None, "not_found")
        except APIError as error:
            self.send_json(error.status, error_object(error), request_id, error.headers)

    def do_OPTIONS(self):
        self.send_response(204)
        self.send_header("Content-Length", "0")
        self.send_cors_headers()
        self.end_headers()

    def do_POST(self):
        request_id = "req_" + uuid.uuid4().hex
        try:
            self.require_auth()
            body = self.read_json()
            path = urlsplit(self.path).path
            if path == "/v1/embeddings":
                self.embeddings(body, request_id)
                return
            self.check_model(body)
            if path == "/v1/chat/completions":
                self.chat_completion(body, request_id)
            elif path == "/v1/completions":
                self.completion(body, request_id)
            else:
                raise APIError(404, "Not found.", None, "not_found")
        except APIError as error:
            self.send_json(error.status, error_object(error), request_id, error.headers)
        except ClientCancelled:
            pass
        except (BrokenPipeError, ConnectionResetError):
            pass
        except Exception as error:
            self.log_error("request failed: %s", error)
            api_error = APIError(500, "The colibri engine failed to process the request.",
                                 None, "engine_error", "server_error")
            try:
                self.send_json(500, error_object(api_error), request_id)
            except OSError:
                pass

    def generation(self, body, prompt, request_id, chat, enable_thinking=False):
        maximum, temperature, top_p, top_k, seed, logprobs, min_p = generation_options(body, self.server.max_tokens)
        response_format = body.get("response_format")
        tools = (body.get("tools") or body.get("functions") or None) if chat else None
        # colibri-specific extension: select a per-request KV-cache slot for continued generation.
        cache_slot = body.get("cache_slot", 0)
        if isinstance(cache_slot, bool) or not isinstance(cache_slot, int) or not 0 <= cache_slot < self.server.kv_slots:
            raise APIError(400, f"`cache_slot` must be an integer between 0 and {self.server.kv_slots - 1}.",
                           "cache_slot")
        stream = body.get("stream", False)
        if not isinstance(stream, bool):
            raise APIError(400, "`stream` must be a boolean.", "stream")
        stream_options = body.get("stream_options") if stream else None
        if stream and stream_options is not None and not isinstance(stream_options, dict):
            raise APIError(400, "`stream_options` must be an object.", "stream_options")
        include_usage = bool((stream_options or {}).get("include_usage"))
        object_name = "chat.completion" if chat else "text_completion"
        id_prefix = "chatcmpl-" if chat else "cmpl-"
        completion_id = id_prefix + uuid.uuid4().hex
        created = int(time.time())
        is_qwen = self.server.model_family == "qwen35"
        _parse_tools = parse_tool_calls_qwen if is_qwen else parse_tool_calls
        # Determine the tool-call start marker for streaming suppression
        tc_start_marker = "<tool_call>" if is_qwen else BOX_START

        with self.server.scheduler.admit(self.client_disconnected) as queue_wait:
            queue_headers = {"x-colibri-queue-wait-ms": str(round(queue_wait * 1000))}
            if not stream:
                output = []
                stats = self.server.engine.generate(
                    prompt, maximum, temperature, top_p, output.append, cache_slot,
                    top_k=top_k, seed=seed, min_p=min_p)
                text = "".join(output)
                length_finish = "length" if stats["length_limited"] else "stop"
                choice_logprobs = build_logprobs(text, logprobs) if logprobs else None
                if chat and tools:
                    content, calls = _parse_tools(text, tools)
                    message = {"role": "assistant", "content": content or None, "refusal": None}
                    if calls:
                        message["tool_calls"] = calls
                    finish = "tool_calls" if calls else length_finish
                    choice = {"index": 0, "message": message, "logprobs": choice_logprobs,
                              "finish_reason": finish}
                elif chat:
                    reasoning, content = split_thinking(text)
                    message = {"role": "assistant", "content": content, "refusal": None}
                    if reasoning:
                        message["reasoning_content"] = reasoning
                    choice = {"index": 0, "message": message, "logprobs": choice_logprobs,
                              "finish_reason": length_finish}
                else:
                    choice = {"index": 0, "text": text, "logprobs": choice_logprobs,
                              "finish_reason": length_finish}
                self.send_json(200, {"id": completion_id, "object": object_name, "created": created,
                    "model": self.server.model_id, "choices": [choice], "usage": self.usage(stats)},
                    request_id, queue_headers)
                return

            stream_object = "chat.completion.chunk" if chat else object_name
            self.send_response(200)
            self.send_header("Content-Type", "text/event-stream")
            self.send_header("Cache-Control", "no-cache")
            self.send_header("X-Accel-Buffering", "no")
            self.send_header("x-request-id", request_id)
            for name, value in queue_headers.items(): self.send_header(name, value)
            self.send_cors_headers()
            self.end_headers()
            connected = True
            # KEEPALIVE: engine.generate() blocks SILENTLY during the (minutes-long) cold
            # prefill, and the client drops the socket after its idle timeout. A background pump
            # emits a reasoning_content "." delta (the channel that reliably resets the client's
            # timer and lands in the thinking panel, so answer content stays clean) whenever no
            # event has been written for KA_GAP seconds. All wfile writes share ka_lock so the
            # pump and event() never interleave; last_write gates the pump so it stays quiet
            # while real tokens are flowing (e.g. during decode).
            ka_lock = threading.Lock()
            last_write = [time.time()]
            ka_stop = threading.Event()
            KA_GAP = 10.0
            dbg_echo = os.environ.get("COLI_DEBUG", "0") == "1"   # tee decoded tokens to stderr

            def event(choices, usage_marker=False):
                nonlocal connected
                if not connected:
                    return
                event_body = {"id": completion_id, "object": stream_object, "created": created,
                              "model": self.server.model_id, "choices": choices}
                if include_usage:
                    event_body["usage"] = None if not usage_marker else usage_marker
                data = json.dumps(event_body, ensure_ascii=False, separators=(",", ":"))
                with ka_lock:
                    try:
                        self.wfile.write(f"data: {data}\n\n".encode())
                        self.wfile.flush()
                        last_write[0] = time.time()
                    except OSError:
                        connected = False

            def _keepalive():
                ping = [{"index": 0, "delta": ({"reasoning_content": "."} if chat else {"content": ""}),
                         "logprobs": None, "finish_reason": None}]
                while not ka_stop.wait(1.0):
                    if not connected:
                        return
                    if time.time() - last_write[0] >= KA_GAP:
                        event(ping)

            if chat:
                event([{"index": 0, "delta": {"role": "assistant", "content": ""},
                        "logprobs": None, "finish_reason": None}])

            def emit_content(text):
                choice = ({"index": 0, "delta": {"content": text}, "logprobs": None,
                           "finish_reason": None} if chat else
                          {"index": 0, "text": text, "logprobs": None, "finish_reason": None})
                event([choice])

            def emit_reasoning(text):
                event([{"index": 0, "delta": {"reasoning_content": text},
                        "logprobs": None, "finish_reason": None}])

            ka_thread = threading.Thread(target=_keepalive, daemon=True)
            ka_thread.start()
            if chat and tools:
                # Suppress tool-call markers from the streamed content and parse the authoritative
                # calls from the FULL reply after generation. Hold back a marker-length tail so a
                # <tool_call> split across engine chunks is still caught.
                sp = {"buf": "", "tool": False}
                hold = len(tc_start_marker) - 1
                raw = []
                def emit_tools(chunk):
                    raw.append(chunk)
                    if dbg_echo:
                        sys.stderr.write(chunk); sys.stderr.flush()
                    if sp["tool"]:
                        return
                    sp["buf"] += chunk
                    cut = sp["buf"].find(tc_start_marker)
                    if cut >= 0:
                        if cut:
                            emit_content(sp["buf"][:cut])
                        sp["buf"] = ""
                        sp["tool"] = True
                        return
                    flush = max(0, len(sp["buf"]) - hold)
                    if flush:
                        emit_content(sp["buf"][:flush])
                        sp["buf"] = sp["buf"][flush:]
                stats = self.server.engine.generate(
                    prompt, maximum, temperature, top_p, emit_tools, cache_slot,
                    top_k=top_k, seed=seed, min_p=min_p)
                if not sp["tool"] and sp["buf"]:
                    emit_content(sp["buf"])             # no tool call happened: flush held tail
                _content, calls = _parse_tools("".join(raw), tools)
                for i, tc in enumerate(calls):
                    event([{"index": 0, "delta": {"tool_calls": [{"index": i, "id": tc["id"],
                             "type": "function", "function": {"name": tc["function"]["name"],
                             "arguments": tc["function"]["arguments"]}}]},
                            "logprobs": None, "finish_reason": None}])
                finish = "tool_calls" if calls else ("length" if stats["length_limited"] else "stop")
            elif chat and enable_thinking:
                # Stream reasoning_content for <think> blocks then content after </think>
                ts = {"in_think": False, "seen_close": False, "buf": ""}
                hold = max(len(THINK_OPEN), len(THINK_CLOSE)) - 1
                raw = []
                def emit_thinking(chunk):
                    if dbg_echo:
                        sys.stderr.write(chunk); sys.stderr.flush()
                    raw.append(chunk)
                    ts["buf"] += chunk
                    while True:
                        if ts["seen_close"]:
                            # Past the thinking block: emit as content
                            emit_content(ts["buf"])
                            ts["buf"] = ""
                            break
                        if not ts["in_think"]:
                            idx = ts["buf"].find(THINK_OPEN)
                            if idx >= 0:
                                # Emit text before <think> as content, enter reasoning mode
                                if idx:
                                    emit_content(ts["buf"][:idx])
                                ts["buf"] = ts["buf"][idx + len(THINK_OPEN):]
                                ts["in_think"] = True
                                continue
                            # No <think> yet: hold back enough to catch a split tag
                            flush = max(0, len(ts["buf"]) - hold)
                            if flush:
                                emit_content(ts["buf"][:flush])
                                ts["buf"] = ts["buf"][flush:]
                            break
                        else:
                            idx = ts["buf"].find(THINK_CLOSE)
                            if idx >= 0:
                                if idx:
                                    emit_reasoning(ts["buf"][:idx])
                                ts["buf"] = ts["buf"][idx + len(THINK_CLOSE):].lstrip("\n")
                                ts["in_think"] = False
                                ts["seen_close"] = True
                                continue
                            flush = max(0, len(ts["buf"]) - hold)
                            if flush:
                                emit_reasoning(ts["buf"][:flush])
                                ts["buf"] = ts["buf"][flush:]
                            break
                stats = self.server.engine.generate(
                    prompt, maximum, temperature, top_p, emit_thinking, cache_slot,
                    top_k=top_k, seed=seed, min_p=min_p)
                # Flush any remaining buffer
                if ts["buf"]:
                    if ts["seen_close"] or not ts["in_think"]:
                        emit_content(ts["buf"])
                    else:
                        emit_reasoning(ts["buf"])
                finish = "length" if stats["length_limited"] else "stop"
            else:
                def emit_plain(chunk):
                    if dbg_echo:
                        sys.stderr.write(chunk); sys.stderr.flush()
                    emit_content(chunk)
                stats = self.server.engine.generate(
                    prompt, maximum, temperature, top_p, emit_plain, cache_slot,
                    top_k=top_k, seed=seed, min_p=min_p)
                finish = "length" if stats["length_limited"] else "stop"
            ka_stop.set()                          # generation done: stop the keepalive pump
            ka_thread.join(timeout=2)
            final_choice = ({"index": 0, "delta": {}, "logprobs": None, "finish_reason": finish}
                            if chat else {"index": 0, "text": "", "logprobs": None,
                                          "finish_reason": finish})
            event([final_choice])
            if include_usage:
                event([], self.usage(stats))
            if connected:
                try:
                    self.wfile.write(b"data: [DONE]\n\n")
                    self.wfile.flush()
                except OSError:
                    pass
            self.close_connection = True

    def client_disconnected(self):
        try:
            readable, _, _ = select.select([self.connection], [], [], 0)
            if not readable:
                return False
            flags = socket.MSG_PEEK | getattr(socket, "MSG_DONTWAIT", 0)
            return self.connection.recv(1, flags) == b""
        except (OSError, ValueError):
            return True

    @staticmethod
    def usage(stats):
        prompt = stats["prompt_tokens"]
        completion = stats["completion_tokens"]
        return {"prompt_tokens": prompt, "completion_tokens": completion,
                "total_tokens": prompt + completion}

    def embeddings(self, body, request_id):
        """Stub /v1/embeddings endpoint. Returns 501 until a dedicated embedding engine is wired."""
        raise APIError(501, "Embeddings are not yet supported by this colibri engine.",
                       None, "embeddings_not_supported", "server_error")

    def chat_completion(self, body, request_id):
        reasoning_effort = body.get("reasoning_effort")
        efforts = (None, "none", "minimal", "low", "medium", "high", "xhigh")
        if reasoning_effort not in efforts:
            raise APIError(400, "`reasoning_effort` must be none, minimal, low, medium, high, or xhigh.",
                           "reasoning_effort")
        # COLI_THINK=1 makes thinking the default when the client sends NEITHER reasoning_effort
        # nor enable_thinking (a global switch, like the old server's --think). An explicit
        # client value always wins. Default off => exact OpenAI-standard behavior.
        if (reasoning_effort is None and "enable_thinking" not in body
                and os.environ.get("COLI_THINK", "0") == "1"):
            reasoning_effort = "high"
        enable_thinking = body.get("enable_thinking", reasoning_effort not in (None, "none"))
        if not isinstance(enable_thinking, bool):
            raise APIError(400, "`enable_thinking` must be a boolean.", "enable_thinking")
        tools = body.get("tools") or body.get("functions") or None
        # A model-provided Jinja chat template (opt-in via COLI_JINJA_CHAT=1) takes
        # precedence over the built-in text renderers for exact-template fidelity.
        if self.server.chat_template and os.environ.get("COLI_JINJA_CHAT", "0") == "1":
            prompt = render_chat_jinja(self.server.chat_template, body.get("messages"),
                                       enable_thinking, reasoning_effort, tools,
                                       response_format=body.get("response_format"))
        else:
            is_qwen = self.server.model_family == "qwen35"
            render = render_chat_qwen if is_qwen else render_chat
            prompt = render(body.get("messages"), enable_thinking, reasoning_effort, tools,
                           response_format=body.get("response_format"))
        self.generation(body, prompt, request_id, True, enable_thinking)

    def completion(self, body, request_id):
        prompt = body.get("prompt")
        if not isinstance(prompt, str):
            raise APIError(400, "Colibri currently requires `prompt` to be a string.", "prompt")
        prompt = prompt_with_response_format(prompt, body.get("response_format"))
        self.generation(body, prompt, request_id, False)


def serve(model, host="127.0.0.1", port=8000, model_id="glm-5.2-colibri", api_key=None,
          cap=8, max_tokens=1024, engine=None, env=None, cors_origins=None,
          max_queue=8, queue_timeout=300, kv_slots=1):
    if not 1 <= max_tokens:
        raise ValueError("max_tokens must be positive")
    if not 1 <= port <= 65535:
        raise ValueError("port must be between 1 and 65535")
    if max_queue < 0:
        raise ValueError("max_queue cannot be negative")
    if queue_timeout <= 0:
        raise ValueError("queue_timeout must be positive")
    if not 1 <= kv_slots <= 16:
        raise ValueError("kv_slots must be between 1 and 16")
    if host not in ("127.0.0.1", "localhost", "::1") and not api_key:
        print("WARNING: API is listening beyond localhost without COLI_API_KEY", file=sys.stderr)
    engine_path = detect_engine_for_model(model, engine)
    # Detect model family to select the correct chat template and tool-call format
    try:
        config_payload = json.loads((Path(model) / "config.json").read_text())
    except (OSError, json.JSONDecodeError, TypeError):
        config_payload = {}
    model_family = detect_model_family(config_payload) if isinstance(config_payload, dict) else "glm"
    chat_template = load_jinja_chat_template(model)
    runtime = Engine(engine_path,model,cap,max_tokens,env,kv_slots)
    origins = DEFAULT_CORS_ORIGINS if cors_origins is None else tuple(cors_origins)
    server = APIServer((host, port), runtime, model_id, api_key, max_tokens, origins,
                       max_queue, queue_timeout, kv_slots, model_family, chat_template)
    print(f"OpenAI-compatible API listening on http://{host}:{port}/v1", file=sys.stderr)
    previous_sigterm = signal.getsignal(signal.SIGTERM)
    signal.signal(signal.SIGTERM, lambda *_: threading.Thread(target=server.shutdown, daemon=True).start())
    try:
        server.serve_forever()
    finally:
        signal.signal(signal.SIGTERM, previous_sigterm)
        server.scheduler.close()
        server.server_close()
        runtime.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--model", default=os.environ.get("COLI_MODEL"), required=not os.environ.get("COLI_MODEL"))
    parser.add_argument("--engine", default=None)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=8000)
    parser.add_argument("--model-id", default=os.environ.get("COLI_MODEL_ID", "glm-5.2-colibri"))
    parser.add_argument("--api-key", default=os.environ.get("COLI_API_KEY"))
    parser.add_argument("--cors-origin", action="append", default=None,
                        help="allowed browser origin; repeat as needed (use '*' for any origin)")
    parser.add_argument("--cap", type=int, default=8)
    parser.add_argument("--max-tokens", type=int, default=1024)
    parser.add_argument("--max-queue", type=int, default=int(os.environ.get("COLI_MAX_QUEUE", "8")))
    parser.add_argument("--queue-timeout", type=float,
                        default=float(os.environ.get("COLI_QUEUE_TIMEOUT", "300")))
    parser.add_argument("--kv-slots", type=int, default=int(os.environ.get("COLI_KV_SLOTS", "1")))
    args = parser.parse_args()
    serve(args.model, args.host, args.port, args.model_id, args.api_key,
          args.cap,args.max_tokens,args.engine,cors_origins=args.cors_origin,
          max_queue=args.max_queue,queue_timeout=args.queue_timeout,kv_slots=args.kv_slots)


if __name__ == "__main__":
    main()
