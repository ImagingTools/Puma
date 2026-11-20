# SPDX-License-Identifier: LGPL-2.1-or-later OR GPL-2.0-or-later OR GPL-3.0-or-later OR LicenseRef-ImtCore-Commercial
<#
.SYNOPSIS
    Dot-sourced by the CI runners: puts groups of settings into the settings
    file of a test server for one run and restores the file afterwards.

.DESCRIPTION
    A server reads its settings file at start; a group missing from the file
    keeps its defaults. Set-ServerSettings replaces the named groups only, so
    the database and the ports in the file stay as they are.

    $Groups maps a group id to its parameters, each a map of the attributes the
    settings file holds for it:

        @{ Sessions = @{ AccessTokenLifetime = @{ Text = "20" } } }
#>

function Set-ServerSettings([string]$Path, [hashtable]$Groups) {
    $backup = $null
    $document = New-Object System.Xml.XmlDocument
    if (Test-Path $Path) {
        $backup = [System.IO.File]::ReadAllBytes($Path)
        $document.Load($Path)
    }
    else {
        $document.LoadXml('<?xml version="1.0" encoding="UTF-8"?><Acf><AcfHeader><VersionInfos><Version Id="0" Number="6435" Description="ACF"/></VersionInfos></AcfHeader><Parameters/></Acf>')
    }

    $parameters = $document.SelectSingleNode('/Acf/Parameters')
    if ($null -eq $parameters) {
        $parameters = $document.DocumentElement.AppendChild($document.CreateElement('Parameters'))
    }

    foreach ($groupId in $Groups.Keys) {
        $existing = $parameters.SelectSingleNode("Parameter[@Id='$groupId']")
        if ($null -ne $existing) {
            [void]$parameters.RemoveChild($existing)
        }

        $group = $document.CreateElement('Parameter')
        $group.SetAttribute('Id', $groupId)
        $groupParameters = $group.AppendChild($document.CreateElement('Parameters'))
        foreach ($parameterId in $Groups[$groupId].Keys) {
            $parameter = $document.CreateElement('Parameter')
            $parameter.SetAttribute('Id', $parameterId)
            $attributes = $Groups[$groupId][$parameterId]
            foreach ($name in $attributes.Keys) {
                $parameter.SetAttribute($name, [string]$attributes[$name])
            }
            [void]$groupParameters.AppendChild($parameter)
        }
        [void]$parameters.AppendChild($group)
    }

    $directory = Split-Path -Parent $Path
    if (-not (Test-Path $directory)) {
        New-Item -ItemType Directory -Force $directory | Out-Null
    }
    $document.Save($Path)
    Write-Host "Settings $($Groups.Keys -join ', ') written to $Path"

    # The original bytes, or $null for a file that did not exist
    return ,$backup
}

function Restore-ServerSettings([string]$Path, $Backup) {
    if ($null -eq $Backup) {
        Remove-Item -Path $Path -Force -ErrorAction SilentlyContinue
    }
    else {
        [System.IO.File]::WriteAllBytes($Path, $Backup)
    }
    Write-Host "Settings restored: $Path"
}

# Lifetimes of the short sessions the token expiry suites run with
function Get-ShortSessionSettings([int]$JwtLifetime, [int]$SessionLifetime) {
    return @{
        Sessions = @{
            AccessTokenLifetime = @{ Text = "$JwtLifetime" }
            SessionLifetime = @{ Text = "$SessionLifetime" }
        }
    }
}
