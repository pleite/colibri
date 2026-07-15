import importlib.util
import io
import json
import logging
import math
import os
import shutil
import struct
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch


class Qwen35QuantConverterTest(unittest.TestCase):
    def write_safetensors(self, path, tensors):
        converted_tensors = []
        for name, values, shape, dtype in tensors:
            payload = struct.pack('<%df' % len(values), *values)
            converted_tensors.append((name, payload, shape, dtype))
        self._write_safetensors(path, converted_tensors)

    def write_safetensors_with_payloads(self, path, tensors):
        self._write_safetensors(path, tensors)

    def _write_safetensors(self, path, tensors):
        header = {}
        data_offset = 0
        payloads = []
        for name, payload, shape, dtype in tensors:
            header[name] = {
                'dtype': dtype,
                'shape': list(shape),
                'data_offsets': [data_offset, data_offset + len(payload)],
            }
            payloads.append(payload)
            data_offset += len(payload)
        header_bytes = json.dumps(header, separators=(',', ':')).encode('utf-8')
        with open(path, 'wb') as fh:
            fh.write(len(header_bytes).to_bytes(8, 'little'))
            fh.write(header_bytes)
            for payload in payloads:
                fh.write(payload)

    def encode_fp8_e4m3(self, values):
        payload = bytearray()
        mantissa_bits = 3
        exponent_bias = 7
        mantissa_scale = 2 ** mantissa_bits
        exponent_bits = 8 - mantissa_bits - 1
        max_exponent = (1 << exponent_bits) - 1
        max_mantissa = (1 << mantissa_bits) - 1
        subnormal_threshold = 2 ** (1 - exponent_bias)
        for value in values:
            if value == 0.0:
                payload.append(0)
                continue
            sign = 0x80 if value < 0 else 0
            magnitude = abs(value)
            if magnitude < subnormal_threshold:
                mantissa = int(round(magnitude / subnormal_threshold * mantissa_scale))
                mantissa = max(0, min(max_mantissa, mantissa))
                payload.append(sign | (mantissa & 0x07))
                continue
            unbiased_exponent = math.frexp(magnitude)[1] - 1
            exponent = max(0, min(max_exponent, unbiased_exponent + exponent_bias))
            mantissa = int(round((magnitude / (2 ** (exponent - exponent_bias)) - 1.0) * mantissa_scale))
            mantissa = max(0, min(max_mantissa, mantissa))
            payload.append(sign | ((exponent & max_exponent) << mantissa_bits) | (mantissa & 0x07))
        return bytes(payload)

    def read_header(self, path):
        with open(path, 'rb') as fh:
            header_len = int.from_bytes(fh.read(8), 'little')
            return json.loads(fh.read(header_len).decode('utf-8'))

    def converter_script_path(self):
        return Path(__file__).resolve().parent.parent / 'tools' / 'convert_qwen35_safetensors.py'

    def run_converter(self, input_dir, output_dir, extra_args=None):
        command = [sys.executable, str(self.converter_script_path()), '--input', str(input_dir), '--output', str(output_dir)]
        if extra_args:
            command.extend(extra_args)
        subprocess.run(command, check=True)

    def test_converter_emits_quantized_tensors(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4, 0.5, -0.6, 0.7, -0.8], [2, 4], 'F32'),
                    ('model.layers.0.mlp.experts.0.gate_proj.weight', [0.25, -0.5, 0.75, -1.0, 1.25, -1.5, 1.75, -2.0], [2, 4], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir)
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['shape'], [2, 4])
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['dtype'], 'F32')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['shape'], [2])
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight']['shape'], [2, 4])
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight.qs']['dtype'], 'F32')
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight.qs']['shape'], [2])
            index_payload = json.loads((output_dir / 'model.safetensors.index.json').read_text(encoding='utf-8'))
            self.assertEqual(index_payload['weight_map']['model.layers.0.self_attn.q_proj.weight'], 'model.safetensors')
            self.assertEqual(index_payload['weight_map']['model.layers.0.self_attn.q_proj.weight.qs'], 'model.safetensors')
            self.assertEqual(index_payload['metadata']['total_size'], (output_dir / 'model.safetensors').stat().st_size)

    def test_converter_supports_fp8_inputs(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            payload = self.encode_fp8_e4m3([0.5, -1.0, 2.0, -4.0])
            self.write_safetensors_with_payloads(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', payload, [2, 2], 'F8_E4M3'),
                ],
            )
            self.run_converter(input_dir, output_dir)
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['dtype'], 'F32')

    def test_converter_normalizes_language_model_tensor_names_for_engine(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.language_model.embed_tokens.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                    ('model.language_model.norm.weight', [1.0, 1.0], [2], 'F32'),
                    ('model.language_model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                    ('model.language_model.layers.0.linear_attn.A_log', [0.5, -0.6], [2], 'F32'),
                    ('model.language_model.layers.0.linear_attn.dt_bias', [0.7, -0.8], [2], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir)
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertIn('model.embed_tokens.weight', header)
            self.assertIn('model.embed_tokens.weight.qs', header)
            self.assertIn('model.norm.weight', header)
            self.assertIn('model.layers.0.self_attn.q_proj.weight', header)
            self.assertIn('model.layers.0.self_attn.q_proj.weight.qs', header)
            self.assertIn('model.layers.0.linear_attn.A_log.weight', header)
            self.assertIn('model.layers.0.linear_attn.dt_bias.weight', header)
            self.assertEqual(header['model.layers.0.linear_attn.A_log.weight']['dtype'], 'F32')
            self.assertEqual(header['model.layers.0.linear_attn.dt_bias.weight']['dtype'], 'F32')
            self.assertNotIn('model.language_model.embed_tokens.weight', header)
            self.assertNotIn('model.language_model.norm.weight', header)
            self.assertNotIn('model.language_model.layers.0.self_attn.q_proj.weight', header)
            self.assertNotIn('model.language_model.layers.0.linear_attn.A_log', header)
            self.assertNotIn('model.language_model.layers.0.linear_attn.dt_bias', header)
            index_payload = json.loads((output_dir / 'model.safetensors.index.json').read_text(encoding='utf-8'))
            self.assertEqual(index_payload['weight_map']['model.embed_tokens.weight'], 'model.safetensors')
            self.assertEqual(index_payload['weight_map']['model.embed_tokens.weight.qs'], 'model.safetensors')
            self.assertEqual(index_payload['weight_map']['model.norm.weight'], 'model.safetensors')
            self.assertEqual(index_payload['weight_map']['model.layers.0.linear_attn.A_log.weight'], 'model.safetensors')
            self.assertEqual(index_payload['weight_map']['model.layers.0.linear_attn.dt_bias.weight'], 'model.safetensors')

    def test_read_safetensors_tensor_payload_reads_from_data_section(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_path = tmpdir / 'model.safetensors'
            payload = b'\x00\x00\x80?\x00\x00\x00@\x00\x00\x80?\x00\x00\x00@'
            header = {
                'model.layers.0.self_attn.q_proj.weight': {
                    'dtype': 'F32',
                    'shape': [2, 2],
                    'data_offsets': [0, len(payload)],
                }
            }
            header_bytes = json.dumps(header, separators=(',', ':')).encode('utf-8')
            with open(input_path, 'wb') as fh:
                fh.write(len(header_bytes).to_bytes(8, 'little'))
                fh.write(header_bytes)
                fh.write(payload)
            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            got_payload = module.read_safetensors_tensor_payload(input_path, header['model.layers.0.self_attn.q_proj.weight'])
            self.assertEqual(got_payload, payload)

    def test_converter_handles_non_finite_values(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, float('nan'), -0.2, float('inf')], [2, 2], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir)
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['dtype'], 'F32')

    def test_converter_generate_index_only_from_existing_output(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir)
            self.assertTrue((output_dir / 'model.safetensors').exists())
            shutil.rmtree(input_dir)
            self.assertFalse(input_dir.exists())
            result = subprocess.run(
                [sys.executable, str(self.converter_script_path()), '--input', str(input_dir), '--output', str(output_dir), '--generate-index-only'],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 0)
            self.assertIn('generating index from 1 safetensors file(s)', result.stderr)
            index_payload = json.loads((output_dir / 'model.safetensors.index.json').read_text(encoding='utf-8'))
            self.assertEqual(index_payload['weight_map']['model.layers.0.self_attn.q_proj.weight'], 'model.safetensors')
            self.assertEqual(index_payload['weight_map']['model.layers.0.self_attn.q_proj.weight.qs'], 'model.safetensors')
 
    def test_converter_generate_index_only_ignores_invalid_output_files(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            output_dir = tmpdir / 'output'
            output_dir.mkdir()
            self.write_safetensors(
                output_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            (output_dir / 'ignore.txt').write_text('ignore me', encoding='utf-8')
            (output_dir / 'broken.safetensors').write_bytes(b'not-a-valid-safetensors-file')
            result = subprocess.run(
                [sys.executable, str(self.converter_script_path()), '--output', str(output_dir), '--generate-index-only'],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 0)
            index_payload = json.loads((output_dir / 'model.safetensors.index.json').read_text(encoding='utf-8'))
            self.assertEqual(index_payload['weight_map']['model.layers.0.self_attn.q_proj.weight'], 'model.safetensors')
            # The synthetic output shard in this test only contains the main tensor entry.
            self.assertNotIn('model.layers.0.self_attn.q_proj.weight.qs', index_payload['weight_map'])
 
    def test_converter_resumes_from_existing_output_without_state_files(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            state_dir = output_dir / '.state'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir, ['--state-dir', str(state_dir)])
            shutil.rmtree(state_dir)
            self.run_converter(input_dir, output_dir, ['--state-dir', str(state_dir)])
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['dtype'], 'F32')

    def test_state_progress_marks_incomplete_state_files_for_reprocessing(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            state_dir = output_dir / '.state'
            input_dir.mkdir()
            output_dir.mkdir()
            state_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            task = {
                'task_id': 'model.safetensors:model.layers.0.self_attn.q_proj.weight',
                'source_name': 'model.safetensors',
                'tensor_name': 'model.layers.0.self_attn.q_proj.weight',
                'meta': {'dtype': 'F32', 'shape': [2, 2], 'data_offsets': [0, 16]},
                'source_path': input_dir / 'model.safetensors',
            }
            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            state_payload = {
                'task_id': task['task_id'],
                'output_tensors': [
                    {'name': 'model.layers.0.self_attn.q_proj.weight', 'payload_path': 'missing.bin', 'dtype': 'U8', 'shape': [2, 2]},
                ],
            }
            state_path = state_dir / module.state_file_name_for_task_id(task['task_id'])
            state_path.write_text(json.dumps(state_payload), encoding='utf-8')
            summary = module.summarize_state_progress(state_dir, output_dir, [task])
            self.assertEqual(summary['incomplete_state_files'], 1)
            self.assertEqual(summary['needs_reprocessing_count'], 1)
            self.assertEqual(summary['needs_reprocessing'][0], task['task_id'])

    def test_inspect_state_reports_reprocessing_candidates(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            state_dir = output_dir / '.state'
            input_dir.mkdir()
            output_dir.mkdir()
            state_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            state_payload = {
                'task_id': 'model.safetensors:model.layers.0.self_attn.q_proj.weight',
                'output_tensors': [
                    {'name': 'model.layers.0.self_attn.q_proj.weight', 'payload_path': 'missing.bin', 'dtype': 'U8', 'shape': [2, 2]},
                ],
            }
            state_file_path = state_dir / module.state_file_name_for_task_id(
                'model.safetensors:model.layers.0.self_attn.q_proj.weight',
            )
            state_file_path.write_text(json.dumps(state_payload), encoding='utf-8')
            result = subprocess.run(
                [sys.executable, str(self.converter_script_path()), '--input', str(input_dir), '--output', str(output_dir), '--inspect-state'],
                text=True,
                capture_output=True,
                check=False,
            )
            self.assertEqual(result.returncode, 0)
            self.assertIn('state progress:', result.stdout)
            self.assertIn('needs reprocessing: 1', result.stdout)

    def test_converter_retries_after_worker_pool_failure(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            state_dir = tmpdir / 'state'
            state_dir.mkdir()
            task = {
                'task_id': 'model.safetensors:some.tensor',
                'source_name': 'model.safetensors',
                'tensor_name': 'some.tensor',
                'meta': {
                    'dtype': 'F32',
                    'shape': [1],
                    'data_offsets': [0, 4],
                },
                'source_path': tmpdir / 'model.safetensors',
            }
            self.write_safetensors(tmpdir / 'model.safetensors', [('some.tensor', [0.0], [1], 'F32')])

            class FakeFuture:
                def __init__(self, result=None, exception=None):
                    self._result = result
                    self._exception = exception

                def result(self):
                    if self._exception is not None:
                        raise self._exception
                    return self._result

            class FakeExecutor:
                submit_calls = 0

                def __init__(self, max_workers):
                    self.max_workers = max_workers

                def __enter__(self):
                    return self

                def __exit__(self, exc_type, exc, tb):
                    return False

                def submit(self, fn, task, logger=None):
                    FakeExecutor.submit_calls += 1
                    if FakeExecutor.submit_calls == 1:
                        raise module.concurrent.futures.process.BrokenProcessPool('simulated worker crash')
                    return FakeFuture(result=[('some.tensor', b'payload', 'U8', [1])])

            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            def fake_wait(futures, return_when=None):
                return ({next(iter(futures))}, set())

            logger = logging.getLogger('test.converter')
            logger.setLevel(logging.INFO)
            logger.propagate = False
            stream = io.StringIO()
            handler = logging.StreamHandler(stream)
            handler.setLevel(logging.INFO)
            logger.addHandler(handler)
            try:
                with patch.object(module.concurrent.futures, 'ProcessPoolExecutor', FakeExecutor), \
                     patch.object(module.concurrent.futures, 'wait', side_effect=fake_wait), \
                     patch.object(module, 'convert_task', return_value=[('some.tensor', b'payload', 'U8', [1])]) as mock_convert:
                    completed = {}
                    module.process_pending_tasks(
                        [task],
                        workers=2,
                        logger=logger,
                        completed=completed,
                        state_dir=state_dir,
                        total_tasks=1,
                        max_retries=1,
                    )
            finally:
                logger.handlers.clear()

            self.assertIn(task['task_id'], completed)
            self.assertEqual(completed[task['task_id']]['kind'], 'state')
            self.assertTrue(mock_convert.called)
            self.assertIn('falling back to sequential conversion', stream.getvalue())

    def test_converter_falls_back_after_single_worker_retry_failure(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            state_dir = tmpdir / 'state'
            state_dir.mkdir()
            task = {
                'task_id': 'model.safetensors:some.tensor',
                'source_name': 'model.safetensors',
                'tensor_name': 'some.tensor',
                'meta': {
                    'dtype': 'F32',
                    'shape': [1],
                    'data_offsets': [0, 4],
                },
                'source_path': tmpdir / 'model.safetensors',
            }
            self.write_safetensors(tmpdir / 'model.safetensors', [('some.tensor', [0.0], [1], 'F32')])

            class FakeExecutor:
                submit_calls = 0

                def __init__(self, max_workers):
                    self.max_workers = max_workers

                def __enter__(self):
                    return self

                def __exit__(self, exc_type, exc, tb):
                    return False

                def submit(self, fn, task, logger=None):
                    FakeExecutor.submit_calls += 1
                    raise module.concurrent.futures.process.BrokenProcessPool('simulated worker crash')

            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            logger = logging.getLogger('test.converter.fallback')
            logger.setLevel(logging.INFO)
            logger.propagate = False
            stream = io.StringIO()
            handler = logging.StreamHandler(stream)
            handler.setLevel(logging.INFO)
            logger.addHandler(handler)
            try:
                with patch.object(module.concurrent.futures, 'ProcessPoolExecutor', FakeExecutor), \
                     patch.object(module, 'convert_task', return_value=[('some.tensor', b'payload', 'U8', [1])]) as mock_convert:
                    completed = {}
                    module.process_pending_tasks(
                        [task],
                        workers=2,
                        logger=logger,
                        completed=completed,
                        state_dir=state_dir,
                        total_tasks=1,
                        max_retries=2,
                    )
            finally:
                logger.handlers.clear()

            self.assertIn(task['task_id'], completed)
            self.assertTrue(mock_convert.called)
            self.assertEqual(FakeExecutor.submit_calls, 2)
            self.assertIn('falling back to sequential conversion', stream.getvalue())

    def test_converter_falls_back_after_wait_failure(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            state_dir = tmpdir / 'state'
            state_dir.mkdir()
            task = {
                'task_id': 'model.safetensors:some.tensor',
                'source_name': 'model.safetensors',
                'tensor_name': 'some.tensor',
                'meta': {
                    'dtype': 'F32',
                    'shape': [1],
                    'data_offsets': [0, 4],
                },
                'source_path': tmpdir / 'model.safetensors',
            }
            self.write_safetensors(tmpdir / 'model.safetensors', [('some.tensor', [0.0], [1], 'F32')])

            class FakeFuture:
                def __init__(self, result=None):
                    self._result = result

                def result(self):
                    return self._result

            class FakeExecutor:
                def __init__(self, max_workers):
                    self.max_workers = max_workers

                def __enter__(self):
                    return self

                def __exit__(self, exc_type, exc, tb):
                    return False

                def submit(self, fn, task, logger=None):
                    return FakeFuture(result=[('some.tensor', b'payload', 'U8', [1])])

            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            logger = logging.getLogger('test.converter.wait-fallback')
            logger.setLevel(logging.INFO)
            logger.propagate = False
            stream = io.StringIO()
            handler = logging.StreamHandler(stream)
            handler.setLevel(logging.INFO)
            logger.addHandler(handler)
            try:
                with patch.object(module.concurrent.futures, 'ProcessPoolExecutor', FakeExecutor), \
                     patch.object(module.concurrent.futures, 'wait', side_effect=module.concurrent.futures.process.BrokenProcessPool('simulated wait failure')), \
                     patch.object(module, 'convert_task', return_value=[('some.tensor', b'payload', 'U8', [1])]) as mock_convert:
                    completed = {}
                    module.process_pending_tasks(
                       [task],
                       workers=1,
                       logger=logger,
                       completed=completed,
                       state_dir=state_dir,
                       total_tasks=1,
                       max_retries=2,
                    )
            finally:
                logger.handlers.clear()

            self.assertIn(task['task_id'], completed)
            self.assertTrue(mock_convert.called)
            self.assertIn('falling back to sequential conversion', stream.getvalue())

    def test_converter_processes_oversized_tasks_sequentially_without_worker_pool(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            state_dir = tmpdir / 'state'
            state_dir.mkdir()
            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            task = {
                'task_id': 'model.safetensors:model.norm.weight',
                'source_name': 'model.safetensors',
                'tensor_name': 'model.norm.weight',
                'meta': {
                    'dtype': 'BF16',
                    'shape': [module.MAX_WORKER_POOL_TASK_BYTES // 4 + 1],
                    'data_offsets': [0, module.MAX_WORKER_POOL_TASK_BYTES // 2],
                },
                'source_path': tmpdir / 'model.safetensors',
            }

            logger = logging.getLogger('test.converter.oversized')
            logger.setLevel(logging.INFO)
            logger.propagate = False
            stream = io.StringIO()
            handler = logging.StreamHandler(stream)
            handler.setLevel(logging.INFO)
            logger.addHandler(handler)
            try:
                with patch.object(module.concurrent.futures, 'ProcessPoolExecutor') as mock_executor, \
                     patch.object(module, 'convert_task', return_value=[('model.norm.weight', b'payload', 'F32', [1])]) as mock_convert:
                    completed = {}
                    module.process_pending_tasks(
                        [task],
                        workers=4,
                        logger=logger,
                        completed=completed,
                        state_dir=state_dir,
                        total_tasks=1,
                    )
            finally:
                logger.handlers.clear()

            self.assertIn(task['task_id'], completed)
            mock_executor.assert_not_called()
            self.assertTrue(mock_convert.called)
            self.assertIn('processing 1 oversized task(s) sequentially', stream.getvalue())

    def test_converter_logs_failures_to_log_file(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'missing-input'
            output_dir = tmpdir / 'output'
            output_dir.mkdir()
            result = subprocess.run(
                [sys.executable, str(self.converter_script_path()), '--input', str(input_dir), '--output', str(output_dir)],
                text=True,
                capture_output=True,
            )
            self.assertNotEqual(result.returncode, 0)
            log_files = list(output_dir.glob('convert_qwen35_safetensors-*.log'))
            self.assertTrue(log_files)
            log_text = log_files[0].read_text(encoding='utf-8')
            self.assertIn('conversion failed', log_text)
            self.assertIn('FileNotFoundError', log_text)

    def test_converter_cleans_up_state_artifacts_after_success(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            state_dir = output_dir / '.state'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir, ['--state-dir', str(state_dir)])
            self.assertTrue((state_dir / '.conversion_complete').exists())
            self.assertFalse((state_dir / 'payloads').exists())
            self.assertEqual(list(state_dir.glob('*.json')), [])
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['dtype'], 'F32')

    def test_converter_deletes_each_source_shard_after_it_is_written(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            self.write_safetensors(
                input_dir / 'model2.safetensors',
                [
                    ('model.layers.0.mlp.experts.0.gate_proj.weight', [0.5, -0.6, 0.7, -0.8], [2, 2], 'F32'),
                ],
            )
            self.run_converter(input_dir, output_dir)
            self.assertFalse((input_dir / 'model.safetensors').exists())
            self.assertFalse((input_dir / 'model2.safetensors').exists())
            self.assertTrue((output_dir / 'model.safetensors').exists())
            self.assertTrue((output_dir / 'model2.safetensors').exists())
            model_header = self.read_header(output_dir / 'model.safetensors')
            model2_header = self.read_header(output_dir / 'model2.safetensors')
            self.assertIn('model.layers.0.self_attn.q_proj.weight', model_header)
            self.assertNotIn('model.layers.0.mlp.experts.0.gate_proj.weight', model_header)
            self.assertIn('model.layers.0.mlp.experts.0.gate_proj.weight', model2_header)
            self.assertNotIn('model.layers.0.self_attn.q_proj.weight', model2_header)

    def test_converter_migrates_legacy_state_dir_to_state_format(self):
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = Path(tmpdir)
            input_dir = tmpdir / 'input'
            output_dir = tmpdir / 'output'
            input_dir.mkdir()
            output_dir.mkdir()
            self.write_safetensors(
                input_dir / 'model.safetensors',
                [
                    ('model.layers.0.self_attn.q_proj.weight', [0.1, -0.2, 0.3, -0.4], [2, 2], 'F32'),
                ],
            )
            spec = importlib.util.spec_from_file_location('convert_qwen35_safetensors', self.converter_script_path())
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)
            legacy_state_dir = output_dir / '.conversion_state'
            legacy_state_dir.mkdir(parents=True, exist_ok=True)
            payload = struct.pack('<4B', 1, 2, 3, 4)
            qs_payload = struct.pack('<2f', 0.1, 0.2)
            task = {
                'task_id': 'model.safetensors:model.layers.0.self_attn.q_proj.weight',
                'source_name': 'model.safetensors',
                'tensor_name': 'model.layers.0.self_attn.q_proj.weight',
            }
            module.persist_task_result(
                legacy_state_dir,
                task,
                [
                    ('model.layers.0.self_attn.q_proj.weight', payload, 'U8', [2, 2]),
                    ('model.layers.0.self_attn.q_proj.weight.qs', qs_payload, 'F32', [2]),
                ],
            )
            self.run_converter(input_dir, output_dir)
            self.assertTrue((output_dir / '.state').exists())
            self.assertFalse(legacy_state_dir.exists())
            self.assertTrue((output_dir / '.state' / '.conversion_complete').exists())


if __name__ == '__main__':
    unittest.main()
