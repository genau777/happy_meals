@echo off
setlocal

cd /d "%~dp0"

set "QT_BIN=C:\Qt\6.11.1\mingw_64\bin"
set "MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin"
set "PATH=%QT_BIN%;%MINGW_BIN%;%PATH%"

echo Building HappyMealsServer...
qmake HappyMealsServer.pro
if errorlevel 1 goto error

mingw32-make
if errorlevel 1 goto error

echo Deploying Qt runtime...
"%QT_BIN%\windeployqt.exe" release\HappyMealsServer.exe
if errorlevel 1 goto error

echo.
echo Starting HappyMealsServer on port 40000...
echo Keep this window open while the server is running.
echo.
release\HappyMealsServer.exe
goto end

:error
echo.
echo Build or deploy failed. Check the messages above.
pause

:end
endlocal
