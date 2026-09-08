set -e
BUILD=/home/cmake_out/lckfb_huangshan_pi
ls "$BUILD/bin_host" | grep -E 'conf|kconfig' || true
command -v olddefconfig || true
find "$BUILD/bin_host" -maxdepth 2 -type f -executable -printf '%p\n' | head -30
echo '== generated source context =='
sed -n '180,195p' "$BUILD/nuttx-apps/_home_nuttx-apps_examples_Kconfig"
echo '== parser help =='
olddefconfig --help 2>&1 | head -20 || true