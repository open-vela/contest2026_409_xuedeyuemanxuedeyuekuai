grep -RIn 'APPSBINDIR\|set(ENV{APPSDIR}\|set(ENV{APPSBINDIR}' /home/nuttx/cmake /home/nuttx/CMakeLists.txt /home/nuttx/tools 2>/dev/null | head -100 || true
echo '== process env =='
env | grep -E 'APPS|BINDIR|EXTERNALDIR|NUTTX' || true