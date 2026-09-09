#!/bin/bash
set -euo pipefail

APP_SRC="/mnt/c/Users/21561/Desktop/比赛/app/watchface_app"
APP_DST=/home/nuttx-apps/examples/watchface
VENDOR_BOARD=/home/vendor/sifli/boards/sf32lb52/lckfb_huangshan_pi
DEFCONFIG=$VENDOR_BOARD/configs/nsh/defconfig
BUILD=/home/cmake_out/lckfb_huangshan_pi
RCS=$VENDOR_BOARD/src/etc/init.d/rcS

# 1. pick up generated wf_data.c produced by assets_gen.py (lands in /tmp)
if [ -f /tmp/assets/wf_data.c ]; then
  mkdir -p "$APP_SRC/assets"
  cp /tmp/assets/wf_data.c "$APP_SRC/assets/wf_data.c"
fi

# 2. sync app tree into nuttx-apps
rm -rf "$APP_DST"
mkdir -p "$(dirname $APP_DST)"
cp -r "$APP_SRC" "$APP_DST"

# The emergency reset policy lives in the board lower-half, which is outside
# the application tree. Keep the source used by the build in sync with the
# checked-in watchface policy before CMake configures the board.
cp "$APP_SRC/src/sf32lb52_buttons.c" \
   "$VENDOR_BOARD/src/sf32lb52_buttons.c"

# 2a. Force Kconfig rescanning. nuttx_generate_kconfig() returns immediately
# when the previously generated apps bindir still exists.
rm -rf "$BUILD/nuttx-apps"

# 3. boot script: launch the watch UI instead of the missing vapp demo
if grep -q '^vapp hap://' "$RCS"; then
  sed -i 's|^vapp hap://.*|watchface \&|' "$RCS"
  echo "rcS patched:"
  cat "$RCS"
elif ! grep -q '^watchface &' "$RCS"; then
  echo 'watchface &' >> "$RCS"
fi

# 4. enable the app in the board defconfig
grep -q '^CONFIG_EXAMPLES_WATCHFACE=y$' "$DEFCONFIG" || \
  printf '\nCONFIG_EXAMPLES_WATCHFACE=y\n' >> "$DEFCONFIG"
grep 'WATCHFACE' "$DEFCONFIG"

# 5. reconfigure (picks up the new Kconfig) and build
cmake -B "$BUILD" -S /home/nuttx -GNinja \
  -DBOARD_CONFIG="$VENDOR_BOARD/configs/nsh" \
  -DEXTRA_FLAGS='-Wno-cpp -Wno-deprecated-declarations'

# Reapply the product trim after CMake regenerates .config.  This is the
# effective config file; defconfig alone is not enough on incremental builds.
bash /mnt/c/Users/21561/Desktop/比赛/apply_optimized_config.sh

# NuttX's plain olddefconfig can discard this app on the first pass even when
# its dependency is enabled. setconfig validates and persists the choice.
export KCONFIG_CONFIG="$BUILD/.config"
export EXTERNALDIR=dummy
export APPSDIR=/home/nuttx-apps
export DRIVERS_PLATFORM_DIR=dummy
export APPSBINDIR="$BUILD/nuttx-apps"
export BINDIR="$BUILD"
cd /home/nuttx
setconfig EXAMPLES_WATCHFACE=y --kconfig /home/nuttx/Kconfig

# setconfig only updates .config; regenerate config.h before Ninja runs,
# otherwise LV_CACHE_DEF_SIZE and other runtime values can stay stale.
cmake -B "$BUILD" -S /home/nuttx -GNinja \
  -DBOARD_CONFIG="$VENDOR_BOARD/configs/nsh" \
  -DEXTRA_FLAGS='-Wno-cpp -Wno-deprecated-declarations'

cmake --build "$BUILD" -j2

ls -la "$BUILD"/nuttx.bin
cp "$BUILD/nuttx.bin" '/mnt/c/Users/21561/Desktop/比赛/nuttx_huangshan.bin'
echo "===== build done, firmware copied to Windows ====="
