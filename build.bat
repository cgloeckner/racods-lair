@echo off

set PATH=%PATH%;C:\GCC_x86\bin

cls
set DIR=%CD%
C:
cd .\%HOMEPATH%\.build\racod_x86
mingw32-make -j 8
exit %ERRORLEVEL%

