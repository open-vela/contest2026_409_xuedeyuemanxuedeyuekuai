#!/bin/bash
set -euo pipefail

APPS=/home/nuttx-apps
BUILD=/home/cmake_out/lckfb_huangshan_pi
CONFIG=$BUILD/.config
HEADER=$APPS/graphics/lvgl/lvgl/src/lv_conf_internal.h

echo "===== diagnose ====="
ls "$APPS/external" 2>/dev/null || echo "(no external dir)"
ls "$APPS/external/libpng" 2>/dev/null || echo "(external/libpng empty or missing)"

echo "===== try submodules ====="
timeout 180 git -C "$APPS" submodule update --init --depth 1 || \
  echo "(submodule fetch failed or skipped; will use fallback)"

HAVE_PNG=0
[ -f "$APPS/external/libpng/png.h" ] && HAVE_PNG=1
[ -f "$APPS/external/libpng/libpng/png.h" ] && HAVE_PNG=1
echo "libpng available: $HAVE_PNG"

if [ "$HAVE_PNG" != "1" ]; then
  echo "===== disabling optional decoders that lack sources ====="
  sed -i 's/^CONFIG_LV_USE_LIBPNG=y$/# CONFIG_LV_USE_LIBPNG is not set/' "$CONFIG"
  sed -i 's/^CONFIG_LV_USE_LODEPNG=y$/# CONFIG_LV_USE_LODEPNG is not set/' "$CONFIG"
  grep -E '^#? ?CONFIG_LV_USE_(LIBPNG|LODEPNG)' "$CONFIG" || true
fi

echo "===== reconfigure ====="
cmake -B "$BUILD" -S /home/nuttx -GNinja \
  -DBOARD_CONFIG=/home/vendor/sifli/boards/sf32lb52/lckfb_huangshan_pi/configs/nsh \
  -DEXTRA_FLAGS='-Wno-cpp -Wno-deprecated-declarations'

echo "===== patch LV_ATTRIBUTE_LARGE_CONST ====="
if [ ! -f "$HEADER.codex-backup" ]; then
  cp "$HEADER" "$HEADER.codex-backup"
fi
sed -i 's/#define LV_ATTRIBUTE_LARGE_CONST CONFIG_LV_ATTRIBUTE_LARGE_CONST/#define LV_ATTRIBUTE_LARGE_CONST/' "$HEADER"
grep -n 'define LV_ATTRIBUTE_LARGE_CONST' "$HEADER"

echo "===== build ====="
cmake --build "$BUILD" -j2
