set -e
cd /home/nuttx
BUILD=/home/cmake_out/lckfb_huangshan_pi
export KCONFIG_CONFIG=$BUILD/.config
export EXTERNALDIR=dummy APPSDIR=/home/nuttx-apps DRIVERS_PLATFORM_DIR=dummy APPSBINDIR=$BUILD/nuttx-apps BINDIR=$BUILD
setconfig EXAMPLES_WATCHFACE=y --kconfig /home/nuttx/Kconfig 2>&1 | tee /tmp/setconfig.log
grep -n 'WATCHFACE' "$KCONFIG_CONFIG" /tmp/setconfig.log || true
setconfig EXAMPLES_WATCHFACE=y --kconfig /home/nuttx/Kconfig
grep -n 'WATCHFACE' "$KCONFIG_CONFIG" || true