#!/usr/bin/env bash
# run-headless.sh — run the v4 app on a virtual display (Xvfb) so no window
# ever appears on the user's screen. All screenshots are produced by the
# app's own --screenshot flag (internal grab, works offscreen).
#
# Usage: run-headless.sh <app args...>
#   e.g. run-headless.sh --theme dark --selftest "hi" --screenshot /tmp/opencode/v4.jpg

set -euo pipefail
cd "$(dirname "$0")/../.."

XVFB_DISPLAY=":97"
APP="${APP:-./v4/build/llama-ui-v4}"

if ! xdpyinfo -display "$XVFB_DISPLAY" >/dev/null 2>&1; then
    Xvfb "$XVFB_DISPLAY" -screen 0 1280x800x24 >/tmp/opencode/xvfb.log 2>&1 &
    for i in $(seq 1 20); do
        xdpyinfo -display "$XVFB_DISPLAY" >/dev/null 2>&1 && break
        sleep 0.5
    done
fi

DISPLAY="$XVFB_DISPLAY" timeout "${TIMEOUT:-90}" "$APP" "$@"
