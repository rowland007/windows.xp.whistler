@echo off
setlocal
set _nokrnl=0
if "%1" == "user" set _nokrnl=1
if "%1" == "user" cd user
if "%1" == "user" shift
set _bldflags=-z
if NOT "%1" == "" set _bldflags=%1 %2 %3 %4 %5
build -z -M 4
if ERRORLEVEL 1 goto done
cd \xp\neptune\xpos\dll
build -z
if ERRORLEVEL 1 goto done
if "%_nokrnl%" == "1" goto done
cd \xp\neptune\xpos\boot
build -z -M 4
if ERRORLEVEL 1 goto done
cd \xp\neptune\xpos\init
build -z -M 2
cd ..\rtl
:done
endlocal
