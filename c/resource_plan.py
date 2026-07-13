#!/usr/bin/env python3
"""Hardware and model placement planning for colibri's disk/RAM/VRAM tiers."""

import json
import os
import re
import shutil
import statistics
import subprocess
from collections import OrderedDict
from pathlib import Path


GB = 1_000_000_000
SUPPORTED_BACKENDS = ("cuda", "rocm", "vulkan", "npu")
EXPERT_RE = re.compile(r"model\.layers\.(\d+)\.mlp\.experts\.(\d+)\.")
LAYER_RE = re.compile(r"(?:^|\.)model\.layers\.(\d+)\.")
# ROCm APU heuristic: integrated GPUs (e.g. Strix Halo Radeon 890M) report
# only the small BIOS VRAM carve-out via rocm-smi, typically < 4 GB.
# Discrete GPUs always have ≥ 4 GB of dedicated VRAM.
APU_VRAM_THRESHOLD_BYTES = 4 * GB


def _cfg_value(cfg, *keys, default=None):
    """Return the first non-missing config value, preserving 0/False."""
    for key in keys:
        value = cfg.get(key)
        if value is None or value == "":
            continue
        if isinstance(value, (list, tuple, set, dict)):
            if len(value) > 0:
                return value
            continue
        return value
    return default


def _cfg_int(cfg, *keys, default=0):
    value = _cfg_value(cfg, *keys, default=None)
    if value is None:
        return default
    return int(value)


def _first_present_value(*values):
    """Return the first non-empty value, preserving 0/False as valid values."""
    for value in values:
        if value is None or value == "":
            continue
        return value
    return None


def apply_runtime_environment(env=None):
    result = dict(env or {})
    threads = _first_present_value(result.get("COLI_CPU_THREADS"), result.get("OMP_NUM_THREADS"),
                                  os.environ.get("COLI_CPU_THREADS"), os.environ.get("OMP_NUM_THREADS"))
    if threads is None:
        threads = str(os.cpu_count() or 1)
    result.setdefault("COLI_CPU_THREADS", threads)
    result.setdefault("OMP_NUM_THREADS", threads)
    result.setdefault("OMP_DYNAMIC", result.get("OMP_DYNAMIC") or os.environ.get("COLI_OMP_DYNAMIC", "FALSE"))
    result.setdefault("OMP_PROC_BIND", result.get("OMP_PROC_BIND") or os.environ.get("COLI_OMP_PROC_BIND", "TRUE"))
    result.setdefault("OMP_PLACES", result.get("OMP_PLACES") or os.environ.get("COLI_OMP_PLACES", "cores"))
    return result


def _tensor_sizes(path):
    file_size = path.stat().st_size
    with path.open("rb") as stream:
        raw = stream.read(8)
        if len(raw) != 8:
            raise ValueError(f"short safetensors header: {path}")
        length = int.from_bytes(raw, "little")
        if length < 2 or length > file_size - 8:
            raise ValueError(f"invalid safetensors header length: {path}")
        header = json.loads(stream.read(length))
    for name, meta in header.items():
        if name == "__metadata__":
            continue
        start, end = meta["data_offsets"]
        if not 0 <= start <= end <= file_size - 8 - length:
            raise ValueError(f"invalid tensor offsets for {name}: {path}")
        yield name, end - start


def analyze_model(model):
    model = Path(model).resolve()
    config_path = model / "config.json"
    if not config_path.is_file():
        raise ValueError(f"missing config.json: {model}")
    config = json.loads(config_path.read_text())
    shards = sorted(model.glob("*.safetensors"))
    if not shards:
        raise ValueError(f"no safetensors shards: {model}")

    dense_bytes = 0
    expert_groups = {}
    layer_indices = set()
    for shard in shards:
        for name, size in _tensor_sizes(shard):
            match = EXPERT_RE.search(name)
            if match:
                key = tuple(map(int, match.groups()))
                expert_groups[key] = expert_groups.get(key, 0) + size
            else:
                dense_bytes += size
            layer_match = LAYER_RE.search(name)
            if layer_match:
                layer_indices.add(int(layer_match.group(1)))

    layer_sizes = {}
    for (layer, _), size in expert_groups.items():
        layer_sizes.setdefault(layer, []).append(size)
    per_layer = {layer: int(statistics.median(sizes)) for layer, sizes in layer_sizes.items()}
    per_cap_bytes = sum(per_layer.values())
    typical_expert_bytes = int(statistics.median(per_layer.values())) if per_layer else 0
    model_bytes = sum(shard.stat().st_size for shard in shards)
    return {
        "path": str(model),
        "shards": len(shards),
        "model_bytes": model_bytes,
        "dense_bytes": dense_bytes,
        "expert_bytes": sum(expert_groups.values()),
        "expert_count": len(expert_groups),
        "expert_layers": len(per_layer),
        "typical_expert_bytes": typical_expert_bytes,
        "per_cap_bytes": per_cap_bytes,
        "layer_count": len(layer_indices),
        "config": config,
    }


def memory_available():
    try:
        text = Path("/proc/meminfo").read_text()
        return int(re.search(r"MemAvailable:\s+(\d+)", text).group(1)) * 1024
    except (OSError, AttributeError):
        return 0


def discover_gpus():
    command = ["nvidia-smi", "--query-gpu=index,name,memory.total,memory.free",
               "--format=csv,noheader,nounits"]
    try:
        result = subprocess.run(command, text=True, capture_output=True, check=True, timeout=5)
    except (OSError, subprocess.SubprocessError):
        return []
    devices = []
    for line in result.stdout.splitlines():
        fields = [field.strip() for field in line.split(",", 3)]
        if len(fields) != 4:
            continue
        try:
            index, total, free = int(fields[0]), int(fields[2]), int(fields[3])
        except ValueError:
            continue
        devices.append({"index": index, "name": fields[1],
                        "total_bytes": total * 1024 * 1024,
                        "free_bytes": free * 1024 * 1024})
    return devices


def _extract_number(value):
    if isinstance(value, int):
        return value
    if isinstance(value, float):
        return int(value)
    if isinstance(value, str):
        match = re.search(r"\d+(?:\.\d+)?", value.replace(",", ""))
        return int(round(float(match.group(0)))) if match else 0
    return 0


def discover_rocm_gpus():
    command = ["rocm-smi", "--showid", "--showproductname", "--showmeminfo", "vram", "--json"]
    try:
        result = subprocess.run(command, text=True, capture_output=True, check=True, timeout=5)
    except (OSError, subprocess.SubprocessError):
        return []
    try:
        payload = json.loads(result.stdout)
    except json.JSONDecodeError:
        return []
    devices = []
    for _, card in sorted(payload.items()):
        if not isinstance(card, dict):
            continue
        index = _extract_number(card.get("GPU ID"))
        name = str(card.get("Card series") or card.get("Card model") or "rocm-gpu")
        total = _extract_number(card.get("VRAM Total Memory (B)"))
        used = _extract_number(card.get("VRAM Total Used Memory (B)"))
        free = max(0, total - used) if total else 0
        # APU/iGPU heuristic: VRAM carve-out is usually < 4 GB on integrated GPUs
        # (Strix Halo Radeon 890M, Phoenix, etc.).  Users can override via
        # COLI_ROCM_UNIFIED=1/0 at runtime; the plan exposes this field for
        # informational purposes and to set environment variables.
        unified_memory = total > 0 and total < APU_VRAM_THRESHOLD_BYTES
        devices.append({"index": index, "name": name,
                        "total_bytes": total, "free_bytes": free,
                        "unified_memory": unified_memory})
    return devices


def discover_vulkan_gpus():
    command = ["vulkaninfo", "--summary"]
    try:
        result = subprocess.run(command, text=True, capture_output=True, check=True, timeout=5)
    except (OSError, subprocess.SubprocessError):
        return []
    devices = []
    # Minimal parser: "GPU0: ...", "GPU1: ...", or "GPU id = 0 (name)"
    patterns = [
        re.compile(r"GPU(\d+)\s*:\s*(.+)$"),
        re.compile(r"GPU id\s*=\s*(\d+)\s*\((.+)\)"),
    ]
    for line in result.stdout.splitlines():
        text = line.strip()
        for pattern in patterns:
            match = pattern.search(text)
            if not match:
                continue
            devices.append({
                "index": int(match.group(1)),
                "name": match.group(2).strip(),
                "total_bytes": 0,
                "free_bytes": 0,
            })
            break
    unique = OrderedDict()
    for device in devices:
        unique[device["index"]] = device
    return list(unique.values())


def discover_npus():
    devices = []
    accel_root = Path("/sys/class/accel")
    if accel_root.is_dir():
        for node in sorted(accel_root.glob("accel*")):
            idx = _extract_number(node.name)
            name = "npu"
            for candidate in ("device/vendor_name", "device/name", "name"):
                path = node / candidate
                try:
                    if path.is_file():
                        value = path.read_text().strip()
                        if value:
                            name = value
                            break
                except OSError:
                    continue
            devices.append({"index": idx, "name": name, "total_bytes": 0, "free_bytes": 0})
    return devices


def discover_accelerators():
    cuda = discover_gpus()
    rocm = discover_rocm_gpus()
    vulkan = discover_vulkan_gpus()
    npu = discover_npus()
    return {"cuda": cuda, "rocm": rocm, "vulkan": vulkan, "npu": npu}


def _select_backend(backend, accelerators):
    choices = SUPPORTED_BACKENDS
    if backend and backend not in (*choices, "auto", "cpu"):
        raise ValueError(f"unsupported accelerator backend: {backend}")
    if backend in choices:
        return backend
    if backend == "cpu":
        return "cpu"
    for name in choices:
        if accelerators.get(name):
            return name
    return "cpu"


def build_plan(model, ram_gb=0, context=4096, gpu_indices=None, vram_gb=0,
               available_memory=None, available_disk=None, gpus=None,
               backend="auto", accelerators=None):
    info = analyze_model(model)
    cfg = info["config"]
    available_memory = memory_available() if available_memory is None else available_memory
    if available_disk is None:
        try:
            usage = shutil.disk_usage(info["path"])
            available_disk = usage.free
        except OSError:
            available_disk = 500 * GB
    accelerators = discover_accelerators() if accelerators is None else dict(accelerators)
    if gpus is not None:
        accelerators["cuda"] = gpus
    for key in ("cuda", "rocm", "vulkan", "npu"):
        accelerators.setdefault(key, [])
    selected_backend = _select_backend(backend, accelerators)
    gpus = list(accelerators.get(selected_backend, []))
    if gpu_indices is not None:
        wanted = set(gpu_indices)
        gpus = [gpu for gpu in gpus if gpu["index"] in wanted]

    ram_budget = int(ram_gb * GB) if ram_gb > 0 else int(available_memory * 0.88)
    if ram_budget < 4 * GB:
        ram_budget = 8 * GB
    typical = info["typical_expert_bytes"]
    # Prefer config-defined layer counts when present, but fall back to tensor
    # inference and finally to a conservative default so planning remains stable
    # even for models without traditional layer metadata.
    layers = _cfg_int(cfg, "num_hidden_layers", "n_layers", "num_layers", default=0)
    if layers <= 0:
        layers = info["layer_count"]
    if layers <= 0:
        layers = 1
        layer_warning = "model layer count unavailable; defaulting to 1 layer for planning"
    else:
        layer_warning = None
    kv_lora_rank = _cfg_int(cfg, "kv_lora_rank", default=0)
    qk_rope_head_dim = _cfg_int(cfg, "qk_rope_head_dim", "rope_head_dim", "rotary_dim", default=0)
    qk_nope_head_dim = _cfg_int(cfg, "qk_nope_head_dim", "nope_head_dim", default=0)
    v_head_dim = _cfg_int(cfg, "v_head_dim", "value_head_dim", default=0)
    num_attention_heads = _cfg_int(cfg, "num_attention_heads", "n_head", "num_heads", default=0)
    kv_bytes = layers * context * (kv_lora_rank + qk_rope_head_dim) * 4
    kv_buffer = context * num_attention_heads * (qk_nope_head_dim + v_head_dim) * 4
    runtime_bytes = int(1.2 * GB + 2.5 * GB + 64 * typical + kv_bytes + kv_buffer)
    cache_bytes = max(0, ram_budget - info["dense_bytes"] - runtime_bytes)
    per_cap = info["per_cap_bytes"]
    configured_experts = _cfg_int(cfg, "n_routed_experts", "num_experts_per_tok",
                                  "num_experts", "num_local_experts", default=0)
    cap = int(cache_bytes // per_cap) if per_cap else 0
    if configured_experts:
        cap = min(cap, configured_experts)

    reserve = 2 * GB
    gpu_plan = []
    safe_vram = 0
    has_unified = False
    for gpu in gpus:
        unified = gpu.get("unified_memory", False)
        if unified:
            # APU/iGPU: GPU and CPU share the same physical DRAM.  The "free
            # VRAM" reported by rocm-smi is the small hardware carve-out; the
            # true usable budget is bounded by the remaining RAM cache pool.
            # No headroom reserve is deducted (there is no separate VRAM pool
            # to protect); the final budget is capped by vram_budget below.
            gpu_reserve = 0
            usable = cache_bytes
            has_unified = True
        else:
            gpu_reserve = reserve
            usable = max(0, gpu["free_bytes"] - gpu_reserve)
        safe_vram += usable
        gpu_plan.append(dict(gpu, reserve_bytes=gpu_reserve, usable_bytes=usable))
    requested_vram = int(vram_gb * GB) if vram_gb > 0 else safe_vram
    vram_budget = min(requested_vram, safe_vram, cache_bytes)
    vram_experts = int(vram_budget // typical) if typical else 0

    warnings = []
    if layer_warning:
        warnings.append(layer_warning)
    explicit_backend = backend not in (None, "", "auto", "cpu")
    if cap < 1:
        warnings.append("RAM budget cannot hold one expert slot per sparse layer")
    if explicit_backend and not accelerators.get(backend):
        warnings.append(f"requested backend '{backend}' is not detected on this system")
    if gpu_indices is not None and len(gpus) != len(set(gpu_indices)):
        warnings.append("one or more requested GPUs were not detected")
    if gpus and vram_budget < requested_vram:
        warnings.append("VRAM tier was clamped by free VRAM or its required RAM backing")
    if has_unified:
        warnings.append("unified memory APU detected: GPU and CPU share DRAM; "
                        "VRAM budget and CPU expert cache compete for the same pool")

    return {
        "version": 1,
        "model": {key: value for key, value in info.items() if key != "config"},
        "tiers": {
            "disk": {"role": "backing", "model_bytes": info["model_bytes"],
                     "available_bytes": available_disk},
            "ram": {"role": "resident+cache", "available_bytes": available_memory,
                    "budget_bytes": ram_budget, "dense_bytes": info["dense_bytes"],
                    "runtime_bytes": runtime_bytes, "expert_cache_bytes": cache_bytes,
                    "cache_slots_per_layer": cap},
            "vram": {"role": "hot-experts", "devices": gpu_plan,
                     "backend": selected_backend,
                     "budget_bytes": vram_budget, "expert_capacity": vram_experts,
                     "unified_memory": has_unified},
        },
        "accelerator": {
            "requested_backend": backend or "auto",
            "selected_backend": selected_backend,
            "detected": {name: len(devices) for name, devices in accelerators.items()},
        },
        "warnings": warnings,
    }


def environment_for_plan(plan, env=None, cuda_enabled=True):
    """Apply a plan without overriding explicit user environment settings."""
    result = dict(env or {})
    result = apply_runtime_environment(result)
    ram = plan["tiers"]["ram"]
    result.setdefault("RAM_GB", f"{ram['budget_bytes'] / GB:.3f}")

    vram = plan["tiers"]["vram"]
    devices = [device["index"] for device in vram["devices"]]
    backend = vram.get("backend") or plan.get("accelerator", {}).get("selected_backend", "cpu")
    if not devices or vram["budget_bytes"] <= 0:
        return result
    if backend == "cuda" and (not cuda_enabled or result.get("COLI_CUDA", "1") == "0"):
        return result

    expert_budget = f"{vram['budget_bytes'] / GB:.3f}"
    if backend == "cuda":
        result.setdefault("COLI_CUDA", "1")
        if "COLI_GPU" not in result and "COLI_GPUS" not in result:
            key = "COLI_GPU" if len(devices) == 1 else "COLI_GPUS"
            result[key] = ",".join(map(str, devices))
        budget_key = "CUDA_EXPERT_GB"
        result.setdefault(budget_key, expert_budget)
        expert_budget = result[budget_key]
    else:
        result.setdefault("COLI_ACCEL", backend)
        result.setdefault("COLI_ACCEL_DEVICES", ",".join(map(str, devices)))
        budget_key = "COLI_ACCEL_EXPERT_GB"
        result.setdefault(budget_key, expert_budget)
        expert_budget = result[budget_key]
        # For ROCm on unified-memory APUs: signal zero-copy mode to the backend.
        if backend == "rocm" and vram.get("unified_memory"):
            result.setdefault("COLI_ROCM_UNIFIED", "1")
    if result.get("PIN"):
        result.setdefault("PIN_GB", expert_budget)
    return result


def format_bytes(value):
    return f"{value / GB:.1f} GB"


def format_plan(plan):
    model, tiers = plan["model"], plan["tiers"]
    backend = tiers["vram"].get("backend", "cuda")
    lines = [f"model  {model['shards']} shards · {format_bytes(model['model_bytes'])}",
             f"disk   backing store · {format_bytes(tiers['disk']['available_bytes'])} free",
             f"RAM    {format_bytes(tiers['ram']['budget_bytes'])} budget · "
             f"{format_bytes(tiers['ram']['dense_bytes'])} dense · "
             f"{format_bytes(tiers['ram']['runtime_bytes'])} runtime · "
             f"cap {tiers['ram']['cache_slots_per_layer']}/layer"]
    vram = tiers["vram"]
    if vram["devices"]:
        names = ", ".join(f"{gpu['index']}:{gpu['name']}" for gpu in vram["devices"])
        lines.append(f"VRAM   [{backend}] {format_bytes(vram['budget_bytes'])} hot tier · "
                     f"~{vram['expert_capacity']} experts · {names}")
    else:
        lines.append(f"VRAM   [{backend}] no compatible device detected · CPU path")
    lines.extend(f"warn   {warning}" for warning in plan["warnings"])
    return "\n".join(lines)
