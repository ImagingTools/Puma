# SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
<#
.SYNOPSIS
    CI entry point for the Tenant API Postman suite against the SQLite
    backend: resets the puma_test.sqlite file, starts PumaServerSlTest.exe,
    runs newman against it, and tears the server back down. Analogous to
    Run-CiTests.ps1 (Postgres variant), intended to be invoked as a single
    TeamCity build step (PowerShell runner).

.PARAMETER RepoRoot
    Root of the Puma checkout. Defaults to the PUMADIR environment
    variable (the convention already used across this repo/org for
    locating the checkout on any given machine); if PUMADIR isn't set,
    falls back to two levels above this script (Tests\TenantApiPostman\..\..).

.PARAMETER BuildConfig
    Name of the Bin\<BuildConfig> folder to look for PumaServerSlTest.exe
    in (e.g. "Release_Qt6_VC17_x64"). Ignored if
    -ServerExePath is passed explicitly.

.PARAMETER ServerExePath
    Full path to PumaServerSlTest.exe. Overrides -BuildConfig when set.
    Pass this explicitly on CI if the build output lives somewhere other
    than RepoRoot\Bin\<BuildConfig>.

.PARAMETER HttpPort
    Port the test server listens on (must match PumaServerSlTest.acc's
    HttpPort attribute, currently 17789).

.PARAMETER SqliteDbPath
    Full path to the puma_test.sqlite file (must match PumaServerSlTest.acc's
    DbFile attribute, currently "Puma/Puma Server/puma_test.sqlite" resolved
    relative to the CommonAppData root, i.e. $env:PUBLIC\ImagingTools\...).
    Deleted (along with -wal/-shm sidecars) before each run so
    PumaServerSlTest.exe recreates it from scratch via its
    CompositeMigrationController, matching Run-CiTests.ps1's Postgres
    drop-and-let-the-exe-recreate-it behavior.

.PARAMETER JUnitReportPath
    Where newman writes the JUnit XML report. Point TeamCity's "XML Report
    Processing" (JUnit) build feature at this same path.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-CiTests-Sqlite.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-CiTests-Sqlite.ps1 `
        -BuildConfig "Release_Qt6_VC17_x64"
#>

[CmdletBinding()]
param(
    # Same resolution strategy as Run-CiTests.ps1 - see the comment there for
    # why $PSScriptRoot/$PSCommandPath/$MyInvocation aren't reliable alone
    # under TeamCity's PowerShell step.
    [string]$RepoRoot = $(
        $collectionRelPath = "Tests\TenantApiPostman\Tenant_System_Full.postman_collection.json"
        if ($env:PUMADIR -and (Test-Path (Join-Path $env:PUMADIR $collectionRelPath))) {
            $env:PUMADIR
        }
        elseif (Test-Path (Join-Path (Get-Location).Path $collectionRelPath)) {
            (Get-Location).Path
        }
        else {
            $sd = if ($PSScriptRoot) { $PSScriptRoot }
                  elseif ($PSCommandPath) { Split-Path -Parent $PSCommandPath }
                  elseif ($MyInvocation.MyCommand.Path) { Split-Path -Parent $MyInvocation.MyCommand.Path }
                  else { $null }
            if ($sd) {
                (Resolve-Path (Join-Path $sd "..\..")).Path
            }
            else {
                throw "Unable to determine the repo checkout root (PUMADIR unset/stale, working directory isn't the checkout root, and this script's own path could not be determined). Pass -RepoRoot explicitly."
            }
        }
    ),
    [string]$ScriptDir = (Join-Path $RepoRoot "Tests\TenantApiPostman"),
    [string]$BuildConfig = "Release_Qt6_VC17_x64",
    [string]$ServerExePath = "",
    [int]$HttpPort = 17789,
    [string]$SqliteDbPath = (Join-Path $env:PUBLIC "ImagingTools\Puma\Puma Server\puma_test.sqlite"),
    [string]$CollectionPath = (Join-Path $ScriptDir "Tenant_System_Full.postman_collection.json"),
    [string]$EnvironmentPath = (Join-Path $ScriptDir "Tenant_System_Full.postman_environment.json"),
    [string]$JUnitReportPath = (Join-Path $ScriptDir "junit-report-sqlite.xml"),
    [string]$JsonReportPath = (Join-Path $ScriptDir "run-report-sqlite.json"),
    [int]$StartupTimeoutSeconds = 60
)

$ErrorActionPreference = "Stop"
$serverProcess = $null
$exitCode = 1

if ([string]::IsNullOrWhiteSpace($ServerExePath)) {
    $ServerExePath = Join-Path $RepoRoot "Bin\$BuildConfig\PumaServerSlTest.exe"
}

function Write-Step($message) {
    Write-Host "`n=== $message ===" -ForegroundColor Cyan
}

function Stop-TestServer {
    Write-Step "Stopping any running PumaServerSlTest.exe"
    Get-Process -Name "PumaServerSlTest" -ErrorAction SilentlyContinue | ForEach-Object {
        Write-Host "Killing PID $($_.Id)"
        Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue
    }
    Start-Sleep -Seconds 2
}

function Reset-TestDatabase {
    # Only deletes the sqlite file (+ -wal/-shm sidecars) if present.
    # PumaServerSlTest.exe creates it (and runs migrations 0-5 via its
    # CompositeMigrationController) on startup when it doesn't already
    # exist, so there's nothing else to do here - a stray leftover file
    # from a previous run is the only thing that would make a run start
    # from non-empty state.
    Write-Step "Resetting SQLite database '$SqliteDbPath'"
    foreach ($suffix in "", "-wal", "-shm") {
        $path = "$SqliteDbPath$suffix"
        if (Test-Path $path) {
            Write-Host "Removing $path"
            Remove-Item -Path $path -Force -ErrorAction SilentlyContinue
        }
    }
}

function Start-TestServer {
    Write-Step "Starting PumaServerSlTest.exe"
    if (-not (Test-Path $ServerExePath)) {
        throw "Server executable not found: $ServerExePath"
    }
    $workDir = Split-Path -Parent $ServerExePath
    $script:serverProcess = Start-Process -FilePath $ServerExePath -WorkingDirectory $workDir -PassThru -WindowStyle Hidden
    Write-Host "Started PID $($script:serverProcess.Id)"

    $deadline = (Get-Date).AddSeconds($StartupTimeoutSeconds)
    $ready = $false
    while ((Get-Date) -lt $deadline) {
        if ($script:serverProcess.HasExited) {
            throw "PumaServerSlTest.exe exited prematurely (exit code $($script:serverProcess.ExitCode))"
        }
        $probe = Test-NetConnection -ComputerName "localhost" -Port $HttpPort -WarningAction SilentlyContinue -InformationLevel Quiet
        if ($probe) { $ready = $true; break }
        Start-Sleep -Milliseconds 500
    }
    if (-not $ready) {
        throw "Server did not open port $HttpPort within $StartupTimeoutSeconds seconds"
    }
    Write-Host "Server is accepting connections on port $HttpPort"
}

function Install-NewmanIfNeeded {
    # newman is pinned as a local devDependency in this folder's package.json
    # rather than assumed to be installed globally on the agent - a fresh or
    # pooled/reprovisioned TeamCity agent otherwise has no way to run it.
    $newmanCmd = Join-Path $ScriptDir "node_modules\.bin\newman.cmd"
    if (Test-Path $newmanCmd) {
        return $newmanCmd
    }

    Write-Step "Installing newman (npm install in $ScriptDir)"
    Push-Location $ScriptDir
    try {
        & npm install --no-audit --no-fund | Out-Host
        if ($LASTEXITCODE -ne 0) { throw "npm install failed (exit $LASTEXITCODE)" }
    }
    finally {
        Pop-Location
    }

    if (-not (Test-Path $newmanCmd)) {
        throw "npm install completed but newman binary still not found at: $newmanCmd"
    }
    return $newmanCmd
}

function Invoke-NewmanSuite {
    $newmanCmd = Install-NewmanIfNeeded

    Write-Step "Running newman suite"
    $baseUrl = "http://localhost:$HttpPort/Puma/graphql"
    $newmanArgs = @(
        "run", $CollectionPath,
        "-e", $EnvironmentPath,
        "--env-var", "base_url=$baseUrl",
        "--reporters", "cli,junit,json",
        "--reporter-junit-export", $JUnitReportPath,
        "--reporter-json-export", $JsonReportPath
    )
    & $newmanCmd @newmanArgs | Out-Host
    return $LASTEXITCODE
}

$repoRootSource = if ($PSBoundParameters.ContainsKey('RepoRoot')) { 'explicit -RepoRoot' }
    elseif ($env:PUMADIR -eq $RepoRoot) { 'PUMADIR env var' }
    elseif ((Get-Location).Path -eq $RepoRoot) { 'working directory' }
    else { 'script-relative fallback' }

Write-Step "Resolved paths"
Write-Host "RepoRoot:       $RepoRoot  (source: $repoRootSource)"
Write-Host "ScriptDir:      $ScriptDir"
Write-Host "ServerExePath:  $ServerExePath"
Write-Host "SqliteDbPath:   $SqliteDbPath"
Write-Host "CollectionPath: $CollectionPath"
Write-Host "EnvironmentPath: $EnvironmentPath"

try {
    Stop-TestServer
    Reset-TestDatabase
    Start-TestServer
    $exitCode = Invoke-NewmanSuite
}
finally {
    if ($serverProcess -and -not $serverProcess.HasExited) {
        Write-Step "Stopping PumaServerSlTest.exe (PID $($serverProcess.Id))"
        Stop-Process -Id $serverProcess.Id -Force -ErrorAction SilentlyContinue
    }
}

if ($exitCode -eq 0) {
    Write-Host "`nAll tests passed." -ForegroundColor Green
} else {
    Write-Host "`nTest run failed (newman exit code $exitCode)." -ForegroundColor Red
}

exit $exitCode
