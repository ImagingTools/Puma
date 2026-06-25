@echo off
REM ==========================================
REM Puma Docker Tests - Preparation Script
REM ==========================================
REM This script prepares the test directory layout expected by ImtCore Docker.
REM It copies the Postman collection and environment into the Tests/API folder
REM so the ImtCore entrypoint auto-detects and runs them via Newman.
REM
REM Run this BEFORE run-tests-windows.bat if Tests/API is not populated.

setlocal

set "SCRIPT_DIR=%~dp0"
set "PUMA_ROOT=%SCRIPT_DIR%..\.."
set "POSTMAN_SRC=%PUMA_ROOT%\Tests\TenantApiPostman"
set "API_DEST=%SCRIPT_DIR%..\API"

echo Preparing API test files...

REM Create API directory
if not exist "%API_DEST%" mkdir "%API_DEST%"

REM Copy Postman collection
if exist "%POSTMAN_SRC%\Tenant_System_Full.postman_collection.json" (
    copy /Y "%POSTMAN_SRC%\Tenant_System_Full.postman_collection.json" "%API_DEST%\" >nul
    echo [OK] Copied collection
) else (
    echo [ERROR] Collection file not found
    exit /b 1
)

REM Copy environment file
if exist "%POSTMAN_SRC%\Tenant_System_Full.postman_environment.json" (
    copy /Y "%POSTMAN_SRC%\Tenant_System_Full.postman_environment.json" "%API_DEST%\" >nul
    echo [OK] Copied environment
) else (
    echo [ERROR] Environment file not found
    exit /b 1
)

REM Copy iteration data file if present
if exist "%POSTMAN_SRC%\permission_visibility_matrix.iteration.json" (
    copy /Y "%POSTMAN_SRC%\permission_visibility_matrix.iteration.json" "%API_DEST%\" >nul
    echo [OK] Copied iteration data
)

echo.
echo [OK] API test files prepared in: %API_DEST%
echo.
echo Now run: run-tests-windows.bat

exit /b 0
