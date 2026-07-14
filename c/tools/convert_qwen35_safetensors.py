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
import logging
import math
import os
import shutil
import struct
import sys
import time
from datetime import datetime, timezone
from pathlib import Path


def read_safetensors_file(path):
    with open(path, 'rb') as fh:
        header_len = int.from_bytes(fh.read(8), 'little')
        header_bytes = fh.read(header_len)
        header = json.loads(header_bytes.decode('utf-8'))
        data = fh.read()
    return header, data


def decode_fp8_payload(payload, dtype):
    values = []
    dtype_name = dtype.lower()
    if 'e4m3' in dtype_name:
        mantissa_bits = 3
        exponent_bias = 7
    elif 'e5m2' in dtype_name:
        mantissa_bits = 2
        exponent_bias = 15
    else:
        raise ValueError('unsupported fp8 dtype %s' % dtype)
    exponent_bits = 8 - mantissa_bits - 1
    max_exponent = (1 << exponent_bits) - 1
    for byte in payload:
        sign = -1.0 if (byte & 0x80) else 1.0
        exponent = (byte >> mantissa_bits) & max_exponent
        mantissa = byte & ((1 << mantissa_bits) - 1)
        if exponent == 0:
            if mantissa == 0:
                values.append(0.0)
            else:
                values.append(sign * (2 ** (1 - exponent_bias)) * (mantissa / (2 ** mantissa_bits)))
        elif exponent == max_exponent:
            values.append(math.copysign(float('inf'), sign) if mantissa == 0 else float('nan'))
        else:
            values.append(sign * (2 ** (exponent - exponent_bias)) * (1.0 + mantissa / (2 ** mantissa_bits)))
    return values


def decode_tensor_payload(dtype, payload, shape, logger=None):
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
    if 'e4m3' in dtype.lower() or 'e5m2' in dtype.lower():
        values = decode_fp8_payload(payload, dtype)
        if logger is not None:
            logger.info('decoded %s tensor as fp8 (%d values)', dtype, len(values))
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


def setup_logger(output_path, log_path=None):
    logger = logging.getLogger('c.tools.convert_qwen35_safetensors')
    logger.setLevel(logging.INFO)
    logger.propagate = False
    if log_path is None:
        timestamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%SZ')
        log_path = output_path / ('convert_qwen35_safetensors-%s.log' % timestamp)
    else:
        log_path = Path(log_path).expanduser()
        if not log_path.is_absolute():
            log_path = output_path / log_path
    if logger.handlers:
        for handler in list(logger.handlers):
            logger.removeHandler(handler)
            handler.close()
    file_handler = logging.FileHandler(log_path, encoding='utf-8')
    file_handler.setFormatter(logging.Formatter('%(asctime)s %(levelname)s %(message)s'))
    stream_handler = logging.StreamHandler(sys.stderr)
    stream_handler.setFormatter(logging.Formatter('%(levelname)s %(message)s'))
    logger.addHandler(file_handler)
    logger.addHandler(stream_handler)
    return logger, log_path


def render_progress(current, total, label):
    if total <= 0:
        return
    if not sys.stderr.isatty():
        if current == total:
            sys.stderr.write('%s: %d/%d\n' % (label, current, total))
        return
    width = 24
    filled = int(width * current / total)
    bar = '#' * filled + '-' * (width - filled)
    percent = int(100 * current / total)
    sys.stderr.write('\r%s [%s] %d/%d (%d%%)' % (label, bar, current, total, percent))
    sys.stderr.flush()
    if current >= total:
        sys.stderr.write('\n')


def convert_tensor_payload(tensor_name, meta, payload, logger):
    shape = list(meta['shape'])
    dtype = meta['dtype']
    values = decode_tensor_payload(dtype, payload, shape, logger=logger)
    quant_kind = should_quantize(tensor_name)
    if len(shape) == 2 and quant_kind:
        out_dim, in_dim = shape
        if quant_kind == 'int8':
            packed, scales = quantize_int8(values, out_dim, in_dim)
            logger.info('quantized %s as int8 (%dx%d)', tensor_name, out_dim, in_dim)
            return [
                (tensor_name, packed, 'U8', shape),
                (tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]),
            ]
        packed, scales = quantize_int4(values, out_dim, in_dim)
        logger.info('quantized %s as int4 (%dx%d)', tensor_name, out_dim, in_dim)
        return [
            (tensor_name, packed, 'U8', shape),
            (tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]),
        ]
    logger.info('copied %s as F32 (shape=%s)', tensor_name, shape)
    return [(tensor_name, encode_tensor_payload('F32', values), 'F32', shape)]


def convert_safetensors(path, out_path, logger):
    header, data = read_safetensors_file(path)
    output_tensors = []
    tensor_names = [name for name in header if name != '__metadata__']
    logger.info('processing safetensors file %s (%d tensors)', path.name, len(tensor_names))
    for index, tensor_name in enumerate(tensor_names, 1):
        meta = header[tensor_name]
        shape = meta['shape']
        dtype = meta['dtype']
        offsets = meta['data_offsets']
        raw_payload = data[offsets[0]:offsets[1]]
        logger.info('processing tensor %d/%d %s (dtype=%s shape=%s)', index, len(tensor_names), tensor_name, dtype, shape)
        output_tensors.extend(convert_tensor_payload(tensor_name, meta, raw_payload, logger))
        render_progress(index, len(tensor_names), 'converting %s' % path.name)
    write_safetensors_file(out_path, output_tensors)
    logger.info('wrote %s', out_path)


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
    parser.add_argument('--log-file', default=None, help='optional path for the conversion log file')
    args = parser.parse_args()

    input_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()
    output_path.mkdir(parents=True, exist_ok=True)
    logger, log_path = setup_logger(output_path, args.log_file)
    logger.info('converter starting: input=%s output=%s', input_path, output_path)
    logger.info('log file: %s', log_path)
    if input_path.is_dir():
        for name in ['config.json', 'tokenizer.json']:
            src = input_path / name
            if src.exists():
                shutil.copy2(src, output_path / name)
                logger.info('copied %s', src.name)
    else:
        parent = input_path.parent
        for name in ['config.json', 'tokenizer.json']:
            src = parent / name
            if src.exists():
                shutil.copy2(src, output_path / name)
                logger.info('copied %s', src.name)

    files = discover_safetensors(input_path)
    logger.info('discovered %d safetensors file(s)', len(files))
    if len(files) == 1:
        out_file = output_path / 'model.safetensors'
        convert_safetensors(files[0], out_file, logger)
        return 0

    merged_tensors = []
    for index, path in enumerate(files, 1):
        logger.info('processing shard %d/%d %s', index, len(files), path.name)
        header, data = read_safetensors_file(path)
        tensor_names = [name for name in header if name != '__metadata__']
        for tensor_index, tensor_name in enumerate(tensor_names, 1):
            meta = header[tensor_name]
            shape = meta['shape']
            dtype = meta['dtype']
            offsets = meta['data_offsets']
            raw_payload = data[offsets[0]:offsets[1]]
            logger.info('processing tensor %d/%d %s (dtype=%s shape=%s)', tensor_index, len(tensor_names), tensor_name, dtype, shape)
            merged_tensors.extend(convert_tensor_payload(tensor_name, meta, raw_payload, logger))
            render_progress(tensor_index, len(tensor_names), 'converting %s' % path.name)
        render_progress(index, len(files), 'shards')
    write_safetensors_file(output_path / 'model.safetensors', merged_tensors)
    logger.info('wrote merged safetensors %s', output_path / 'model.safetensors')
    return 0


if __name__ == '__main__':
    sys.exit(main())
