#!/usr/bin/env python3
"""Convert standard safetensors tensors to the Qwen3.5 quantized layout used by qwen35_moe.

The converter writes tensors as U8 payloads plus per-row F32 scales. It targets the
layout expected by c/qwen35_moe.c:
  - int8 quantization for embed/lm_head/attention/shared-expert tensors
  - int4 quantization for routed expert tensors
  - F32 tensors are copied through unchanged
"""
import argparse
import json
import math
import os
import shutil
import struct
import sys
from pathlib import Path


def read_safetensors_file(path):
    with open(path, 'rb') as fh:
        header_len = int.from_bytes(fh.read(8), 'little')
        header_bytes = fh.read(header_len)
        header = json.loads(header_bytes.decode('utf-8'))
        data = fh.read()
    return header, data


def decode_tensor_payload(dtype, payload, shape):
    if dtype == 'F32':
        if not shape:
            return []
        elems = math.prod(shape)
        return list(struct.unpack('<%df' % elems, payload))
    if dtype == 'F16':
        values = []
        for i in range(0, len(payload), 2):
            h = struct.unpack_from('<H', payload, i)[0]
            sign = -1.0 if (h & 0x8000) else 1.0
            exp = (h >> 10) & 0x1f
            man = h & 0x3ff
            if exp == 0:
                if man == 0:
                    values.append(0.0)
                else:
                    values.append(sign * (2 ** -24) * man)
            elif exp == 0x1f:
                values.append(float('inf') if man == 0 else float('nan'))
            else:
                values.append(sign * (2 ** (exp - 15)) * (1.0 + man / 1024.0))
        return values
    if dtype == 'BF16':
        values = []
        for i in range(0, len(payload), 2):
            h = struct.unpack_from('<H', payload, i)[0]
            u = (h << 16) & 0xFFFFFFFF
            values.append(struct.unpack('<f', struct.pack('<I', u))[0])
        return values
    raise ValueError('unsupported dtype %s' % dtype)


def encode_tensor_payload(dtype, values):
    if dtype == 'F32':
        return struct.pack('<%df' % len(values), *values)
    raise ValueError('unsupported dtype %s' % dtype)


def quantize_int8(values, out_dim, in_dim):
    scales = []
    packed = bytearray(out_dim * in_dim)
    for row in range(out_dim):
        start = row * in_dim
        row_vals = values[start:start + in_dim]
        amax = max(abs(v) for v in row_vals) if row_vals else 0.0
        scale = max(amax / 127.0, 1e-8)
        scales.append(scale)
        for col, value in enumerate(row_vals):
            quantized = int(round(value / scale))
            quantized = max(-128, min(127, quantized))
            packed[row * in_dim + col] = quantized & 0xFF
    return bytes(packed), scales


def quantize_int4(values, out_dim, in_dim):
    scales = []
    packed = bytearray(out_dim * ((in_dim + 1) // 2))
    for row in range(out_dim):
        start = row * in_dim
        row_vals = values[start:start + in_dim]
        amax = max(abs(v) for v in row_vals) if row_vals else 0.0
        scale = max(amax / 7.0, 1e-8)
        scales.append(scale)
        for col, value in enumerate(row_vals):
            quantized = int(round(value / scale))
            quantized = max(-8, min(7, quantized))
            packed_byte = row * ((in_dim + 1) // 2) + (col // 2)
            if col % 2 == 0:
                packed[packed_byte] = (quantized + 8) & 0x0F
            else:
                packed[packed_byte] |= ((quantized + 8) & 0x0F) << 4
    return bytes(packed), scales


def should_quantize(name):
    if not name.endswith('.weight'):
        return None
    if '.mlp.experts.' in name:
        return 'int4'
    if (
        name.startswith('model.embed_tokens')
        or name.startswith('lm_head')
        or '.self_attn.' in name
        or '.shared_expert.' in name
        or '.linear_attn.' in name
        or (name.startswith('model.layers.') and '.mlp.experts.' not in name)
        or (name.startswith('model.language_model.layers.') and '.mlp.experts.' not in name)
    ):
        return 'int8'
    return None


def convert_safetensors(path, out_path):
    header, data = read_safetensors_file(path)
    output_tensors = []

    for tensor_name in header:
        if tensor_name == '__metadata__':
            continue
        # Each tuple stores (tensor_name, payload, dtype, shape).
        meta = header[tensor_name]
        shape = meta['shape']
        dtype = meta['dtype']
        offsets = meta['data_offsets']
        raw_payload = data[offsets[0]:offsets[1]]
        values = decode_tensor_payload(dtype, raw_payload, shape)
        quant_kind = should_quantize(tensor_name)
        if len(shape) == 2 and quant_kind:
            out_dim, in_dim = shape
            if quant_kind == 'int8':
                packed, scales = quantize_int8(values, out_dim, in_dim)
                output_tensors.append((tensor_name, packed, 'U8', shape))
                output_tensors.append((tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]))
            else:
                packed, scales = quantize_int4(values, out_dim, in_dim)
                output_tensors.append((tensor_name, packed, 'U8', shape))
                output_tensors.append((tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]))
        else:
            output_tensors.append((tensor_name, encode_tensor_payload('F32', values), 'F32', shape))

    write_safetensors_file(out_path, output_tensors)


def write_safetensors_file(path, tensors):
    header = {}
    data_offset = 0
    for name, payload, dtype, shape in tensors:
        header[name] = {
            'dtype': dtype,
            'shape': list(shape),
            'data_offsets': [data_offset, data_offset + len(payload)],
        }
        data_offset += len(payload)
    header_bytes = json.dumps(header, separators=(',', ':')).encode('utf-8')
    with open(path, 'wb') as fh:
        fh.write(len(header_bytes).to_bytes(8, 'little'))
        fh.write(header_bytes)
        for _, payload, _, _ in tensors:
            fh.write(payload)


def discover_safetensors(input_path):
    if input_path.is_dir():
        files = sorted(input_path.glob('*.safetensors'))
        if not files:
            raise FileNotFoundError('no safetensors files found in %s' % input_path)
        return files
    if input_path.is_file() and input_path.suffix == '.safetensors':
        return [input_path]
    raise FileNotFoundError('input path is not a safetensors file or directory')


def main():
    parser = argparse.ArgumentParser(description='Convert standard safetensors to the qwen35_moe quantized layout')
    parser.add_argument('--input', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args()

    input_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()
    output_path.mkdir(parents=True, exist_ok=True)
    if input_path.is_dir():
        for name in ['config.json', 'tokenizer.json']:
            src = input_path / name
            if src.exists():
                shutil.copy2(src, output_path / name)
    else:
        parent = input_path.parent
        for name in ['config.json', 'tokenizer.json']:
            src = parent / name
            if src.exists():
                shutil.copy2(src, output_path / name)

    files = discover_safetensors(input_path)
    if len(files) == 1:
        out_file = output_path / 'model.safetensors'
        convert_safetensors(files[0], out_file)
        return 0

    merged_tensors = []
    for path in files:
        header, data = read_safetensors_file(path)
        for tensor_name in header:
            if tensor_name == '__metadata__':
                continue
            meta = header[tensor_name]
            shape = meta['shape']
            dtype = meta['dtype']
            offsets = meta['data_offsets']
            raw_payload = data[offsets[0]:offsets[1]]
            values = decode_tensor_payload(dtype, raw_payload, shape)
            quant_kind = should_quantize(tensor_name)
            if len(shape) == 2 and quant_kind:
                out_dim, in_dim = shape
                if quant_kind == 'int8':
                    packed, scales = quantize_int8(values, out_dim, in_dim)
                    merged_tensors.append((tensor_name, packed, 'U8', shape))
                    merged_tensors.append((tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]))
                else:
                    packed, scales = quantize_int4(values, out_dim, in_dim)
                    merged_tensors.append((tensor_name, packed, 'U8', shape))
                    merged_tensors.append((tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]))
            else:
                merged_tensors.append((tensor_name, encode_tensor_payload('F32', values), 'F32', shape))
    write_safetensors_file(output_path / 'model.safetensors', merged_tensors)
    return 0


if __name__ == '__main__':
    sys.exit(main())
