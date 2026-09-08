#!/bin/bash
grep -E 'CONFIG_SYSTEM_REBOOT|CONFIG_BOARD_RESET|CONFIG_SYSTEM_POWEROFF' /home/cmake_out/lckfb_huangshan_pi/.config || true
grep -n "watchface" /home/vendor/sifli/boards/sf32lb52/lckfb_huangshan_pi/src/etc/init.d/rcS || true
