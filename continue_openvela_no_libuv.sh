#!/bin/bash
set -euo pipefail

BUILD=/home/cmake_out/lckfb_huangshan_pi
CONFIG=$BUILD/.config
HEADER=/home/nuttx-apps/graphics/lvgl/lvgl/src/lv_conf_internal.h

sed -i 's/^CONFIG_LV_USE_NUTTX_LIBUV=y$/# CONFIG_LV_USE_NUTTX_LIBUV is not set/' "$CONFIG"

cmake -B "$BUILD" \
  -S /home/nuttx \
  -GNinja \
  -DBOARD_CONFIG=/home/vendor/sifli/boards/sf32lb52/lckfb_huangshan_pi/configs/nsh \
  -DEXTRA_FLAGS='-Wno-cpp -Wno-deprecated-declarations'

if ! grep -q '.codex-large-const-backup' <<< "$(head -5 "$HEADER")"; then
    if [ ! -f "$HEADER.codex-backup" ]; then
        cp "$HEADER" "$HEADER.codex-backup"
    fi
fi

sed -i \
  's/#define LV_ATTRIBUTE_LARGE_CONST CONFIG_LV_ATTRIBUTE_LARGE_CONST/#define LV_ATTRIBUTE_LARGE_CONST/' \
  "$HEADER"

echo "Current LVGL options:"
grep '^# CONFIG_LV_USE_FREETYPE' "$CONFIG"
grep '^# CONFIG_LV_USE_NUTTX_LIBUV' "$CONFIG"
cmake --build "$BUILD" -j2
