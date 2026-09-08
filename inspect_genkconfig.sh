set -e
F=/home/cmake_out/lckfb_huangshan_pi/nuttx-apps/_home_nuttx-apps_examples_Kconfig
echo '== watchface lines =='
grep -n 'watchface\|WATCHFACE' "$F" || true
echo '== around l =='
sed -n '72,92p' "$F"
echo '== Windows assets state =='
find /mnt/c/Users/21561/Desktop/比赛/watchface_app -maxdepth 3 -type f -printf '%p %s bytes\n' | sort