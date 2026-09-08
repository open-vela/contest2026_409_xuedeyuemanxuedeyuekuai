#!/bin/bash
LV=/home/nuttx-apps/graphics/lvgl/lvgl
echo "=== lvgl version ==="
grep -rn 'LVGL_VERSION_MAJOR\|LVGL_VERSION_MINOR' "$LV/lv_version.h" "$LV/src/lv_version.h" 2>/dev/null | head -6

echo "=== lcd resolution in driver ==="
grep -n 'WIDTH\|HEIGHT\|454\|466' /home/vendor/sifli/boards/sf32lb52/drivers/lcd/co5300.c | head -10

echo "=== defconfig lcd keys ==="
grep -iE 'CO5300|LCD_|LCDC' /home/cmake_out/lckfb_huangshan_pi/.config | grep -v '^#' | head -20

echo "=== PIL availability ==="
python3 - <<'EOF'
try:
    import PIL
    print("PIL ok", PIL.__version__)
except Exception as e:
    print("PIL missing:", e)
EOF
