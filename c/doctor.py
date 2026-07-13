#!/usr/bin/env python3
"""Read-only installation diagnostics for colibri."""

import os
import json
import subprocess
from pathlib import Path

from resource_plan import (
    GB,
    build_plan,
    detect_model_family,
    discover_accelerators,
    format_plan,
    memory_available,
)


def _check(identifier, status, summary, **details):
    item = {"id": identifier, "status": status, "summary": summary}
    if details:
        item["details"] = details
    return item


def cuda_linkage(engine_path):
    """Return CUDA linkage state without loading the executable or CUDA runtime."""
    if not Path(engine_path).is_file() or os.name != "posix":
        return {"linked": False, "missing": False}
    try:
        result = subprocess.run(["ldd", str(engine_path)], capture_output=True, text=True,
                                timeout=3, check=False)
    except (OSError, subprocess.SubprocessError):
        return {"linked": False, "missing": False}
    lines = [line for line in result.stdout.splitlines() if "libcudart" in line]
    return {"linked": any("not found" not in line for line in lines),
            "missing": any("not found" in line for line in lines)}


def run_doctor(model, ram_gb=0, context=4096, gpu_indices=None, vram_gb=0, *,
               backend="auto",
               engine_path, available_memory=None, available_disk=None, gpus=None,
               accelerators=None, linkage=None):
    """Collect a complete report. No model payload, engine, or CUDA context is loaded."""
    model = Path(model).expanduser().resolve()
    checks = []
    plan = None

    if model.is_dir() and os.access(model, os.R_OK):
        checks.append(_check("model.path", "pass", "model directory is readable", path=str(model)))
    elif model.is_dir():
        checks.append(_check("model.path", "fail", "model directory is not readable", path=str(model)))
    else:
        checks.append(_check("model.path", "fail", "model directory does not exist", path=str(model)))

    config = model / "config.json"
    config_family = "generic"
    try:
        config_payload = json.loads(config.read_text())
        valid_config = isinstance(config_payload, dict)
        config_family = detect_model_family(config_payload)
    except (OSError, ValueError):
        valid_config = False
        config_payload = {}
    checks.append(_check("model.config", "pass" if valid_config else "fail",
                         "config.json is valid" if valid_config else "config.json is missing or invalid"))
    if valid_config and config_family == "qwen35":
        checks.append(_check("model.architecture", "pass", "Qwen3.5 MoE model detected",
                            family=config_family, model_type=config_payload.get("model_type")))
    elif valid_config:
        checks.append(_check("model.architecture", "pass", "generic model detected",
                            family=config_family))
    tokenizer = model / "tokenizer.json"
    checks.append(_check("model.tokenizer", "pass" if tokenizer.is_file() else "fail",
                         "tokenizer.json found" if tokenizer.is_file() else "tokenizer.json is missing"))
    if model.is_dir() and os.access(model, os.W_OK):
        checks.append(_check("storage.persistence", "pass", "model directory can store usage and KV state"))
    elif model.is_dir():
        checks.append(_check("storage.persistence", "warn", "model directory is read-only; disable persistence or change permissions"))
    else:
        checks.append(_check("storage.persistence", "skip", "persistence requires a model directory"))

    engine = Path(engine_path)
    if engine.is_file() and os.access(engine, os.X_OK):
        checks.append(_check("engine.binary", "pass", "engine executable is ready", path=str(engine)))
    elif engine.is_file():
        checks.append(_check("engine.binary", "fail", "engine exists but is not executable", path=str(engine)))
    else:
        checks.append(_check("engine.binary", "fail", "engine is not built", path=str(engine)))

    available_memory = memory_available() if available_memory is None else available_memory
    detected = discover_accelerators() if accelerators is None else dict(accelerators)
    if gpus is not None:
        detected["cuda"] = list(gpus)
        detected_gpus = list(gpus)
    else:
        detected_gpus = list(detected.get("cuda", []))
    linkage = cuda_linkage(engine) if linkage is None else linkage
    requested_backend = backend or "auto"
    checks.append(_check("accelerator.backend", "pass",
                         f"requested backend: {requested_backend}",
                         requested=requested_backend))
    selected_gpus = detected_gpus
    if gpu_indices is not None:
        wanted = set(gpu_indices)
        selected_gpus = [gpu for gpu in detected_gpus if gpu["index"] in wanted]

    if gpu_indices == []:
        checks.append(_check("accelerator.cuda", "skip", "GPU use was explicitly disabled"))
    elif gpu_indices is not None and len(selected_gpus) != len(set(gpu_indices)):
        checks.append(_check("accelerator.cuda", "fail", "one or more requested GPUs were not detected",
                             requested=gpu_indices, detected=[gpu["index"] for gpu in detected_gpus]))
    elif selected_gpus and linkage.get("missing"):
        checks.append(_check("accelerator.cuda", "fail", "CUDA runtime library is missing"))
    elif selected_gpus and linkage.get("linked"):
        checks.append(_check("accelerator.cuda", "pass", "CUDA engine and devices are available",
                             devices=[gpu["index"] for gpu in selected_gpus]))
    elif selected_gpus:
        checks.append(_check("accelerator.cuda", "warn", "NVIDIA GPU detected but the engine is CPU-only",
                             devices=[gpu["index"] for gpu in selected_gpus]))
    else:
        checks.append(_check("accelerator.cuda", "skip", "no NVIDIA GPU detected; CPU path is available"))
    vulkan_devices = detected.get("vulkan", [])
    if vulkan_devices:
        checks.append(_check("accelerator.vulkan", "pass", "Vulkan devices detected",
                             devices=[device["index"] for device in vulkan_devices]))
    else:
        checks.append(_check("accelerator.vulkan", "skip", "no Vulkan device detected"))
    npus = detected.get("npu", [])
    if npus:
        checks.append(_check("accelerator.npu", "pass", "NPU devices detected",
                             devices=[device["index"] for device in npus]))
    else:
        checks.append(_check("accelerator.npu", "skip", "no NPU device detected"))

    try:
        plan = build_plan(model, ram_gb, context, gpu_indices, vram_gb,
                          backend=backend,
                          available_memory=available_memory, available_disk=available_disk,
                          gpus=detected_gpus, accelerators=detected)
        model_info = plan["model"]
        accelerator = plan.get("accelerator", {})
        selected_backend = accelerator.get("selected_backend", "cpu")
        checks.append(_check("accelerator.selection", "pass",
                             f"selected backend: {selected_backend}",
                             detected=accelerator.get("detected", {})))
        checks.append(_check("model.shards", "pass", "safetensors headers are valid",
                             shards=model_info["shards"], model_bytes=model_info["model_bytes"]))
        disk = plan["tiers"]["disk"]
        disk_status = "warn" if disk["available_bytes"] < GB else "pass"
        disk_summary = ("less than 1 GB is free for runtime state" if disk_status == "warn" else
                        "model backing store is available")
        checks.append(_check("storage.disk", disk_status, disk_summary,
                             available_bytes=disk["available_bytes"], model_bytes=disk["model_bytes"]))
        ram = plan["tiers"]["ram"]
        if not available_memory:
            ram_status, ram_summary = "warn", "available RAM could not be measured"
        elif ram["budget_bytes"] > available_memory:
            ram_status, ram_summary = "fail", "planned RAM budget exceeds available memory"
        elif ram["cache_slots_per_layer"] < 1:
            ram_status, ram_summary = "fail", "RAM budget cannot hold one expert slot per sparse layer"
        else:
            ram_status, ram_summary = "pass", "RAM budget is viable"
        checks.append(_check("memory.ram", ram_status, ram_summary,
                             available_bytes=available_memory, budget_bytes=ram["budget_bytes"],
                             cache_slots_per_layer=ram["cache_slots_per_layer"]))
        if plan["warnings"]:
            checks.append(_check("placement.plan", "warn", "; ".join(plan["warnings"])))
        else:
            checks.append(_check("placement.plan", "pass", "tier placement has no warnings"))
    except (OSError, ValueError, KeyError) as error:
        checks.append(_check("model.shards", "fail", str(error)))
        checks.append(_check("storage.disk", "skip", "storage check requires a valid model"))
        checks.append(_check("memory.ram", "skip", "RAM projection requires a valid model"))
        checks.append(_check("placement.plan", "skip", "placement requires a valid model"))

    statuses = {item["status"] for item in checks}
    status = "error" if "fail" in statuses else "warning" if "warn" in statuses else "ok"
    return {"schema_version": 1, "status": status, "model": str(model),
            "checks": checks, "plan": plan}


def format_doctor(report):
    icons = {"pass": "ok", "warn": "warn", "fail": "fail", "skip": "skip"}
    lines = [f"colibri doctor · {report['model']}"]
    for check in report["checks"]:
        lines.append(f"[{icons[check['status']]:>4}] {check['id']:<18} {check['summary']}")
    if report["plan"]:
        lines.extend(["", format_plan(report["plan"])])
    lines.extend(["", f"result {report['status']}"])
    return "\n".join(lines)


def exit_code(report):
    return 1 if report["status"] == "error" else 0
