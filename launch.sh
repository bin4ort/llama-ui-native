#!/usr/bin/env bash
# Copyright (C) 2025 Llama UI Native contributors
# Licensed under the GNU General Public License v3 or later — see LICENSE.
DIR="$(cd "$(dirname "$0")" && pwd)"
export LD_LIBRARY_PATH="$DIR${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$DIR/llama-ui-native"
