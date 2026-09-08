set -e
APP_SRC=/mnt/c/Users/21561/Desktop/比赛/watchface_app
APP_DST=/home/nuttx-apps/examples/watchface
cp "$APP_SRC/assets/wf_data.c" "$APP_DST/assets/wf_data.c"
cp "$APP_SRC/src/watchface_main.c" "$APP_DST/src/watchface_main.c"
cp "$APP_SRC/src/wf_assets.h" "$APP_DST/src/wf_assets.h"
cmake --build /home/cmake_out/lckfb_huangshan_pi -j2
ls -la /home/cmake_out/lckfb_huangshan_pi/nuttx.bin
cp /home/cmake_out/lckfb_huangshan_pi/nuttx.bin '/mnt/c/Users/21561/Desktop/比赛/nuttx_huangshan.bin'
