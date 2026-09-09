############################################################################
# apps/examples/watchface/CMakeLists.txt
#
# SPDX-License-Identifier: Apache-2.0
############################################################################

if(CONFIG_EXAMPLES_WATCHFACE)
  nuttx_add_application(
    NAME watchface
    PRIORITY ${CONFIG_EXAMPLES_WATCHFACE_PRIORITY}
    STACKSIZE ${CONFIG_EXAMPLES_WATCHFACE_STACKSIZE}
    MODULE ${CONFIG_EXAMPLES_WATCHFACE}
    DEPENDS lvgl
    SRCS
      src/watchface_main.c
      src/honeycomb_model.c
      assets/wf_data.c
      src/font_zh_16.c
      src/font_zh_20.c
      src/buttons.c
      src/sensor_imu.c
      src/watch_power.c
      src/watch_haptics.c
      src/watch_board.c)
endif()
