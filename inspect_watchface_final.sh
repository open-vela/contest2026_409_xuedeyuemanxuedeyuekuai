set -e
BUILD=/home/cmake_out/lckfb_huangshan_pi
echo '== generated examples Kconfig =='
grep -n -A3 -B3 'watchface\|WATCHFACE' "$BUILD/nuttx-apps/_home_nuttx-apps_examples_Kconfig" || true
echo '== final config symbols =='
grep -n 'WATCHFACE\|CONFIG_LVGL' "$BUILD/.config" | head -80
echo '== builtins =='
grep -RIn 'watchface\|WATCHFACE' "$BUILD/apps" 2>/dev/null | head -80 || true
echo '== app files =='
find /home/nuttx-apps/examples/watchface -maxdepth 3 -type f -printf '%p %s bytes\n' | sort
echo '== Kconfig full =='
cat /home/nuttx-apps/examples/watchface/Kconfig
echo '== CMake =='
cat /home/nuttx-apps/examples/watchface/CMakeLists.txt