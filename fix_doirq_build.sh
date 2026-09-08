#!/bin/bash
set -euo pipefail

F=/home/nuttx/arch/arm/src/armv8-m/arm_doirq.c
BUILD=/home/cmake_out/lckfb_huangshan_pi

if [ ! -f "$F.codex-backup" ]; then
  cp "$F" "$F.codex-backup"
fi

python3 - <<'EOF'
import io
p = "/home/nuttx/arch/arm/src/armv8-m/arm_doirq.c"
s = open(p, encoding="utf-8").read()

old = '''#ifdef CONFIG_ARCH_FPU
  __asm__ __volatile__
    (
      "mov r0, %0\\n"
      "vmsr fpscr, r0\\n"
      :
      : "i" (ARM_FPSCR_LTPSIZE_NONE)
    );
#endif'''

new = '''#ifdef CONFIG_ARCH_FPU
  {
    /* Fix: original inline asm clobbered r0 without telling the compiler,
     * destroying the IPSR-derived irq number on thumb2 builds. */
    uint32_t tmp = (uint32_t)ARM_FPSCR_LTPSIZE_NONE;
    __asm__ __volatile__
      (
        "vmsr fpscr, %0\\n"
        : "+l" (tmp)
        :
        : "memory"
      );
  }
#endif'''

if old in s:
    s = s.replace(old, new, 1)
elif '"mov r0' not in s:
    print("already patched")
else:
    raise SystemExit("pattern mismatch -- inspect manually")

open(p, "w", encoding="utf-8").write(s)
print("patched ok")
EOF

cmake --build "$BUILD" -j2

echo "===== outputs ====="
ls -la "$BUILD"/nuttx.bin
cp "$BUILD/nuttx.bin" '/mnt/c/Users/21561/Desktop/比赛/nuttx_huangshan.bin'
echo "copied to Windows"
