@echo off
tasm /m2 lab7.asm
tlink /3 lab7.obj
del *.obj
del *.map
