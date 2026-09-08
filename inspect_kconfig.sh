set -e
echo '== root/Kconfig inclusion traces =='
grep -RIn 'examples/Kconfig\|examples$\|source.*examples' /home/nuttx-apps/Kconfig /home/nuttx-apps/*/Kconfig 2>/dev/null | head -100 || true
echo '== generated Kconfigs near build =='
find /home/cmake_out/lckfb_huangshan_pi -type f \( -iname '*Kconfig*' -o -iname '*CMakeCache.txt' \) -printf '%p %s\n' | head -80
echo '== lvgldemo registration =='
grep -RIn 'lvgldemo\|EXAMPLES_LVGLDEMO' /home/nuttx-apps/examples/Kconfig /home/cmake_out/lckfb_huangshan_pi 2>/dev/null | grep -E 'source|CONFIG_EXAMPLES_LVGLDEMO=y|Kconfig:' | head -80
