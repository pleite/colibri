import json
import os
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path

from resource_plan import GB, analyze_model, build_plan, environment_for_plan, format_plan


def write_shard(path, tensors):
    offset = 0
    header = {}
    payload = b""
    for name, size in tensors:
        header[name] = {"dtype": "U8", "shape": [size], "data_offsets": [offset, offset + size]}
        payload += b"\0" * size
        offset += size
    raw = json.dumps(header).encode()
    path.write_bytes(struct.pack("<Q", len(raw)) + raw + payload)


class ResourcePlanTest(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.model = Path(self.tmp.name)
        (self.model / "config.json").write_text(json.dumps({
            "num_hidden_layers": 2,
            "n_routed_experts": 2,
            "kv_lora_rank": 4,
            "qk_rope_head_dim": 2,
            "qk_nope_head_dim": 3,
            "v_head_dim": 5,
            "num_attention_heads": 2,
        }))
        write_shard(self.model / "model.safetensors", [
            ("model.embed_tokens.weight", 100),
            ("model.layers.0.self_attn.q_a_proj.weight", 200),
            ("model.layers.1.mlp.experts.0.gate_proj.weight", 30),
            ("model.layers.1.mlp.experts.0.up_proj.weight", 30),
            ("model.layers.1.mlp.experts.1.gate_proj.weight", 30),
            ("model.layers.1.mlp.experts.1.up_proj.weight", 30),
        ])

    def tearDown(self):
        self.tmp.cleanup()

    def test_analyzes_dense_and_expert_storage(self):
        info = analyze_model(self.model)
        self.assertEqual(info["dense_bytes"], 300)
        self.assertEqual(info["expert_bytes"], 120)
        self.assertEqual(info["expert_count"], 2)
        self.assertEqual(info["per_cap_bytes"], 60)

    def test_builds_bounded_three_tier_plan(self):
        gpus = [{"index": 0, "name": "test-gpu", "total_bytes": 12 * GB,
                 "free_bytes": 10 * GB}]
        plan = build_plan(self.model, ram_gb=16, context=32, vram_gb=20,
                          available_memory=32 * GB, available_disk=100 * GB, gpus=gpus)
        self.assertEqual(plan["version"], 1)
        self.assertEqual(plan["accelerator"]["selected_backend"], "cuda")
        self.assertEqual(plan["tiers"]["ram"]["budget_bytes"], 16 * GB)
        self.assertLessEqual(plan["tiers"]["vram"]["budget_bytes"], 8 * GB)
        self.assertIn("required RAM backing", plan["warnings"][0])
        self.assertIn("0:test-gpu", format_plan(plan))

    def test_filters_requested_devices(self):
        gpus = [{"index": 0, "name": "a", "total_bytes": 8 * GB, "free_bytes": 8 * GB}]
        plan = build_plan(self.model, available_memory=16 * GB, available_disk=1,
                          gpus=gpus, gpu_indices=[1])
        self.assertEqual(plan["tiers"]["vram"]["devices"], [])
        self.assertIn("not detected", plan["warnings"][0])

    def test_cli_emits_versioned_json(self):
        cli = Path(__file__).parents[1] / "coli"
        run = subprocess.run([
            sys.executable, str(cli), "plan", "--model", str(self.model),
            "--gpu", "none", "--json",
        ], text=True, capture_output=True, check=True)
        plan = json.loads(run.stdout)
        self.assertEqual(plan["version"], 1)
        self.assertEqual(plan["model"]["expert_count"], 2)

    def test_applies_plan_without_overriding_explicit_settings(self):
        gpus = [
            {"index": 0, "name": "a", "total_bytes": 12 * GB, "free_bytes": 10 * GB},
            {"index": 1, "name": "b", "total_bytes": 12 * GB, "free_bytes": 10 * GB},
        ]
        plan = build_plan(self.model, ram_gb=16, available_memory=32 * GB,
                          available_disk=1, gpus=gpus)
        env = environment_for_plan(plan, {"RAM_GB": "12", "PIN": "stats.txt",
                                               "COLI_GPUS": "1"})
        self.assertEqual(env["RAM_GB"], "12")
        self.assertEqual(env["COLI_CUDA"], "1")
        self.assertEqual(env["COLI_GPUS"], "1")
        self.assertEqual(env["PIN_GB"], env["CUDA_EXPERT_GB"])

    def test_cpu_binary_does_not_apply_gpu_tier(self):
        plan = build_plan(self.model, available_memory=16 * GB, available_disk=1,
                          gpus=[{"index": 0, "name": "a", "total_bytes": 8 * GB,
                                 "free_bytes": 8 * GB}])
        env = environment_for_plan(plan, cuda_enabled=False)
        self.assertIn("RAM_GB", env)
        self.assertNotIn("COLI_CUDA", env)
        disabled = environment_for_plan(plan, {"COLI_CUDA": "0"}, cuda_enabled=True)
        self.assertNotIn("COLI_GPU", disabled)
        self.assertNotIn("CUDA_EXPERT_GB", disabled)

    def test_non_cuda_backend_uses_generic_environment(self):
        accelerators = {
            "cuda": [],
            "rocm": [{"index": 3, "name": "amd", "total_bytes": 16 * GB, "free_bytes": 14 * GB}],
            "vulkan": [],
            "npu": [],
        }
        plan = build_plan(self.model, ram_gb=16, available_memory=32 * GB,
                          available_disk=1, backend="rocm", accelerators=accelerators)
        env = environment_for_plan(plan)
        self.assertEqual(plan["tiers"]["vram"]["backend"], "rocm")
        self.assertEqual(env["COLI_ACCEL"], "rocm")
        self.assertEqual(env["COLI_ACCEL_DEVICES"], "3")
        self.assertIn("COLI_ACCEL_EXPERT_GB", env)

    def test_unified_memory_apu_detected(self):
        # APU/iGPU: VRAM carve-out < 4 GB → unified_memory=True
        accelerators = {
            "cuda": [],
            "rocm": [{"index": 0, "name": "Radeon 890M", "total_bytes": 512 * 1024 * 1024,
                      "free_bytes": 512 * 1024 * 1024, "unified_memory": True}],
            "vulkan": [],
            "npu": [],
        }
        plan = build_plan(self.model, ram_gb=16, available_memory=32 * GB,
                          available_disk=1, backend="rocm", accelerators=accelerators)
        self.assertTrue(plan["tiers"]["vram"]["unified_memory"])
        # Budget is bounded by cache_bytes (RAM pool), not the tiny VRAM carve-out
        self.assertGreater(plan["tiers"]["vram"]["budget_bytes"],
                           512 * 1024 * 1024)
        env = environment_for_plan(plan)
        self.assertEqual(env.get("COLI_ROCM_UNIFIED"), "1")
        # Unified warning should be present
        self.assertTrue(any("unified memory" in w for w in plan["warnings"]))

    def test_unified_memory_does_not_set_flag_for_discrete(self):
        accelerators = {
            "cuda": [],
            "rocm": [{"index": 0, "name": "RX 7900 XTX", "total_bytes": 24 * GB,
                      "free_bytes": 22 * GB, "unified_memory": False}],
            "vulkan": [],
            "npu": [],
        }
        plan = build_plan(self.model, ram_gb=16, available_memory=32 * GB,
                          available_disk=1, backend="rocm", accelerators=accelerators)
        self.assertFalse(plan["tiers"]["vram"]["unified_memory"])
        env = environment_for_plan(plan)
        self.assertNotIn("COLI_ROCM_UNIFIED", env)

    def test_qwen_style_config_uses_aliases_and_layer_fallback(self):
        qwen_model = self.model / "qwen_model"
        qwen_model.mkdir()
        (qwen_model / "config.json").write_text(json.dumps({
            "num_hidden_layers": 0,
            "num_experts_per_tok": 3,
            "num_attention_heads": 8,
            "head_dim": 64,
            "kv_lora_rank": 4,
        }))
        write_shard(qwen_model / "model2.safetensors", [
            ("model.layers.0.self_attn.q_proj.weight", 128),
            ("model.layers.0.mlp.experts.0.gate_proj.weight", 64),
            ("model.layers.0.mlp.experts.1.gate_proj.weight", 64),
            ("model.layers.0.mlp.experts.2.gate_proj.weight", 64),
        ])
        info = analyze_model(qwen_model)
        self.assertEqual(info["layer_count"], 1)
        self.assertEqual(info["expert_count"], 3)
        plan = build_plan(qwen_model, available_memory=16 * GB, available_disk=1)
        self.assertEqual(plan["model"]["layer_count"], 1)
        self.assertGreater(plan["tiers"]["ram"]["cache_slots_per_layer"], 0)

    def test_runtime_environment_defaults_to_all_cpu_cores(self):
        env = environment_for_plan(build_plan(self.model, available_memory=16 * GB,
                                              available_disk=1), {})
        expected = str(os.cpu_count() or 1)
        self.assertEqual(env["COLI_CPU_THREADS"], expected)
        self.assertEqual(env["OMP_NUM_THREADS"], expected)
        self.assertEqual(env["OMP_DYNAMIC"], "FALSE")
        self.assertEqual(env["OMP_PROC_BIND"], "TRUE")

    def test_analyzes_zero_layers_without_tensor_layer_names(self):
        zero_model = self.model / "zero_model"
        zero_model.mkdir()
        (zero_model / "config.json").write_text(json.dumps({"num_hidden_layers": 0}))
        write_shard(zero_model / "dense.safetensors", [
            ("model.embed_tokens.weight", 64),
        ])
        info = analyze_model(zero_model)
        self.assertEqual(info["layer_count"], 0)


if __name__ == "__main__":
    unittest.main()
