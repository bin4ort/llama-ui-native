#!/usr/bin/env bash
# Copyright (C) 2025 Llama UI Native contributors
# Licensed under the GNU General Public License v3 or later — see LICENSE.
# Llama UI Native — install script
set -euo pipefail

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_DIR"

NO_DESKTOP=false
[[ "${1:-}" == "--no-desktop" ]] && NO_DESKTOP=true

echo "=== Llama UI Native Install ==="
echo "Project dir: $PROJECT_DIR"

echo ""
echo "[1/3] Checking dependencies..."

missing=""
for cmd in gcc pkg-config curl; do
    command -v "$cmd" >/dev/null 2>&1 || missing="$missing $cmd"
done
for pkg in gtk+-3.0 webkit2gtk-4.1; do
    pkg-config --exists "$pkg" 2>/dev/null || missing="$missing $pkg"
done
test -f /usr/lib/libcurl.so || test -f /usr/lib/libcurl.so.4 || missing="$missing libcurl"

if [ -n "$missing" ]; then
    echo "Missing dependencies:$missing"
    echo ""
    echo "Arch:  sudo pacman -S gcc gtk3 webkit2gtk-4.1 curl pkgconf"
    echo "Debian: sudo apt install gcc libgtk-3-dev libwebkit2gtk-4.1-dev libcurl4-openssl-dev pkg-config"
    exit 1
fi
echo "  All dependencies found."

echo ""
echo "[2/3] Building llama-ui-native..."
gcc -o llama-ui-native main.c server.c \
    $(pkg-config --cflags --libs gtk+-3.0 webkit2gtk-4.1) \
    -L. -ljxl -ljxl_threads -lpthread -lm -lcurl -Wall \
    -Wl,-rpath,'$ORIGIN' \
    -Wl,-rpath-link,.

echo "  Binary: $(ls -lh llama-ui-native | awk '{print $5}')"

echo ""
echo "[3/3] Desktop entry..."
if $NO_DESKTOP; then
    echo "  Skipped (--no-desktop)."
else
    DESKTOP_DIR="${XDG_DATA_HOME:-$HOME/.local/share}/applications"
    mkdir -p "$DESKTOP_DIR"
    cp "Llama UI Native.desktop" "$DESKTOP_DIR/llama-ui-native.desktop"
    sed -i "s|Icon=icon-128.png|Icon=$PROJECT_DIR/static/icon-128.png|" "$DESKTOP_DIR/llama-ui-native.desktop"
    sed -i "s|Exec=.*|Exec=$PROJECT_DIR/launch.sh|" "$DESKTOP_DIR/llama-ui-native.desktop"
    echo "  Installed to $DESKTOP_DIR/llama-ui-native.desktop"
fi

echo ""
echo "=== Done ==="
echo "Launch: ./launch.sh  (or from your app menu: Llama UI Native)"
