set -e
cd /home/cmake_out/lckfb_huangshan_pi
cp .config /tmp/config.before
olddefconfig Kconfig 2>&1 | tee /tmp/kconfig.log || true
grep -n 'WATCHFACE' .config /tmp/kconfig.log || true