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
import re
import shutil
import struct
import sys
from datetime import datetime, timezone
from pathlib import Path


INT8_MIN_VALUE = -128
INT8_MAX_VALUE = 127
INT4_MIN_VALUE = -8
INT4_MAX_VALUE = 7
METADATA_KEY = '__metadata__'
FP8_E4M3_PATTERN = re.compile(r'(?:f8|fp8|float8)?(?:_)?e4m3')
FP8_E5M2_PATTERN = re.compile(r'(?:f8|fp8|float8)?(?:_)?e5m2')


def read_safetensors_file(path):
    with open(path, 'rb') as fh:
        header_len = int.from_bytes(fh.read(8), 'little')
        header_bytes = fh.read(header_len)
        header = json.loads(header_bytes.decode('utf-8'))
        data = fh.read()
    return header, data


def read_safetensors_header(path):
    with open(path, 'rb') as fh:
        header_len = int.from_bytes(fh.read(8), 'little')
        header_bytes = fh.read(header_len)
        return json.loads(header_bytes.decode('utf-8'))


def read_safetensors_tensor_payload(path, meta):
    offsets = meta['data_offsets']
    with open(path, 'rb') as fh:
        fh.seek(offsets[0])
        return fh.read(offsets[1] - offsets[0])


def fp8_format(dtype):
    dtype_name = str(dtype).lower()
    if FP8_E4M3_PATTERN.fullmatch(dtype_name):
        return 'e4m3'
    if FP8_E5M2_PATTERN.fullmatch(dtype_name):
        return 'e5m2'
    return None


def is_fp8_dtype(dtype):
    return fp8_format(dtype) is not None


def decode_fp8_payload(payload, dtype):
    values = []
    fmt = fp8_format(dtype)
    if fmt == 'e4m3':
        mantissa_bits = 3
        exponent_bias = 7
    elif fmt == 'e5m2':
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
    if is_fp8_dtype(dtype):
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


def _quantize_value(value, scale, min_value, max_value):
    quantized = int(round(value / scale))
    return max(min_value, min(max_value, quantized))


def quantize_int8(values, out_dim, in_dim, logger=None, tensor_name=None):
    scales = []
    packed = bytearray(out_dim * in_dim)
    non_finite_count = 0
    for row in range(out_dim):
        start = row * in_dim
        row_vals = values[start:start + in_dim]
        amax = _finite_amax(row_vals)
        scale = max(amax / 127.0, 1e-8)
        scales.append(scale)
        for col, value in enumerate(row_vals):
            if not math.isfinite(value):
                non_finite_count += 1
                value = 0.0
            quantized = _quantize_value(value, scale, INT8_MIN_VALUE, INT8_MAX_VALUE)
            packed[row * in_dim + col] = quantized & 0xFF
    if logger is not None and non_finite_count:
        logger.warning('substituted %d non-finite value(s) in %s with 0.0', non_finite_count, tensor_name or 'tensor')
    return bytes(packed), scales


def quantize_int4(values, out_dim, in_dim, logger=None, tensor_name=None):
    scales = []
    packed = bytearray(out_dim * ((in_dim + 1) // 2))
    non_finite_count = 0
    for row in range(out_dim):
        start = row * in_dim
        row_vals = values[start:start + in_dim]
        amax = _finite_amax(row_vals)
        scale = max(amax / 7.0, 1e-8)
        scales.append(scale)
        for col, value in enumerate(row_vals):
            if not math.isfinite(value):
                non_finite_count += 1
                value = 0.0
            quantized = _quantize_value(value, scale, INT4_MIN_VALUE, INT4_MAX_VALUE)
            packed_byte = row * ((in_dim + 1) // 2) + (col // 2)
            if col % 2 == 0:
                packed[packed_byte] = (quantized + 8) & 0x0F
            else:
                packed[packed_byte] |= ((quantized + 8) & 0x0F) << 4
    if logger is not None and non_finite_count:
        logger.warning('substituted %d non-finite value(s) in %s with 0.0', non_finite_count, tensor_name or 'tensor')
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
        timestamp = datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S.%f') + 'Z'
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
    shape = meta['shape']
    dtype = meta['dtype']
    validate_payload_length(dtype, len(payload), shape, tensor_name=tensor_name)
    values = decode_tensor_payload(dtype, payload, shape, logger=logger)
    quant_kind = should_quantize(tensor_name)
    if len(shape) == 2 and quant_kind:
        out_dim, in_dim = shape
        if quant_kind == 'int8':
            packed, scales = quantize_int8(values, out_dim, in_dim, logger=logger, tensor_name=tensor_name)
            validate_payload_length('U8', len(packed), shape, quant_kind='int8', tensor_name=tensor_name)
            scales_payload = struct.pack('<%df' % len(scales), *scales)
            validate_payload_length('F32', len(scales_payload), [len(scales)], tensor_name=tensor_name + '.qs')
            if logger is not None:
                logger.info('quantized %s as int8 (%dx%d)', tensor_name, out_dim, in_dim)
            return [
                (tensor_name, packed, 'U8', shape),
                (tensor_name + '.qs', scales_payload, 'F32', [len(scales)]),
            ]
        packed, scales = quantize_int4(values, out_dim, in_dim, logger=logger, tensor_name=tensor_name)
        validate_payload_length('U8', len(packed), shape, quant_kind='int4', tensor_name=tensor_name)
        scales_payload = struct.pack('<%df' % len(scales), *scales)
        validate_payload_length('F32', len(scales_payload), [len(scales)], tensor_name=tensor_name + '.qs')
        if logger is not None:
            logger.info('quantized %s as int4 (%dx%d)', tensor_name, out_dim, in_dim)
        return [
            (tensor_name, packed, 'U8', shape),
            (tensor_name + '.qs', scales_payload, 'F32', [len(scales)]),
        ]
    if logger is not None:
        logger.info('copied %s as F32 (shape=%s)', tensor_name, shape)
    output_payload = encode_tensor_payload('F32', values)
    validate_payload_length('F32', len(output_payload), shape, tensor_name=tensor_name)
    return [(tensor_name, output_payload, 'F32', shape)]


def encode_output_tensors(output_tensors):
    return [{
        'name': name,
        'payload': base64.b64encode(payload).decode('ascii'),
        'dtype': dtype,
        'shape': list(shape),
    } for name, payload, dtype, shape in output_tensors]


def decode_output_tensors(payloads):
    return [(entry['name'], base64.b64decode(entry['payload']), entry['dtype'], entry['shape']) for entry in payloads]


def expected_payload_size(dtype, shape, quant_kind=None):
    shape = tuple(shape)
    elem_count = math.prod(shape) if shape else 1
    dtype_name = dtype.lower()
    if dtype == 'F32':
        return 4 * elem_count
    if dtype in {'F16', 'BF16'}:
        return 2 * elem_count
    if dtype == 'U8':
        if quant_kind == 'int4':
            if len(shape) != 2:
                raise ValueError('int4 quantization expects a 2D tensor')
            out_dim, in_dim = shape
            return out_dim * ((in_dim + 1) // 2)
        if quant_kind == 'int8':
            if len(shape) != 2:
                raise ValueError('int8 quantization expects a 2D tensor')
            out_dim, in_dim = shape
            return out_dim * in_dim
        return elem_count
    if is_fp8_dtype(dtype):
        return elem_count
    raise ValueError('unsupported dtype %s' % dtype)


def validate_payload_length(dtype, payload_length, shape, quant_kind=None, tensor_name=None):
    expected = expected_payload_size(dtype, shape, quant_kind=quant_kind)
    if payload_length != expected:
        name = tensor_name or '<unknown>'
        raise ValueError('%s payload length %d does not match expected %d for dtype=%s shape=%s' % (name, payload_length, expected, dtype, shape))


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


def build_source_task_groups(input_path):
    files = discover_safetensors(input_path)
    source_groups = []
    for path in files:
        header = read_safetensors_header(path)
        tensor_names = [name for name in header if name != '__metadata__']
        tasks = []
        for tensor_name in tensor_names:
            meta = header[tensor_name]
            task_id = '%s:%s' % (path.name, tensor_name)
            tasks.append({
                'task_id': task_id,
                'source_path': path,
                'source_name': path.name,
                'tensor_name': tensor_name,
                'meta': meta,
                'index': len(tasks),
            })
        source_groups.append({
            'source_path': path,
            'source_name': path.name,
            'tasks': tasks,
        })
    return files, source_groups


def build_tasks(input_path):
    files, source_groups = build_source_task_groups(input_path)
    tasks = []
    for group in source_groups:
        tasks.extend(group['tasks'])
    return files, tasks


def read_existing_output_payloads(path, tensor_names=None):
    if not path.exists():
        return {}
    header = read_safetensors_header(path)
    entries = {}
    names = tensor_names or [name for name in header if name != '__metadata__']
    for tensor_name in names:
        if tensor_name not in header:
            continue
        meta = header[tensor_name]
        payload = read_safetensors_tensor_payload(path, meta)
        entries[tensor_name] = (tensor_name, payload, meta['dtype'], list(meta['shape']))
    return entries


def output_file_for_source(output_path, source_name):
    return Path(output_path) / Path(source_name).name


def load_resumed_outputs(state_dir, output_path, tasks):
    completed = {}
    task_ids = {task['task_id'] for task in tasks}
    if state_dir.exists():
        for state_path in sorted(state_dir.glob('*.json')):
            try:
                payload = json.loads(state_path.read_text(encoding='utf-8'))
            except (json.JSONDecodeError, OSError):
                continue
            task_id = payload.get('task_id')
            if not task_id or task_id not in task_ids:
                continue
            completed[task_id] = {'kind': 'state', 'state_path': state_path}
    for task in tasks:
        expected = expected_output_names(task['tensor_name'], task['meta']['shape'])
        if task['task_id'] in completed:
            continue
        out_file = output_file_for_source(output_path, task['source_name'])
        existing = read_existing_output_payloads(out_file, expected)
        if all(name in existing for name in expected):
            completed[task['task_id']] = {'kind': 'output_file', 'output_file': str(out_file), 'tensor_names': expected}
    return completed


def state_file_name_for_task_id(task_id):
    return '%s.json' % task_id.replace(':', '_')


def sanitize_tensor_name(name):
    return ''.join(ch if ch.isalnum() or ch in '._-' else '_' for ch in name)


def sanitize_task_id(task_id):
    return sanitize_tensor_name(task_id.replace(':', '_'))


def persist_task_result(state_dir, task, output_tensors):
    state_dir.mkdir(parents=True, exist_ok=True)
    payload_dir = state_dir / 'payloads'
    payload_dir.mkdir(parents=True, exist_ok=True)
    state_path = state_dir / state_file_name_for_task_id(task['task_id'])
    encoded_tensors = []
    for name, payload, dtype, shape in output_tensors:
        safe_name = sanitize_tensor_name(name)
        safe_task_id = sanitize_task_id(task['task_id'])
        payload_path = payload_dir / ('%s__%s.bin' % (safe_task_id, safe_name))
        payload_path.write_bytes(payload)
        encoded_tensors.append({
            'name': name,
            'payload_path': payload_path.name,
            'dtype': dtype,
            'shape': list(shape),
        })
    payload = {
        'task_id': task['task_id'],
        'source_name': task['source_name'],
        'tensor_name': task['tensor_name'],
        'output_tensors': encoded_tensors,
    }
    state_path.write_text(json.dumps(payload, sort_keys=True), encoding='utf-8')
    return state_path


def load_state_output_tensors(state_path, state_dir):
    payload = json.loads(state_path.read_text(encoding='utf-8'))
    tensors = []
    for entry in payload.get('output_tensors', []):
        payload_path = state_dir / 'payloads' / entry['payload_path']
        tensors.append((entry['name'], payload_path.read_bytes(), entry['dtype'], entry['shape']))
    return tensors


def load_state_output_tensors_metadata(state_path, state_dir):
    payload = json.loads(state_path.read_text(encoding='utf-8'))
    tensors = []
    for entry in payload.get('output_tensors', []):
        payload_path = state_dir / 'payloads' / entry['payload_path']
        tensors.append((entry['name'], payload_path, entry['dtype'], entry['shape']))
    return tensors


def load_completed_task_tensors(task, completed_entry, state_dir, output_path):
    expected = expected_output_names(task['tensor_name'], task['meta']['shape'])
    out_file = output_file_for_source(output_path, task['source_name'])
    if completed_entry['kind'] == 'state':
        state_path = completed_entry['state_path']
        if state_path.exists():
            try:
                return load_state_output_tensors_metadata(state_path, state_dir)
            except (FileNotFoundError, OSError):
                pass
        if out_file.exists():
            existing = read_existing_output_payloads(out_file, expected)
            if all(name in existing for name in expected):
                return [existing[name] for name in expected]
        raise ValueError('state artifacts are missing for %s' % task['task_id'])
    existing = read_existing_output_payloads(out_file, expected)
    if all(name in existing for name in expected):
        return [existing[name] for name in expected]
    raise ValueError('missing completed output for %s' % task['task_id'])


def cleanup_task_state_artifacts(state_dir, task_id):
    state_path = state_dir / state_file_name_for_task_id(task_id)
    if not state_path.exists():
        return
    try:
        payload = json.loads(state_path.read_text(encoding='utf-8'))
    except (json.JSONDecodeError, OSError):
        payload = {}
    for entry in payload.get('output_tensors', []):
        payload_path = state_dir / 'payloads' / entry['payload_path']
        if payload_path.exists():
            payload_path.unlink()
    state_path.unlink(missing_ok=True)


def convert_task(task, logger=None):
    payload = read_safetensors_tensor_payload(task['source_path'], task['meta'])
    output_tensors = convert_tensor_payload(task['tensor_name'], task['meta'], payload, logger=logger)
    return output_tensors


def write_completion_marker(state_dir, output_files, tensor_names):
    marker_path = state_dir / '.conversion_complete'
    marker_payload = {
        'output_files': [str(path) for path in output_files],
        'tensor_names': tensor_names,
    }
    marker_path.write_text(json.dumps(marker_payload, sort_keys=True, indent=2), encoding='utf-8')


def discover_output_safetensors(output_path):
    if not output_path.exists():
        return []
    return sorted(
        path for path in output_path.iterdir()
        if path.is_file() and path.suffix == '.safetensors'
    )


def write_safetensors_index(output_path, output_files=None, logger=None):
    if output_files is None:
        output_files = discover_output_safetensors(output_path)
    else:
        output_files = sorted(
            path for path in output_files
            if path.is_file() and path.suffix == '.safetensors'
        )
    if not output_files:
        raise FileNotFoundError('No safetensors files found in output directory')
    weight_map = {}
    total_size = 0
    for path in output_files:
        header = read_safetensors_header(path)
        for tensor_name in header:
            if tensor_name == METADATA_KEY:
                continue
            weight_map[tensor_name] = path.name
        total_size += path.stat().st_size
    payload = {
        'metadata': {
            'total_size': total_size,
        },
        'weight_map': weight_map,
    }
    index_path = output_path / 'model.safetensors.index.json'
    index_path.write_text(json.dumps(payload, sort_keys=True, indent=2), encoding='utf-8')
    if logger is not None:
        logger.info('wrote %s', index_path)
    return index_path


def cleanup_state_artifacts(state_dir):
    if not state_dir.exists():
        return
    # Keep the state directory around so the completion marker remains available for
    # subsequent resume checks after the temporary payload artifacts are removed.
    payload_dir = state_dir / 'payloads'
    if payload_dir.exists():
        for child in sorted(payload_dir.iterdir()):
            if child.is_file():
                child.unlink()
            elif child.is_dir():
                shutil.rmtree(child)
        payload_dir.rmdir()
    for state_path in sorted(state_dir.glob('*.json')):
        state_path.unlink(missing_ok=True)


def write_final_output(out_file, tasks, completed, state_dir, output_path):
    header = {}
    data_offset = 0
    task_payloads = []
    expected_tensor_names = []
    for task in tasks:
        task_entry = completed.get(task['task_id'])
        if not task_entry:
            continue
        expected = expected_output_names(task['tensor_name'], task['meta']['shape'])
        expected_tensor_names.extend(expected)
        task_tensors = load_completed_task_tensors(task, task_entry, state_dir, output_path)
        payload_entries = []
        for name, payload_source, dtype, shape in task_tensors:
            if isinstance(payload_source, bytes):
                payload_size = len(payload_source)
                validate_payload_length(dtype, payload_size, shape, quant_kind=should_quantize(name), tensor_name=name)
                payload_entries.append((name, payload_source, dtype, shape, payload_size))
                header[name] = {
                    'dtype': dtype,
                    'shape': list(shape),
                    'data_offsets': [data_offset, data_offset + payload_size],
                }
                data_offset += payload_size
            else:
                payload_size = payload_source.stat().st_size
                validate_payload_length(dtype, payload_size, shape, quant_kind=should_quantize(name), tensor_name=name)
                payload_entries.append((name, payload_source, dtype, shape, payload_size))
                header[name] = {
                    'dtype': dtype,
                    'shape': list(shape),
                    'data_offsets': [data_offset, data_offset + payload_size],
                }
                data_offset += payload_size
        task_payloads.append(payload_entries)
    if not header:
        return []
    if len(header) != len(expected_tensor_names):
        raise ValueError('expected %d tensor entry(s) in output but found %d' % (len(expected_tensor_names), len(header)))
    header_bytes = json.dumps(header, separators=(',', ':')).encode('utf-8')
    tmp_path = out_file.with_suffix(out_file.suffix + '.tmp')
    with open(tmp_path, 'wb') as fh:
        fh.write(len(header_bytes).to_bytes(8, 'little'))
        fh.write(header_bytes)
        for payload_entries in task_payloads:
            for _, payload_source, _, _, _ in payload_entries:
                if isinstance(payload_source, bytes):
                    fh.write(payload_source)
                else:
                    with open(payload_source, 'rb') as payload_fh:
                        shutil.copyfileobj(payload_fh, fh, length=1024 * 1024)
    os.replace(tmp_path, out_file)
    return sorted(header.keys())


def _replace_existing_path(source_path, target_path):
    if target_path.exists():
        if target_path.is_dir() and not target_path.is_symlink():
            shutil.rmtree(target_path)
        else:
            target_path.unlink()
    shutil.move(str(source_path), str(target_path))


def resolve_state_dir(output_path, requested_state_dir=None):
    if requested_state_dir:
        return Path(requested_state_dir).expanduser().resolve()
    preferred_state_dir = output_path / '.state'
    default_state_dir = output_path / '.conversion_state'
    if preferred_state_dir.exists():
        return preferred_state_dir
    if default_state_dir.exists():
        preferred_state_dir.mkdir(parents=True, exist_ok=True)
        for child in sorted(default_state_dir.iterdir()):
            _replace_existing_path(child, preferred_state_dir / child.name)
        default_state_dir.rmdir()
        return preferred_state_dir
    default_state_dir.mkdir(parents=True, exist_ok=True)
    return default_state_dir


def handle_completed_task(completed, state_dir, logger, task_result, output_tensors, total_tasks):
    state_path = persist_task_result(state_dir, task_result, output_tensors)
    completed[task_result['task_id']] = {'kind': 'state', 'state_path': state_path}
    for entry in output_tensors:
        logger.info('wrote %s', entry[0])
    render_progress(len(completed), total_tasks, 'converting %s' % task_result['source_name'])


def main():
    parser = argparse.ArgumentParser(description='Convert standard safetensors to the qwen35_moe quantized layout')
    parser.add_argument('--input', required=True)
    parser.add_argument('--output', required=True)
    parser.add_argument('--log-file', default=None, help='optional path for the conversion log file')
    parser.add_argument('--workers', type=int, default=None, help='number of worker processes; defaults to all CPU cores')
    parser.add_argument('--state-dir', default=None, help='directory for per-tensor resume checkpoints')
    parser.add_argument('--generate-index-only', action='store_true', help='generate model.safetensors.index.json from existing output shards')
    args = parser.parse_args()

    input_path = Path(args.input).resolve()
    output_path = Path(args.output).resolve()
    output_path.mkdir(parents=True, exist_ok=True)
    logger, log_path = setup_logger(output_path, args.log_file)
    logger.info('converter starting: input=%s output=%s', input_path, output_path)
    logger.info('log file: %s', log_path)
    try:
        if args.generate_index_only:
            output_files = discover_output_safetensors(output_path)
            logger.info('generating index from %d safetensors file(s)', len(output_files))
            write_safetensors_index(output_path, output_files, logger=logger)
            return 0

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

        try:
            files, source_groups = build_source_task_groups(input_path)
        except FileNotFoundError:
            existing_outputs = discover_output_safetensors(output_path)
            if existing_outputs:
                for path in existing_outputs:
                    try:
                        header = read_safetensors_header(path)
                    except (FileNotFoundError, ValueError, json.JSONDecodeError):
                        continue
                    if header:
                        logger.info('input files are missing but output already exists; generating index from converted shards')
                        write_safetensors_index(output_path, existing_outputs, logger=logger)
                        return 0
            raise
        tasks = [task for group in source_groups for task in group['tasks']]
        logger.info('discovered %d safetensors file(s) and %d tensor task(s)', len(files), len(tasks))

        state_dir = resolve_state_dir(output_path, args.state_dir)
        state_dir.mkdir(parents=True, exist_ok=True)
        logger.info('using state dir: %s', state_dir)
        completed = load_resumed_outputs(state_dir, output_path, tasks)
        logger.info('resumed %d existing tensor task(s)', len(completed))

        workers = args.workers if args.workers is not None else (os.cpu_count() or 1)
        workers = max(1, workers)
        logger.info('using %d worker(s) for streaming conversion', workers)
        max_pending = max(1, workers * 2)
        output_files = []
        tensor_names = []
        for source_group in source_groups:
            source_tasks = source_group['tasks']
            if not source_tasks:
                continue
            pending_tasks = [task for task in source_tasks if task['task_id'] not in completed]
            if pending_tasks:
                with concurrent.futures.ProcessPoolExecutor(max_workers=workers) as executor:
                    futures = {}
                    for task in pending_tasks:
                        # Worker processes do not inherit the parent's logger configuration,
                        # so each worker stays fully self-contained and no shared handler state
                        # is required across the process pool. The converter therefore sends
                        # the already-decoded tensor payloads back to the parent process.
                        futures[executor.submit(convert_task, task, logger=None)] = task
                        if len(futures) >= max_pending:
                            done, _ = concurrent.futures.wait(futures, return_when=concurrent.futures.FIRST_COMPLETED)
                            completed_future = next(iter(done))
                            task_result = futures.pop(completed_future)
                            output_tensors = completed_future.result()
                            handle_completed_task(completed, state_dir, logger, task_result, output_tensors, len(tasks))
                    while futures:
                        done, _ = concurrent.futures.wait(futures, return_when=concurrent.futures.FIRST_COMPLETED)
                        completed_future = next(iter(done))
                        task_result = futures.pop(completed_future)
                        output_tensors = completed_future.result()
                        handle_completed_task(completed, state_dir, logger, task_result, output_tensors, len(tasks))
            source_out_file = output_file_for_source(output_path, source_group['source_name'])
            written_names = write_final_output(source_out_file, source_tasks, completed, state_dir, output_path)
            tensor_names.extend(written_names)
            output_files.append(source_out_file)
            for task in source_tasks:
                cleanup_task_state_artifacts(state_dir, task['task_id'])
            source_path = source_group['source_path']
            if source_path.exists():
                source_path.unlink()
                logger.info('removed source file %s', source_path)
            logger.info('wrote %s', source_out_file)
        if not completed:
            raise RuntimeError('no tensors were converted')
        if len(completed) != len(tasks):
            missing = [task['task_id'] for task in tasks if task['task_id'] not in completed]
            raise RuntimeError('failed to complete %d task(s): %s' % (len(missing), ', '.join(missing)))

        write_completion_marker(state_dir, output_files, tensor_names)
        write_safetensors_index(output_path, output_files, logger=logger)
        cleanup_state_artifacts(state_dir)
        return 0
    except Exception:
        logger.exception('conversion failed')
        return 1


if __name__ == '__main__':
    sys.exit(main())
