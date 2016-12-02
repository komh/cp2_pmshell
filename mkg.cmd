@echo off
rem build script for gcc/kLIBC
gcc -o crt0.o -c crt0.s && ^
emxomf -o crt0.obj -m__text crt0.o && ^
gcc -Zomf -nostdlib -Zlinker /pmtype:pm cp2_pmshell.c crt0.obj -los2
