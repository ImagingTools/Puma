<#
.SYNOPSIS
    Helpers to run the API suites with tenant Row Level Security (RLS) enforced.

.DESCRIPTION
    The server connects with a restricted application role (NOSUPERUSER NOBYPASSRLS) and gets the
    administrative login only to prepare the database itself: it creates the role and the database,
    installs the extensions and passes the existing objects to the role (see AdminDatabaseAccessSettings
    of the ImtCore database engine). These helpers only write both logins into the server settings file
    and check the isolation in the database after the suite.
#>

function Enable-RlsDatabaseSettings {
    param(
        [Parameter(Mandatory)][string]$SettingsPath,
        [Parameter(Mandatory)][string]$DatabaseParameterId,
        [Parameter(Mandatory)][string]$AdminParameterId,
        [Parameter(Mandatory)][string]$AppUser,
        [Parameter(Mandatory)][string]$AppPassword,
        [Parameter(Mandatory)][string]$AdminUser,
        [Parameter(Mandatory)][string]$AdminPassword
    )

    if (-not (Test-Path $SettingsPath)) {
        throw "Server settings not found: $SettingsPath. Start the server once without -Rls to create them."
    }

    Copy-Item -Path $SettingsPath -Destination "$SettingsPath.rls-backup" -Force

    [xml]$xml = Get-Content -Raw -Path $SettingsPath
    $appNode = $xml.SelectSingleNode("//Parameter[@Id='$DatabaseParameterId']")
    if ($null -eq $appNode) {
        throw "Parameter '$DatabaseParameterId' not found in $SettingsPath"
    }

    $adminNode = $xml.SelectSingleNode("//Parameter[@Id='$AdminParameterId']")
    if ($null -eq $adminNode) {
        $adminNode = $appNode.CloneNode($true)
        $adminNode.SetAttribute("Id", $AdminParameterId)
        [void]$appNode.ParentNode.AppendChild($adminNode)
    }

    $appNode.SetAttribute("UserName", $AppUser)
    $appNode.SetAttribute("Password", $AppPassword)
    $adminNode.SetAttribute("UserName", $AdminUser)
    $adminNode.SetAttribute("Password", $AdminPassword)
    $adminNode.SetAttribute("DatabaseName", "postgres")
    $adminNode.SetAttribute("Host", $appNode.GetAttribute("Host"))
    $adminNode.SetAttribute("Port", $appNode.GetAttribute("Port"))

    $xml.Save($SettingsPath)
    Write-Host "RLS mode: $SettingsPath uses application role '$AppUser', administrative login '$AdminUser'"
}

function Restore-RlsDatabaseSettings {
    param([Parameter(Mandatory)][string]$SettingsPath)

    if (Test-Path "$SettingsPath.rls-backup") {
        Move-Item -Path "$SettingsPath.rls-backup" -Destination $SettingsPath -Force
    }
}

function Invoke-RlsIsolationCheck {
    <#
        Runs an isolation check script as the application role. Every NOTICE starting with PASS or FAIL
        becomes a JUnit test case; the result is 0 only if there is at least one PASS and no FAIL.
    #>
    param(
        [Parameter(Mandatory)][string]$PsqlPath,
        [Parameter(Mandatory)][string]$DbHost,
        [Parameter(Mandatory)][int]$DbPort,
        [Parameter(Mandatory)][string]$DbName,
        [Parameter(Mandatory)][string]$AppUser,
        [Parameter(Mandatory)][string]$AppPassword,
        [Parameter(Mandatory)][string]$ScriptPath,
        [Parameter(Mandatory)][string]$JUnitPath,
        [string]$SuiteName = "TenantRowLevelSecurity",
        # Optional fixtures (e.g. tenant bindings) written by the administrative login before the check
        [string]$SetupScriptPath = "",
        [string]$AdminUser = "",
        [string]$AdminPassword = ""
    )

    $previousEap = $ErrorActionPreference
    $ErrorActionPreference = "Continue"
    try {
        if ($SetupScriptPath) {
            $env:PGPASSWORD = $AdminPassword
            & $PsqlPath -h $DbHost -p $DbPort -U $AdminUser -d $DbName -v ON_ERROR_STOP=1 -q -f $SetupScriptPath 2>&1 | Write-Host
            if ($LASTEXITCODE -ne 0) { Write-Host "RLS isolation fixtures could not be written (exit $LASTEXITCODE)" -ForegroundColor Red }
        }

        $env:PGPASSWORD = $AppPassword
        $output = & $PsqlPath -h $DbHost -p $DbPort -U $AppUser -d $DbName -v ON_ERROR_STOP=1 -q -f $ScriptPath 2>&1 | ForEach-Object { "$_" }
        $psqlExitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousEap
        Remove-Item Env:\PGPASSWORD -ErrorAction SilentlyContinue
    }

    $cases = @()
    foreach ($line in $output) {
        if ($line -match '(PASS|FAIL) (.*)$') {
            $cases += [pscustomobject]@{ Passed = ($Matches[1] -eq 'PASS'); Name = $Matches[2] }
        }
    }

    $failed = @($cases | Where-Object { -not $_.Passed }).Count
    $escape = { param($text) [System.Security.SecurityElement]::Escape($text) }
    $junit = New-Object System.Text.StringBuilder
    [void]$junit.AppendLine('<?xml version="1.0" encoding="UTF-8"?>')
    [void]$junit.AppendLine("<testsuites><testsuite name=`"$SuiteName`" tests=`"$($cases.Count)`" failures=`"$failed`">")
    foreach ($case in $cases) {
        if ($case.Passed) {
            [void]$junit.AppendLine("<testcase classname=`"$SuiteName`" name=`"$(& $escape $case.Name)`"/>")
        }
        else {
            [void]$junit.AppendLine("<testcase classname=`"$SuiteName`" name=`"$(& $escape $case.Name)`"><failure message=`"$(& $escape $case.Name)`"/></testcase>")
        }
    }
    if ($psqlExitCode -ne 0) {
        [void]$junit.AppendLine("<testcase classname=`"$SuiteName`" name=`"isolation check script`"><failure message=`"$(& $escape (($output | Select-Object -Last 5) -join ' '))`"/></testcase>")
    }
    [void]$junit.AppendLine("</testsuite></testsuites>")
    Set-Content -Path $JUnitPath -Value $junit.ToString() -Encoding UTF8

    $output | Where-Object { $_ -match 'PASS|FAIL|ERROR|ОШИБКА' } | ForEach-Object { Write-Host $_ }
    Write-Host ("RLS isolation check: {0} passed, {1} failed (psql exit {2})" -f ($cases.Count - $failed), $failed, $psqlExitCode)

    if ($psqlExitCode -ne 0 -or $failed -gt 0 -or $cases.Count -eq 0) {
        return 1
    }
    return 0
}
