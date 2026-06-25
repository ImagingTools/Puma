@echo off
REM ==========================================
REM Puma Docker Tests - Configuration
REM ==========================================
REM This script runs PumaServerPg in a Docker container and executes
REM the TenantApiPostman collection against it using the ImtCore Docker infrastructure.
REM
REM Prerequisites:
REM   - Docker Desktop with Windows containers enabled
REM   - imtcore-tests:windows image built (see ImtCore/Tests/Docker/Scripts/build-docker-windows.bat)
REM   - IMTCOREDIR environment variable set (or ImtCore at ..\..\ImtCore)
REM
REM Usage:
REM   cd Puma\Tests\Docker
REM   run-tests-windows.bat

REM ==========================================
REM CONFIGURATION
REM ==========================================

REM Application URL (PumaServerPg listens on port 7788)
set BASE_URL=http://localhost:7788

REM PostgreSQL password
set POSTGRES_PASSWORD=root

REM Run only API tests (Postman/Newman), no GUI tests
set RUN_GUI_TESTS=false
set RUN_API_TESTS=true

REM ==========================================
REM DO NOT MODIFY BELOW THIS LINE
REM ==========================================

REM Determine ImtCore directory
if not defined IMTCOREDIR (
    for %%A in ("%~dp0..\..\..\ImtCore") do set "IMTCOREDIR=%%~fA"
)

REM Validate ImtCore path
set CORE_SCRIPT_PATH=%IMTCOREDIR%\Tests\Docker\Scripts\run-tests-windows-core.bat
if not exist "%CORE_SCRIPT_PATH%" (
    echo ERROR: Cannot find core script at: %CORE_SCRIPT_PATH%
    echo Please ensure IMTCOREDIR points to a valid ImtCore directory.
    echo Expected fallback location: %~dp0..\..\..\ImtCore
    exit /b 1
)

call "%CORE_SCRIPT_PATH%"
exit /b %ERRORLEVEL%
