set -e
echo '== lvgldemo Kconfig =='
sed -n '1,100p' /home/nuttx-apps/examples/lvgldemo/Kconfig
echo '== graphics generated Kconfig symbols =='
grep -RIn 'config .*LVGL\|config LVGL' /home/nuttx-apps/graphics /home/nuttx/boards 2>/dev/null | head -80
echo '== config LVGL block =='
grep -n -A40 -B10 'CONFIG_LVGL_VERSION_MAJOR' /home/cmake_out/lckfb_huangshan_pi/.config | head -100
echo '== all LVGL bool/tristate symbols =='
grep -nE '^(CONFIG_.*LVGL.*=|CONFIG_LVGL=|# CONFIG_.*LVGL)' /home/cmake_out/lckfb_huangshan_pi/.config | head -100