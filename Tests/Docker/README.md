# Docker-Based Integration Tests for Puma

## Overview

This directory contains scripts for running PumaServerPg integration tests in Docker containers using the **ImtCore Docker testing infrastructure**.

The test pipeline:
1. Builds the PumaServerPg installer (via Inno Setup)
2. Installs PumaServerPg silently in the container
3. Starts PumaServerPg (listening on port 7788)
4. Runs the `TenantApiPostman` collection against the server via Newman

## Prerequisites

- **Windows 10/11 Pro** or **Windows Server** with Docker Desktop (Windows containers mode)
- **ImtCore** repository cloned alongside Puma (or `IMTCOREDIR` env variable set)
- **imtcore-tests:windows** Docker image built:
  ```cmd
  cd %IMTCOREDIR%
  Tests\Docker\Scripts\build-docker-windows.bat
  ```
- **Inno Setup** (`iscc.exe`) available in the container or build environment
- Pre-built Puma binaries in `Bin\Release_Qt6_VC17_x64`

## Directory Structure

```
Tests/Docker/
├── README.md                  # This file
├── run-tests-windows.bat      # Main entry point (calls ImtCore core script)
├── prepare-tests.bat          # Copies Postman files to Tests/API
└── Startup/
    └── 01_install_puma.bat    # Builds installer, installs & starts PumaServerPg
```

## How It Works

The ImtCore Docker test infrastructure provides:
- A Windows Server container with Node.js, PostgreSQL, and Newman pre-installed
- An entrypoint script that:
  1. Starts PostgreSQL
  2. Runs all `*.bat` scripts from the `/app/startup/` folder (alphabetically)
  3. Auto-detects and runs Postman collections from `/app/tests/API/`

### Volume Mounts (handled by ImtCore core script)

| Host Path | Container Path | Purpose |
|-----------|---------------|---------|
| `Tests/Docker/Startup/` | `C:\app\startup\` | Install & start PumaServerPg |
| `Tests/API/` | `C:\app\tests\API\` | Postman collections for Newman |
| `Puma repo root` | `C:\app\resources\` | Source for installer build |

## Quick Start

```cmd
REM 1. Build the ImtCore Docker image (one-time)
cd %IMTCOREDIR%
Tests\Docker\Scripts\build-docker-windows.bat

REM 2. Navigate to Puma test directory
cd Puma\Tests\Docker

REM 3. Prepare API test files
prepare-tests.bat

REM 4. Run tests
run-tests-windows.bat
```

## Test Results

Results are written to `Tests\test-results\` (or `Tests\Docker\..\test-results\`) and include:
- `newman-*.xml` — JUnit XML reports
- `newman-*.json` — JSON reports with full request/response details

## Configuration

Edit `run-tests-windows.bat` to change:
- `BASE_URL` — Server endpoint (default: `http://localhost:7788`)
- `POSTGRES_PASSWORD` — Database password (default: `root`)
- `RUN_GUI_TESTS` / `RUN_API_TESTS` — Enable/disable test types

## Troubleshooting

### Installer build fails
- Ensure `iscc.exe` (Inno Setup Compiler) is in PATH or installed in the container
- Verify pre-built binaries exist at `Bin\Release_Qt6_VC17_x64`

### Server doesn't start
- Check PostgreSQL is running (the entrypoint handles this automatically)
- Verify port 7788 is not blocked by container network policies

### Newman tests fail
- Ensure the server is responding at `http://localhost:7788/Puma/graphql`
- Check that the environment file has correct `base_url` variable
