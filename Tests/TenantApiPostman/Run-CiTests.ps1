# SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
<#
.SYNOPSIS
    CI entry point for the Tenant API Postman suite: resets the puma_test
    database, starts PumaServerPgTest.exe, runs newman against it, and
    tears the server back down. Intended to be invoked as a single
    TeamCity build step (PowerShell runner).

.PARAMETER RepoRoot
    Root of the Puma checkout. Defaults to the PUMADIR environment
    variable (the convention already used across this repo/org for
    locating the checkout on any given machine); if PUMADIR isn't set,
    falls back to two levels above this script (Tests\TenantApiPostman\..\..).

.PARAMETER BuildConfig
    Name of the Bin\<BuildConfig> folder to look for PumaServerPgTest.exe
    in (e.g. "Release_Qt6_VC17_x64"). Ignored if
    -ServerExePath is passed explicitly.

.PARAMETER ServerExePath
    Full path to PumaServerPgTest.exe. Overrides -BuildConfig when set.
    Pass this explicitly on CI if the build output lives somewhere other
    than RepoRoot\Bin\<BuildConfig>.

.PARAMETER HttpPort
    Port the test server listens on (must match PumaServerPgTest.acc's
    HttpPort attribute, currently 17788).

.PARAMETER DbName / DbHost / DbPort / DbUser / DbPassword
    Postgres connection used to drop/recreate the test database before
    each run (must match PumaDatabasePostgres.acc).

.PARAMETER PsqlPath
    Full path to psql.exe. If omitted, resolved from PATH, then from the
    default PostgreSQL install locations (highest version wins). Pass
    this explicitly if the CI agent has Postgres installed somewhere
    non-standard.

.PARAMETER JUnitReportPath
    Where newman writes the JUnit XML report. Point TeamCity's "XML Report
    Processing" (JUnit) build feature at this same path.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-CiTests.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-CiTests.ps1 `
        -BuildConfig "Release_Qt6_VC17_x64" -DbPassword "%db.password%"
#>

[CmdletBinding()]
param(
    # Resolving the repo root purely from PowerShell's automatic script-path
    # variables ($PSScriptRoot / $PSCommandPath / $MyInvocation) is NOT
    # reliable across every CI runner - observed all three empty under
    # TeamCity's PowerShell step despite -File being passed explicitly.
    # TeamCity does reliably set the working directory to the checkout root
    # for build steps (confirmed from agent logs), so that's tried first,
    # self-validated against a file we know must exist at a fixed relative
    # location. PUMADIR (this org's own checkout-location convention) is
    # tried before that; the fragile automatic variables are the last resort,
    # kept for convenience when invoking the script directly during local dev.
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
    [int]$HttpPort = 17788,
    [string]$DbName = "puma_test",
    [string]$DbHost = "localhost",
    [int]$DbPort = 5432,
    [string]$DbUser = "postgres",
    [string]$DbPassword = "root",
    [string]$PsqlPath = "",
    [string]$CollectionPath = (Join-Path $ScriptDir "Tenant_System_Full.postman_collection.json"),
    [string]$EnvironmentPath = (Join-Path $ScriptDir "Tenant_System_Full.postman_environment.json"),
    [string]$JUnitReportPath = (Join-Path $ScriptDir "junit-report.xml"),
    [string]$JsonReportPath = (Join-Path $ScriptDir "run-report.json"),
    [int]$StartupTimeoutSeconds = 60
)

$ErrorActionPreference = "Stop"
$serverProcess = $null
$exitCode = 1

if ([string]::IsNullOrWhiteSpace($ServerExePath)) {
    $ServerExePath = Join-Path $RepoRoot "Bin\$BuildConfig\PumaServerPgTest.exe"
}

function Write-Step($message) {
    Write-Host "`n=== $message ===" -ForegroundColor Cyan
}

function Resolve-PsqlPath {
    if (-not [string]::IsNullOrWhiteSpace($PsqlPath)) {
        if (-not (Test-Path $PsqlPath)) { throw "psql not found at explicit path: $PsqlPath" }
        return $PsqlPath
    }

    $onPath = Get-Command psql.exe -ErrorAction SilentlyContinue
    if ($onPath) { return $onPath.Source }

    $candidates = Get-ChildItem "$env:ProgramFiles\PostgreSQL\*\bin\psql.exe" -ErrorAction SilentlyContinue |
        Sort-Object { [int]([regex]::Match($_.Directory.Parent.Name, '\d+').Value) } -Descending
    if ($candidates) { return $candidates[0].FullName }

    throw "Could not locate psql.exe. Pass -PsqlPath explicitly or add PostgreSQL\bin to PATH on this agent."
}

function Stop-TestServer {
    Write-Step "Stopping any running PumaServerPgTest.exe"
    Get-Process -Name "PumaServerPgTest" -ErrorAction SilentlyContinue | ForEach-Object {
        Write-Host "Killing PID $($_.Id)"
        Stop-Process -Id $_.Id -Force -ErrorAction SilentlyContinue
    }
    Start-Sleep -Seconds 2
}

function Reset-TestDatabase {
    # Only drops the database if present. PumaServerPgTest.exe creates it
    # (and runs migrations 0-5 via its CompositeMigrationController) on
    # startup when it doesn't already exist, so there's nothing else to do
    # here - a stray leftover DB is the only thing that would make a run
    # start from non-empty state.
    Write-Step "Resetting database '$DbName'"
    $psql = Resolve-PsqlPath
    Write-Host "Using psql: $psql"
    $env:PGPASSWORD = $DbPassword
    # psql writes routine NOTICEs (e.g. "database does not exist, skipping")
    # to stderr. Under $ErrorActionPreference = "Stop" (set globally in this
    # script), PowerShell 5.1 turns *any* stderr line from a native command
    # into a terminating NativeCommandError regardless of the process's own
    # exit code - so this must run under "Continue" and be judged solely by
    # $LASTEXITCODE.
    $previousEap = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        & $psql -h $DbHost -p $DbPort -U $DbUser -d postgres -v ON_ERROR_STOP=1 -c "SELECT pg_terminate_backend(pid) FROM pg_stat_activity WHERE datname = '$DbName' AND pid <> pg_backend_pid();" 2>&1 | Write-Host
        & $psql -h $DbHost -p $DbPort -U $DbUser -d postgres -v ON_ERROR_STOP=1 -c "DROP DATABASE IF EXISTS $DbName;" 2>&1 | Write-Host
        if ($LASTEXITCODE -ne 0) { throw "Failed to drop database '$DbName' (exit $LASTEXITCODE)" }
    }
    finally {
        $ErrorActionPreference = $previousEap
        Remove-Item Env:\PGPASSWORD -ErrorAction SilentlyContinue
    }
}

function Start-TestServer {
    Write-Step "Starting PumaServerPgTest.exe"
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
            throw "PumaServerPgTest.exe exited prematurely (exit code $($script:serverProcess.ExitCode))"
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
        Write-Step "Stopping PumaServerPgTest.exe (PID $($serverProcess.Id))"
        Stop-Process -Id $serverProcess.Id -Force -ErrorAction SilentlyContinue
    }
}

if ($exitCode -eq 0) {
    Write-Host "`nAll tests passed." -ForegroundColor Green
} else {
    Write-Host "`nTest run failed (newman exit code $exitCode)." -ForegroundColor Red
}

exit $exitCode
