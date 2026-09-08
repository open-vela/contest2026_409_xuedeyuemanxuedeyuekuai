#!/bin/bash
set -euo pipefail

BUILD=/home/cmake_out/lckfb_huangshan_pi
CONFIG=$BUILD/.config
HEADER=/home/nuttx-apps/graphics/lvgl/lvgl/src/lv_conf_internal.h

sed -i 's/^CONFIG_LV_USE_FREETYPE=y$/# CONFIG_LV_USE_FREETYPE is not set/' "$CONFIG"

cmake -B "$BUILD" \
  -S /home/nuttx \
  -GNinja \
  -DBOARD_CONFIG=/home/vendor/sifli/boards/sf32lb52/lckfb_huangshan_pi/configs/nsh \
  -DEXTRA_FLAGS='-Wno-cpp -Wno-deprecated-declarations'

if [ ! -f "$HEADER.codex-backup" ]; then
    cp "$HEADER" "$HEADER.codex-backup"
fi

sed -i \
  's/#define LV_ATTRIBUTE_LARGE_CONST CONFIG_LV_ATTRIBUTE_LARGE_CONST/#define LV_ATTRIBUTE_LARGE_CONST/' \
  "$HEADER"

echo "FreeType setting:"
grep '^# CONFIG_LV_USE_FREETYPE' "$CONFIG"
echo "Large constant setting:"
grep -n 'LV_ATTRIBUTE_LARGE_CONST' "$HEADER" | head -8
cmake --build "$BUILD" -j2
