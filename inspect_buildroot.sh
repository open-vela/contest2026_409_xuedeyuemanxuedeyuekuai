set -e
BUILD=/home/cmake_out/lckfb_huangshan_pi
find "$BUILD" -maxdepth 1 -type f -printf '%f %s bytes\n' | sort | head -100
find "$BUILD/nuttx-apps" -maxdepth 1 -type f -printf '%f %s bytes\n' | sort