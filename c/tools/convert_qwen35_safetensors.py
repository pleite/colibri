#!/usr/bin/env python3
"""Convert standard safetensors tensors to the Qwen3.5 quantized layout used by qwen35_moe.

The converter writes tensors as U8 payloads plus per-row F32 scales. It targets the
layout expected by c/qwen35_moe.c:
  - int8 quantization for embed/lm_head/attention/shared-expert tensors
  - int4 quantization for routed expert tensors
  - F32 tensors are copied through unchanged
"""
import argparse
import base64
import concurrent.futures
import json
import logging
import math
import os
import shutil
import struct
import sys
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


def _finite_amax(values):
    finite_values = [abs(v) for v in values if math.isfinite(v)]
    return max(finite_values) if finite_values else 0.0


def _quantize_value(value, scale, max_value):
    if not math.isfinite(value):
        value = 0.0
    quantized = int(round(value / scale))
    return max(-max_value, min(max_value, quantized))


def quantize_int8(values, out_dim, in_dim):
    scales = []
    packed = bytearray(out_dim * in_dim)
    for row in range(out_dim):
        start = row * in_dim
        row_vals = values[start:start + in_dim]
        amax = _finite_amax(row_vals)
        scale = max(amax / 127.0, 1e-8)
        scales.append(scale)
        for col, value in enumerate(row_vals):
            quantized = _quantize_value(value, scale, 127)
            packed[row * in_dim + col] = quantized & 0xFF
    return bytes(packed), scales


def quantize_int4(values, out_dim, in_dim):
    scales = []
    packed = bytearray(out_dim * ((in_dim + 1) // 2))
    for row in range(out_dim):
        start = row * in_dim
        row_vals = values[start:start + in_dim]
        amax = _finite_amax(row_vals)
        scale = max(amax / 7.0, 1e-8)
        scales.append(scale)
        for col, value in enumerate(row_vals):
            quantized = _quantize_value(value, scale, 7)
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


def expected_output_names(tensor_name, shape):
    quant_kind = should_quantize(tensor_name)
    if len(shape) == 2 and quant_kind:
        return [tensor_name, tensor_name + '.qs']
    return [tensor_name]


def convert_tensor_payload(tensor_name, meta, payload, logger=None):
    shape = list(meta['shape'])
    dtype = meta['dtype']
    values = decode_tensor_payload(dtype, payload, shape, logger=logger)
    quant_kind = should_quantize(tensor_name)
    if len(shape) == 2 and quant_kind:
        out_dim, in_dim = shape
        if quant_kind == 'int8':
            packed, scales = quantize_int8(values, out_dim, in_dim)
            if logger is not None:
                logger.info('quantized %s as int8 (%dx%d)', tensor_name, out_dim, in_dim)
            return [
                (tensor_name, packed, 'U8', shape),
                (tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]),
            ]
        packed, scales = quantize_int4(values, out_dim, in_dim)
        if logger is not None:
            logger.info('quantized %s as int4 (%dx%d)', tensor_name, out_dim, in_dim)
        return [
            (tensor_name, packed, 'U8', shape),
            (tensor_name + '.qs', struct.pack('<%df' % len(scales), *scales), 'F32', [len(scales)]),
        ]
    if logger is not None:
        logger.info('copied %s as F32 (shape=%s)', tensor_name, shape)
    return [(tensor_name, encode_tensor_payload('F32', values), 'F32', shape)]


def encode_output_tensors(output_tensors):
    return [{
        'name': name,
        'payload': base64.b64encode(payload).decode('ascii'),
        'dtype': dtype,
        'shape': list(shape),
    } for name, payload, dtype, shape in output_tensors]


def decode_output_tensors(payloads):
    return [(entry['name'], base64.b64decode(entry['payload']), entry['dtype'], entry['shape']) for entry in payloads]


def write_safetensors_file(path, tensors):
    path.parent.mkdir(parents=True, exist_ok=True)
    tmp_path = path.with_suffix(path.suffix + '.tmp')
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
    with open(tmp_path, 'wb') as fh:
        fh.write(len(header_bytes).to_bytes(8, 'little'))
        fh.write(header_bytes)
        for _, payload, _, _ in tensors:
            fh.write(payload)
    os.replace(tmp_path, path)


def discover_safetensors(input_path):
    if input_path.is_dir():
        files = sorted(input_path.glob('*.safetensors'))
        if not files:
            raise FileNotFoundError('no safetensors files found in %s' % input_path)
        return files
    if input_path.is_file() and input_path.suffix == '.safetensors':
        return [input_path]
    raise FileNotFoundError('input path is not a safetensors file or directory')


def build_tasks(input_path):
    files = discover_safetensors(input_path)
    tasks = []
    for path in files:
        header, data = read_safetensors_file(path)
        tensor_names = [name for name in header if name != '__metadata__']
        for tensor_name in tensor_names:
            meta = header[tensor_name]
            offsets = meta['data_offsets']
            payload = data[offsets[0]:offsets[1]]
            task_id = '%s:%s' % (path.name, tensor_name)
            tasks.append({
                'task_id': task_id,
                'source_name': path.name,
                'tensor_name': tensor_name,
                'meta': meta,
                'payload': payload,
                'index': len(tasks),
            })
    return files, tasks


def read_existing_output_payloads(path):
    if not path.exists():
        return {}
    header, data = read_safetensors_file(path)
    entries = {}
    for tensor_name, meta in header.items():
        if tensor_name == '__metadata__':
            continue
        offsets = meta['data_offsets']
        payload = data[offsets[0]:offsets[1]]
        entries[tensor_name] = (tensor_name, payload, meta['dtype'], list(meta['shape']))
    return entries


def load_resumed_outputs(state_dir, out_file, tasks):
    completed = {}
    if state_dir.exists():
        for state_path in sorted(state_dir.glob('*.json')):
            try:
                payload = json.loads(state_path.read_text(encoding='utf-8'))
            except (json.JSONDecodeError, OSError):
                continue
            task_id = payload.get('task_id')
            if not task_id:
                continue
            if task_id not in {task['task_id'] for task in tasks}:
                continue
            completed[task_id] = decode_output_tensors(payload.get('output_tensors', []))
    existing = read_existing_output_payloads(out_file)
    for task in tasks:
        expected = expected_output_names(task['tensor_name'], task['meta']['shape'])
        if task['task_id'] in completed:
            continue
        if all(name in existing for name in expected):
            completed[task['task_id']] = [existing[name] for name in expected]
    return completed


def persist_task_result(state_dir, task, output_tensors):
    state_dir.mkdir(parents=True, exist_ok=True)
    state_path = state_dir / ('%s.json' % task['task_id'].replace(os.sep, '_'))
    payload = {
        'task_id': task['task_id'],
        'source_name': task['source_name'],
        'tensor_name': task['tensor_name'],
        'output_tensors': encode_output_tensors(output_tensors),
    }
    state_path.write_text(json.dumps(payload, sort_keys=True), encoding='utf-8')


def convert_task(task, logger=None):
    output_tensors = convert_tensor_payload(task['tensor_name'], task['meta'], task['payload'], logger=logger)
    return output_tensors


def write_final_output(out_file, tasks, completed):
    output_tensors = []
    for task in tasks:
        task_entries = completed.get(task['task_id'])
        if not task_entries:
            raise ValueError('missing completed output for %s' % task['task_id'])
        output_tensors.extend(task_entries)
    write_safetensors_file(out_file, output_tensors)


def main():
    parser = argparse.ArgumentParser(description='Convert standard safetensors to the qwen35_moe quantized layout')
    parser.add_argument('--input', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--log-file', default=None, help='optional path for the conversion log file')
    parser.add_argument('--workers', type=int, default=None, help='number of worker processes; defaults to all CPU cores')
    parser.add_argument('--state-dir', default=None, help='directory for per-tensor resume checkpoints')
    args = parser.parse_args()

    input_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()
    output_path.mkdir(parents=True, exist_ok=True)
    out_file = output_path / 'model.safetensors'
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

    files, tasks = build_tasks(input_path)
    logger.info('discovered %d safetensors file(s) and %d tensor task(s)', len(files), len(tasks))

    state_dir = Path(args.state_dir).expanduser().resolve() if args.state_dir else output_path / '.conversion_state'
    state_dir.mkdir(parents=True, exist_ok=True)
    completed = load_resumed_outputs(state_dir, out_file, tasks)
    logger.info('resumed %d existing tensor task(s)', len(completed))

    pending_tasks = [task for task in tasks if task['task_id'] not in completed]
    if pending_tasks:
        workers = max(1, args.workers if args.workers is not None else (os.cpu_count() or 1))
        logger.info('using %d worker(s) for parallel conversion', workers)
        with concurrent.futures.ProcessPoolExecutor(max_workers=workers) as executor:
            future_map = {executor.submit(convert_task, task): task for task in pending_tasks}
            for future in concurrent.futures.as_completed(future_map):
                task = future_map[future]
                output_tensors = future.result()
                completed[task['task_id']] = output_tensors
                persist_task_result(state_dir, task, output_tensors)
                for entry in output_tensors:
                    if len(output_tensors) > 1:
                        logger.info('wrote %s', entry[0])
                render_progress(len(completed), len(tasks), 'converting %s' % task['source_name'])
    if not completed:
        raise RuntimeError('no tensors were converted')
    if len(completed) != len(tasks):
        missing = [task['task_id'] for task in tasks if task['task_id'] not in completed]
        raise RuntimeError('failed to complete tasks: %s' % ', '.join(missing))

    write_final_output(out_file, tasks, completed)
    logger.info('wrote %s', out_file)
    return 0


if __name__ == '__main__':
    sys.exit(main())
