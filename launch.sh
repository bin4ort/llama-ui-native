#!/usr/bin/env bash
# MIT License — Copyright (c) 2025 Llama UI Native
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$DIR/llama-ui-native"
