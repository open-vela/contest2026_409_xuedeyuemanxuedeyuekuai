set -e
grep -RIn 'nuttx_generate_kconfig\|_home.*Kconfig\|generate.*kconfig\|Examples/Kconfig' /home/nuttx/tools /home/nuttx/cmake /home/nuttx/CMakeLists.txt 2>/dev/null | head -100