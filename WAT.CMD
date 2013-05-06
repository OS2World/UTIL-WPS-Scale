REM This file does compiling and linking using Watcom C 386 9.0 or later
REM It creates an 8K stack (necessary!) and uses the "benchmarking" flags
REM (optional). Invoke it with "wat scale"
wcl386 /l=os2v2_pm /k8192 /omaxet /zp4 /4r %1
