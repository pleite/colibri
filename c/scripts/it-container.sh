#!/bin/bash

podman run --rm --name colibri-all --privileged --publish 8080:8000 --env COLI_ACCEL=all --group-add keep-groups --mount type=bind,src=/dev/accel,dst=/dev/accel --mount type=bind,src=/dev/dri,dst=/dev/dri --mount type=bind,src=/dev/kfd,dst=/dev/kfd --mount type=bind,src=/opt/models/ornith-int8/,dst=/models,ro --env COLI_MODEL=/models --env COLI_RAM=48 --env COLI_API_KEY="secret" ghcr.io/pleite/colibri-all:latest  \
/opt/colibri/c/qwen35_moe --model /models --threads 32 
