# CPU-only Qwen/OpenAI container PoC

This note captures the proof-of-concept container path for exposing the colibrì OpenAI-compatible server from a CPU-only image and trying it with a Qwen model in CI or on a workstation.

## What this PoC validates

- The container starts the OpenAI-compatible gateway on port 8000.
- The gateway can be reached with `/health`, `/v1/models`, and `/v1/chat/completions`.
- The CPU image can be built and published from the existing GitHub Actions workflow without GPU-specific dependencies.
- A Qwen model directory can be mounted into `/models` and served through the same gateway path used by the rest of the repository.

## Current limitations and blockers

This is a smoke-test path, not a production-grade large-model runtime plan:

- The current Qwen implementation is still a correctness scaffold for the CPU path rather than a fully validated production engine.
- Large Qwen models remain limited by host RAM, CPU cores, and disk throughput. The container should be run with a memory cap and enough headroom to avoid swap churn.
- Expert streaming / cache behavior is still a manual validation topic; the container is useful for proving that the server boots and serves requests, not for claiming fully optimized expert loading.
- The container is only as good as the mounted model directory. A valid model tree needs at least `config.json`, tokenizer assets, and the expected weight files.

## Build and run

Build the image locally:

```bash
docker build -f Dockerfile.colibri -t colibri-cpu --build-arg BACKEND=cpu .
```

Run it with a model directory mounted into `/models`:

```bash
podman run --rm -d --name colibri-cpu \
  -p 8000:8000 \
  --memory=12g \
  --cpus=4 \
  --mount type=bind,src=/path/to/your-model,dst=/models,ro \
  --env COLI_MODEL=/models \
  colibri-cpu
```

The helper script can launch the same image with the CPU backend:

```bash
./c/scripts/run-container.sh --backend cpu --image ghcr.io/pleite/colibri-cpu:latest --model-dir /path/to/your-model
```

## CI smoke test checklist

1. Build or pull the `ghcr.io/pleite/colibri-cpu:latest` image.
2. Start the container with a small or converted Qwen model directory.
3. Wait for `/health` to respond.
4. Probe `/v1/models` and one non-streaming `/v1/chat/completions` request.
5. Capture the response and mark the job as a smoke test success or failure.
