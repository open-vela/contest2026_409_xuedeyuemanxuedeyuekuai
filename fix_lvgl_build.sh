#!/bin/bash
set -euo pipefail

HEADER=/home/nuttx-apps/graphics/lvgl/lvgl/src/lv_conf_internal.h
BUILD=/home/cmake_out/lckfb_huangshan_pi

if [ ! -f "$HEADER" ]; then
    echo "Missing file: $HEADER"
    exit 1
fi

if [ ! -f "$HEADER.codex-backup" ]; then
    cp "$HEADER" "$HEADER.codex-backup"
fi

sed -i \
  's/#define LV_ATTRIBUTE_LARGE_CONST CONFIG_LV_ATTRIBUTE_LARGE_CONST/#define LV_ATTRIBUTE_LARGE_CONST/' \
  "$HEADER"

echo "Patched definition:"
grep -n "LV_ATTRIBUTE_LARGE_CONST" "$HEADER"
cmake --build "$BUILD" -j2
