# SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
<#
.SYNOPSIS
    Runs the native pumatest.exe suite (CAuthClientSdkTest, CPersonalAccessTokenTest,
    CAuthServerLifecycleTest, CAuthorizationControllerTest - every class registered
    via I_ADD_TEST in Include/pumatest) against a live Puma backend, starting one
    if needed.

.DESCRIPTION
    CAuthClientSdkTest and CPersonalAccessTokenTest drive AuthClientSdk over the
    network through an embedded AuthServerSdk instance, which in turn delegates
    user/role/group/PAT/login operations to a real Puma server reachable via
    AuthServerSdk::SetPumaConnectionParam() - hardcoded in those test files to
    PumaServerSlTest.acc's ports (17789 HTTP / 18789 WebSocket), i.e. the isolated,
    disposable-DB test server variant, matching how Run-CiTests-Sqlite.ps1
    (Tests/TenantApiPostman) drives the same executable. This is deliberately NOT
    PumaServerSl.exe's ports (7788/8788) - this suite must never touch a
    developer's persistent local Puma database.

    If a server is already listening on -HttpPort, this script reuses it instead of
    starting a new one (and will not stop it afterwards) - handy for iterating on
    tests against a server you're already running. Otherwise it starts
    PumaServerSlTest.exe itself, optionally resetting its SQLite file first, and
    stops it again when done.

.PARAMETER RepoRoot
    Root of the Puma checkout. Defaults to the PUMADIR environment variable (same
    convention as Tests/TenantApiPostman/Run-CiTests*.ps1); falls back to two
    levels above this script if PUMADIR is unset/stale.

.PARAMETER BuildConfig
    Name of the Bin\<BuildConfig> folder to look for PumaServerSlTest.exe and
    pumatest.exe in (e.g. "Debug_Qt6_VC17_x64", "Release_Qt6_VC17_x64"). Ignored
    for whichever of -ServerExePath/-TestExePath is passed explicitly.

.PARAMETER ServerExePath
    Full path to PumaServerSlTest.exe. Overrides -BuildConfig when set.

.PARAMETER TestExePath
    Full path to pumatest.exe. Overrides -BuildConfig when set.

.PARAMETER HttpPort
    Port PumaServerSlTest.exe listens on for HTTP (must match the 17789 hardcoded
    as AuthServerSdk::SetPumaConnectionParam's httpPort in CAuthClientSdkTest.cpp /
    CPersonalAccessTokenTest.cpp - do not change without updating those files too).

.PARAMETER WsPort
    WebSocket counterpart of -HttpPort (18789, see above).

.PARAMETER ResetDatabase
    Delete PumaServerSlTest's puma_test.sqlite (+ -wal/-shm sidecars) before
    starting it, so it recreates a clean schema via its migration controller -
    same behavior as Run-CiTests-Sqlite.ps1's Reset-TestDatabase. Only applies
    when this script starts the server itself (ignored if reusing an
    already-running one). Off by default: the pumatest.exe classes are written to
    be idempotent against existing data.

.PARAMETER SqliteDbPath
    Full path to puma_test.sqlite, used only when -ResetDatabase is set.

.PARAMETER TestArgs
    Extra arguments forwarded verbatim to pumatest.exe (a QTest::qExec-based
    runner applied to every registered test class - e.g. "-v2" for verbose
    output). Without arguments, pumatest.exe runs every registered test class.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1 -BuildConfig "Release_Qt6_VC17_x64" -TestArgs "-v2"

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File Run-AuthSdkTests.ps1 -ResetDatabase
#>

[CmdletBinding()]
param(
    [string]$RepoRoot = $(
        $markerRelPath = "Include\pumatest\PumaTestMain.cpp"
        if ($env:PUMADIR -and (Test-Path (Join-Path $env:PUMADIR $markerRelPath))) {
            $env:PUMADIR
        }
        elseif (Test-Path (Join-Path (Get-Location).Path $markerRelPath)) {
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
    [string]$BuildConfig = "Debug_Qt6_VC17_x64",
    [string]$ServerExePath = "",
    [string]$TestExePath = "",
    [int]$HttpPort = 17789,
    [int]$WsPort = 18789,
    [switch]$ResetDatabase,
    [string]$SqliteDbPath = (Join-Path $env:PUBLIC "ImagingTools\Puma\Puma Server\puma_test.sqlite"),
    [string[]]$TestArgs = @(),
    [int]$StartupTimeoutSeconds = 60
)

$ErrorActionPreference = "Stop"
$serverProcess = $null
$ownsServerProcess = $false
$exitCode = 1

if ([string]::IsNullOrWhiteSpace($ServerExePath)) {
    $ServerExePath = Join-Path $RepoRoot "Bin\$BuildConfig\PumaServerSlTest.exe"
}
if ([string]::IsNullOrWhiteSpace($TestExePath)) {
    $TestExePath = Join-Path $RepoRoot "Bin\$BuildConfig\pumatest.exe"
}

function Write-Step($message) {
    Write-Host "`n=== $message ===" -ForegroundColor Cyan
}

function Test-PortOpen([int]$port) {
    return Test-NetConnection -ComputerName "localhost" -Port $port -WarningAction SilentlyContinue -InformationLevel Quiet
}

function Reset-TestDatabase {
    Write-Step "Resetting SQLite database '$SqliteDbPath'"
    foreach ($suffix in "", "-wal", "-shm") {
        $path = "$SqliteDbPath$suffix"
        if (Test-Path $path) {
            Write-Host "Removing $path"
            Remove-Item -Path $path -Force -ErrorAction SilentlyContinue
        }
    }
}

function Disable-TestLdapAuth {
    # LdapEnabled defaults to true (Partitura/PumaVoce.arp/PumaSettings.acc) so that
    # production Puma installs can authenticate domain accounts out of the box - but
    # PumaServerSlTest only ever has DB-only test users, so every single login first
    # tries (and fails) a Windows LogonUser() call before falling back to the local
    # DB check. That's ~20+ failed LogonUser attempts per test run, pure noise. Only
    # patches an existing settings file (first-ever run on a machine still gets the
    # product default of enabled; rerun this script once and it will be disabled from
    # then on).
    $settingsPath = Join-Path (Split-Path -Parent $SqliteDbPath) "PumaServerSlTestSettings.xml"
    if (-not (Test-Path $settingsPath)) {
        return
    }
    $content = Get-Content -Path $settingsPath -Raw
    $patched = $content -replace '(<Parameter Id="LdapEnabled" IsEnabled=")true(")', '${1}false${2}'
    if ($patched -ne $content) {
        Write-Host "Disabling LdapEnabled in '$settingsPath' (avoids per-login LogonUser noise in test runs)"
        Set-Content -Path $settingsPath -Value $patched -NoNewline
    }
}

function Start-PumaServer {
    Write-Step "Starting PumaServerSlTest.exe"
    if (-not (Test-Path $ServerExePath)) {
        throw "Server executable not found: $ServerExePath (build it, or pass -ServerExePath / -BuildConfig)"
    }
    if ($ResetDatabase) {
        Reset-TestDatabase
    }
    Disable-TestLdapAuth
    $workDir = Split-Path -Parent $ServerExePath
    $script:serverProcess = Start-Process -FilePath $ServerExePath -WorkingDirectory $workDir -PassThru -WindowStyle Hidden
    $script:ownsServerProcess = $true
    Write-Host "Started PID $($script:serverProcess.Id)"

    $deadline = (Get-Date).AddSeconds($StartupTimeoutSeconds)
    $ready = $false
    while ((Get-Date) -lt $deadline) {
        if ($script:serverProcess.HasExited) {
            throw "PumaServerSlTest.exe exited prematurely (exit code $($script:serverProcess.ExitCode)) - check Puma/Puma Server/PumaServerSlTestLog.txt under %PUBLIC%/ImagingTools"
        }
        if (Test-PortOpen $HttpPort) { $ready = $true; break }
        Start-Sleep -Milliseconds 500
    }
    if (-not $ready) {
        throw "Server did not open port $HttpPort within $StartupTimeoutSeconds seconds"
    }
    Write-Host "Server is accepting connections on port $HttpPort"
}

function Sync-PluginDlls {
    # Known packaging gap (see Docs/GQL_CONTEXT_THREAD_PROPAGATION_BUG.md): AuthClientSdk.dll/
    # AuthServerSdk.dll build into Bin\<config>\Plugins\, but pumatest.exe's import table
    # needs them next to itself (Windows default DLL search order excludes Plugins\).
    # Without this, pumatest.exe silently loads a stale copy left over from a previous
    # build/copy, which looks exactly like a regression.
    $testDir = Split-Path -Parent $TestExePath
    $pluginsDir = Join-Path $testDir "Plugins"
    foreach ($dllName in @("AuthClientSdk.dll", "AuthServerSdk.dll")) {
        $sourcePath = Join-Path $pluginsDir $dllName
        $destPath = Join-Path $testDir $dllName
        if (-not (Test-Path $sourcePath)) {
            continue
        }
        if ((Test-Path $destPath) -and (Get-Item $destPath).LastWriteTimeUtc -ge (Get-Item $sourcePath).LastWriteTimeUtc) {
            continue
        }
        Write-Host "Copying newer $dllName from Plugins\ to $testDir"
        Copy-Item -Path $sourcePath -Destination $destPath -Force
    }
}

function Invoke-PumaTest {
    Write-Step "Running pumatest.exe"
    if (-not (Test-Path $TestExePath)) {
        throw "Test executable not found: $TestExePath (build it, or pass -TestExePath / -BuildConfig)"
    }
    Sync-PluginDlls
    & $TestExePath @TestArgs | Out-Host
    return $LASTEXITCODE
}

Write-Step "Resolved paths"
Write-Host "RepoRoot:      $RepoRoot"
Write-Host "ServerExePath: $ServerExePath"
Write-Host "TestExePath:   $TestExePath"
Write-Host "HttpPort/WsPort: $HttpPort / $WsPort"

try {
    if (Test-PortOpen $HttpPort) {
        Write-Step "Port $HttpPort is already accepting connections - reusing the running server (not starting, resetting, or stopping one)"
    }
    else {
        Start-PumaServer
    }

    $exitCode = Invoke-PumaTest
}
finally {
    if ($ownsServerProcess -and $serverProcess -and -not $serverProcess.HasExited) {
        Write-Step "Stopping PumaServerSlTest.exe (PID $($serverProcess.Id))"
        Stop-Process -Id $serverProcess.Id -Force -ErrorAction SilentlyContinue
    }
}

if ($exitCode -eq 0) {
    Write-Host "`nAll tests passed." -ForegroundColor Green
} else {
    Write-Host "`nTest run failed (pumatest.exe exit code $exitCode)." -ForegroundColor Red
}

exit $exitCode
