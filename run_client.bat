@echo off
setlocal

cd /d "%~dp0"

set "QT_BIN=C:\Qt\6.11.0\mingw_64\bin"
set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"
set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"

set "SERVER_HOST=%~1"
set "SERVER_PORT=%~2"

if "%SERVER_HOST%"=="" set /p SERVER_HOST=Enter server IP or host: 
if "%SERVER_PORT%"=="" set "SERVER_PORT=40000"

echo Building HappyMealsClient...
"%QT_BIN%\qmake.exe" HappyMealsClient.pro
if errorlevel 1 goto error

"%MINGW_BIN%\mingw32-make.exe"
if errorlevel 1 goto error

echo Deploying Qt runtime...
"%QT_BIN%\windeployqt.exe" release\HappyMealsClient.exe
if errorlevel 1 goto error

echo.
echo Starting HappyMealsClient. Server: %SERVER_HOST%:%SERVER_PORT%
echo.
release\HappyMealsClient.exe %SERVER_HOST% %SERVER_PORT%
goto end

:error
echo.
echo Build, deploy, or start failed. Check the messages above.
pause

:end
endlocal
