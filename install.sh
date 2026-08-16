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
for cmd in cmake gcc g++; do
    command -v "$cmd" >/dev/null 2>&1 || missing="$missing $cmd"
done
for pkg in Qt6Widgets Qt6Network Qt6Svg; do
    pkg-config --exists "$pkg" 2>/dev/null || missing="$missing $pkg"
done

if [ -n "$missing" ]; then
    echo "Missing dependencies:$missing"
    echo ""
    echo "Arch:  sudo pacman -S cmake gcc qt6-base qt6-svg"
    echo "Debian: sudo apt install cmake g++ qt6-base-dev libqt6svg6-dev"
    exit 1
fi
echo "  All dependencies found."

echo ""
echo "[2/3] Building llama-ui-v4 (Qt)..."
cmake -S v4 -B v4/build -DCMAKE_BUILD_TYPE=Release >/dev/null
cmake --build v4/build -j"$(nproc)" >/dev/null

echo "  Binary: $(ls -lh v4/build/llama-ui-v4 | awk '{print $5}')"

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
