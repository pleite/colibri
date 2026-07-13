import json
import os
import shutil
import struct
import subprocess
import tempfile
import unittest
from pathlib import Path


class Qwen35QuantConverterTest(unittest.TestCase):
    def write_safetensors(self, path, tensors):
        header = {}
        data_offset = 0
        payloads = []
        for name, values, shape, dtype in tensors:
            payload = struct.pack('<%df' % len(values), *values)
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

    def read_header(self, path):
        with open(path, 'rb') as fh:
            header_len = int.from_bytes(fh.read(8), 'little')
            return json.loads(fh.read(header_len).decode('utf-8'))

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
            subprocess.run(
                ['python3', str(Path(__file__).resolve().parents[1] / 'tools' / 'convert_qwen35_safetensors.py'), '--input', str(input_dir), '--output', str(output_dir)],
                check=True,
            )
            header = self.read_header(output_dir / 'model.safetensors')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight']['shape'], [2, 4])
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['dtype'], 'F32')
            self.assertEqual(header['model.layers.0.self_attn.q_proj.weight.qs']['shape'], [2])
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight']['dtype'], 'U8')
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight']['shape'], [2, 4])
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight.qs']['dtype'], 'F32')
            self.assertEqual(header['model.layers.0.mlp.experts.0.gate_proj.weight.qs']['shape'], [2])


if __name__ == '__main__':
    unittest.main()
