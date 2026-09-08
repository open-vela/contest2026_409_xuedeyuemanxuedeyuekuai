#!/bin/bash
set -euo pipefail

F=/home/nuttx-apps/graphics/lvgl/lvgl/src/drivers/nuttx/lv_nuttx_image_cache.c
BUILD=/home/cmake_out/lckfb_huangshan_pi

if [ ! -f "$F.codex-backup" ]; then
  cp "$F" "$F.codex-backup"
fi

# NuttX dropped gettid(); getpid() is the drop-in for this single-threaded UI case.
sed -i 's/(uint32_t)gettid()/(uint32_t)getpid()/' "$F"
# ensure declaration is visible
grep -q 'unistd.h' "$F" || \
  sed -i 's|#include <nuttx/mm/mm.h>|#include <nuttx/mm/mm.h>\n#include <unistd.h>|' "$F"

echo "patched lines:"
grep -n 'getpid\|unistd' "$F"

cmake --build "$BUILD" -j2

echo "===== outputs ====="
ls -la "$BUILD"/nuttx* 2>/dev/null || true
