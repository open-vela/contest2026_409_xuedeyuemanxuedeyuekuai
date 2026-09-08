set -e
cd /home/nuttx
BUILD=/home/cmake_out/lckfb_huangshan_pi
export KCONFIG_CONFIG=$BUILD/.config
export EXTERNALDIR=dummy
export APPSDIR=/home/nuttx-apps
export DRIVERS_PLATFORM_DIR=dummy
export APPSBINDIR=$BUILD/nuttx-apps
export BINDIR=$BUILD
olddefconfig 2>&1 | tee /tmp/kconfig.log
grep -n 'WATCHFACE' "$KCONFIG_CONFIG" /tmp/kconfig.log || true