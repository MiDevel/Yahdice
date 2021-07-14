@echo off
@cls
@if exist *.bin del *.bin
@if exist *.grc del *.grc

if "%1"=="NORES" GOTO DOCOMPILE
echo compile the resource files
pilrc Yahdice.rcp

:DOCOMPILE
echo Step 1 -----------------------------------------------------
call m68k-palmos-gcc -O1 -c Yahdice.c -o Yahdice.o
call m68k-palmos-gcc -O1 -c ..\gc2_common\PalmUtils.c -o PalmUtils.o
call m68k-palmos-gcc -O1 -c YahdiceDB.c -o YahdiceDB.o

echo Step 2 -----------------------------------------------------
call m68k-palmos-gcc -O1 Yahdice.o PalmUtils.o YahdiceDB.o -o Yahdice

echo Step 3 -----------------------------------------------------
call m68k-palmos-obj-res Yahdice

echo Step 4 -----------------------------------------------------
call build-prc  Yahdice.prc "Yahdice" m_YA *.grc *.bin

echo Step 5 -----------------------------------------------------
@del *.bin > nul
@del *.o > nul
@del *.grc > nul
@del Yahdice > nul

echo ------------------------------------------------------------
echo Compile done -----------------------------------------------
echo ------------------------------------------------------------

@if exist *.bin del *.bin
@if exist *.grc del *.grc

rem exit

