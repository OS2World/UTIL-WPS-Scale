# nmake makefile
#
# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::GNU C
#  Make: GNU make
all : scale.exe

scale.exe : scale.obj scale.def
	gcc -Zomf scale.obj scale.def -o scale.exe

scale.obj : scale.c 
	gcc -Wall -Zomf -c -O2 scale.c -o scale.obj

clean :
	rm -rf *exe *res *obj
