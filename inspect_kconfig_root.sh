echo '== root Kconfig include apps =='
grep -n -A20 -B10 'APPSBINDIR\|nuttx-apps' /home/nuttx/Kconfig | head -100
echo '== apps Kconfig head =='
sed -n '1,100p' /home/nuttx-apps/Kconfig
echo '== build generated wrapper? =='
find /home/cmake_out/lckfb_huangshan_pi -maxdepth 2 -name Kconfig -type f -print -exec sed -n '1,30p' {} \;