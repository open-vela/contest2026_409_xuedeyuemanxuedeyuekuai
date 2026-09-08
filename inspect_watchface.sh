set -e
BUILD=/home/cmake_out/lckfb_huangshan_pi
APP=/home/nuttx-apps/examples/watchface
echo '== build Kconfig registration =='
grep -RIn --exclude='*.o' --exclude='*.a' 'watchface\|WATCHFACE' "$BUILD/nuttx-apps" 2>/dev/null | head -80 || true
echo '== apps tree registration =='
grep -RIn 'watchface\|WATCHFACE' /home/nuttx-apps/examples/Kconfig /home/nuttx-apps/examples/Makefile "$APP" 2>/dev/null | head -100 || true
echo '== generated asset state =='
find "$APP" -maxdepth 3 -type f -printf '%p %s bytes\n' | sort
find /tmp/assets -maxdepth 2 -type f -printf '%p %s bytes\n' 2>/dev/null | sort || true
