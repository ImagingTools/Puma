@echo off
REM Startup script for Docker test environment
REM This script builds the PumaServerPg installer, installs it silently, and starts the server.

setlocal enabledelayedexpansion

echo ========================================
echo PumaServerPg Installation
echo ========================================

REM --- Configuration ---
set "PUMA_ROOT=C:\app\resources"
set "INSTALL_DIR=%PUMA_ROOT%\Install\PumaServerPg"
set "OUTPUT_DIR=%INSTALL_DIR%\Output"
set "INSTALLER_EXE=%OUTPUT_DIR%\PumaServerInstall.exe"
set "INSTALL_TARGET=C:\Program Files\ImagingTools\PumaServer"
set "SERVER_EXE=%INSTALL_TARGET%\PumaServerPg.exe"
set "SERVER_PORT=7788"

REM --- Step 1: Build the installer ---
echo.
echo Step 1: Building PumaServerPg installer...

if not exist "%INSTALL_DIR%\install_ReleaseVC17_64.bat" (
    echo [ERROR] install_ReleaseVC17_64.bat not found at: %INSTALL_DIR%
    echo Make sure the Puma repository is mounted to C:\app\resources
    exit /b 1
)

pushd "%INSTALL_DIR%"
call install_ReleaseVC17_64.bat
if errorlevel 1 (
    echo [ERROR] Installer build failed
    popd
    exit /b 1
)
popd

REM --- Step 2: Verify installer was created ---
echo.
echo Step 2: Verifying installer...

if not exist "%INSTALLER_EXE%" (
    echo [ERROR] Installer not found at: %INSTALLER_EXE%
    echo Expected output from Inno Setup: PumaServerInstall.exe
    exit /b 1
)

echo [OK] Installer found: %INSTALLER_EXE%

REM --- Step 3: Run installer silently ---
echo.
echo Step 3: Installing PumaServerPg silently...

"%INSTALLER_EXE%" /VERYSILENT /SUPPRESSMSGBOXES /NORESTART /DIR="%INSTALL_TARGET%"
if errorlevel 1 (
    echo [ERROR] Silent installation failed
    exit /b 1
)

echo [OK] PumaServerPg installed to: %INSTALL_TARGET%

REM --- Step 4: Start PumaServerPg ---
echo.
echo Step 4: Starting PumaServerPg...

if not exist "%SERVER_EXE%" (
    echo [ERROR] PumaServerPg.exe not found at: %SERVER_EXE%
    exit /b 1
)

REM Start PumaServerPg in background
start "" "%SERVER_EXE%"

REM Wait for the server to become available
echo Waiting for PumaServerPg to start on port %SERVER_PORT%...
set ATTEMPTS=0

:WAIT_SERVER
set /a ATTEMPTS+=1
if !ATTEMPTS! gtr 60 (
    echo [ERROR] PumaServerPg failed to start after 60 attempts
    exit /b 1
)

powershell -NoProfile -Command "try { $r = Invoke-WebRequest -Uri 'http://localhost:%SERVER_PORT%/Puma/graphql' -Method POST -ContentType 'application/json' -Body '{\"query\":\"{__typename}\"}' -UseBasicParsing -TimeoutSec 2; exit 0 } catch { exit 1 }"
if errorlevel 1 (
    echo   Attempt !ATTEMPTS!/60 - waiting...
    timeout /t 2 /nobreak >nul
    goto WAIT_SERVER
)

echo [OK] PumaServerPg is running on port %SERVER_PORT%
echo.
echo ========================================
echo PumaServerPg ready for testing
echo ========================================

exit /b 0
