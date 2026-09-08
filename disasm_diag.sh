#!/bin/bash
set -euo pipefail

cd /home/cmake_out/lckfb_huangshan_pi
OBJDUMP=arm-none-eabi-objdump
command -v $OBJDUMP >/dev/null || OBJDUMP=/usr/bin/arm-none-eabi-objdump
$OBJDUMP -d nuttx > /tmp/nuttx.asm

echo "===== exception_direct ====="
awk '/<exception_direct>:/{f=1} f{print; c++} f&&c>44{exit}' /tmp/nuttx.asm

echo "===== irq_dispatch ====="
awk '/<irq_dispatch>:/{f=1} f{print; c++} f&&c>60{exit}' /tmp/nuttx.asm

echo "===== callers of exception_direct / irq_dispatch ====="
grep -n 'bl.*exception_direct\|bl.*irq_dispatch' /tmp/nuttx.asm | head -20
