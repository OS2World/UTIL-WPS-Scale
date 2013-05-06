REM This program does compilation under IBM C Set/2 and calls link386
REM afterwards. Invoke it with "ibmc scale"
icc /C+ /O+ /Sp4 %1.c
linkup %1
