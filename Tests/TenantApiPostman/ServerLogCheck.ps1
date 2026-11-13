# SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
<#
.SYNOPSIS
    Dot-sourced by the CI runners of this folder: fails a run whose server
    logged a database error.

.DESCRIPTION
    Every answer of the API can be right while the server loses data behind
    it: on 2026-11-04 no notification was stored on a fresh database (its
    table did not exist yet) and every suite was green, the only trace was
    941 failed statements in the server log.

    The markers are the ones the ImtCore database engine and the Qt SQL
    drivers write in English. The localized text of PostgreSQL is not
    matched: its NOTICEs ("... already exists, skipping") are routine.
#>

$script:ServerLogErrorPattern = 'sqlError Occured|Database query failed|QPSQL: |Unable to (execute statement|prepare statement|fetch row|create query|bind)'

# True when none of the logs has a database error. A missing log counts as a
# failure: a check that finds nothing to read must not pass.
function Test-ServerLogs([string[]]$Paths) {
    Write-Host "`n=== Checking the server logs for database errors ===" -ForegroundColor Cyan
    $isClean = $true
    foreach ($path in $Paths) {
        if (-not (Test-Path $path)) {
            Write-Host "Server log not found: $path" -ForegroundColor Red
            $isClean = $false
            continue
        }
        $hits = @(Select-String -Path $path -Pattern $script:ServerLogErrorPattern)
        if ($hits.Count -gt 0) {
            Write-Host "$($hits.Count) database error line(s) in $path" -ForegroundColor Red
            $hits | Select-Object -First 20 | ForEach-Object { Write-Host "  $($_.LineNumber): $($_.Line)" }
            $isClean = $false
        }
        else {
            Write-Host "No database errors in $path"
        }
    }
    return $isClean
}
