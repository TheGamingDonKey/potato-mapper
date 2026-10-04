param([Parameter(Mandatory)][string]$PackageDirectory)
$ErrorActionPreference = 'Stop'
$PackageDirectory = (Resolve-Path -LiteralPath $PackageDirectory).Path
$version = (Get-Content -LiteralPath (Join-Path $PackageDirectory 'current.txt') -Raw).Trim()
if ($version -ne '0.3.0') { throw 'This fixture starts with a 0.3.0 package.' }
$fixture = Join-Path $PSScriptRoot ('..\out\launcher-check-' + [guid]::NewGuid().ToString('N'))
$fixture = [IO.Path]::GetFullPath($fixture)
New-Item -ItemType Directory -Path $fixture | Out-Null
Copy-Item -LiteralPath (Join-Path $PackageDirectory 'PotatoMapper.exe') -Destination $fixture
[IO.File]::WriteAllText((Join-Path $fixture 'installation.txt'), "PotatoMapper/1`n")
[IO.File]::WriteAllText((Join-Path $fixture 'current.txt'), "0.2.0`n")
$old = Join-Path $fixture 'versions\0.2.0'
New-Item -ItemType Directory -Path $old | Out-Null
[IO.File]::WriteAllText((Join-Path $old 'PotatoMapperApp.exe'), 'old-version-sentinel')
[IO.File]::WriteAllText((Join-Path $fixture 'My mapping.pmap'), '{"format":"PotatoMapper","version":1,"width":1280,"height":720,"surfaces":[]}')
$media = Join-Path $fixture 'media\clip.mp4'
New-Item -ItemType Directory -Path (Split-Path $media) | Out-Null
[IO.File]::WriteAllBytes($media, [byte[]](0, 1, 2, 253, 254, 255))
$personal = @('My mapping.pmap', 'media\clip.mp4')
$hashes = @{}
foreach ($file in $personal) { $hashes[$file] = (Get-FileHash -LiteralPath (Join-Path $fixture $file)).Hash }
function Assert-PersonalFiles {
    foreach ($file in $personal) {
        if ((Get-FileHash -LiteralPath (Join-Path $fixture $file)).Hash -ne $hashes[$file]) { throw "Personal fixture changed: $file" }
    }
}
function Run-Launcher([string[]]$Arguments, [int]$ExpectedCode = 0) {
    # Quote only fixture paths; use ProcessStartInfo.ArgumentList on PowerShell 7.
    $info = [Diagnostics.ProcessStartInfo]::new((Join-Path $fixture 'PotatoMapper.exe'))
    $info.UseShellExecute = $false
    foreach ($arg in $Arguments) { $info.ArgumentList.Add($arg) }
    $process = [Diagnostics.Process]::Start($info)
    if (-not $process.WaitForExit(30000)) { throw 'Launcher did not finish.' }
    if ($process.ExitCode -ne $ExpectedCode) { throw "Launcher exit $($process.ExitCode), expected $ExpectedCode" }
    Assert-PersonalFiles
}
function Stage-Release([string]$Name) {
    $session = Join-Path $fixture ".updates\$Name"
    $payload = Join-Path $session 'payload'
    New-Item -ItemType Directory -Path $payload -Force | Out-Null
    Copy-Item -LiteralPath (Join-Path $PackageDirectory 'versions') -Destination $payload -Recurse
    return $session
}
$bad = Join-Path $fixture '.updates\incomplete'
New-Item -ItemType Directory -Path $bad -Force | Out-Null
Run-Launcher @('--apply-update', $bad, '--version', $version, '--no-launch') 2
if ((Get-Content (Join-Path $fixture 'current.txt') -Raw).Trim() -ne '0.2.0') { throw 'Failed installation changed the selected version.' }
if (Test-Path -LiteralPath $bad) { throw 'Failed update staging was not cleaned up.' }
Write-Output 'PASS incomplete installation leaves old version selected and personal files unchanged'
$session = Stage-Release 'prepared'
Run-Launcher @('--apply-update', $session, '--version', $version, '--no-launch')
if ((Get-Content (Join-Path $fixture 'current.txt') -Raw).Trim() -ne $version -or (Get-Content (Join-Path $fixture 'previous.txt') -Raw).Trim() -ne '0.2.0') { throw 'Incorrect update pointers.' }
if (Test-Path -LiteralPath $session) { throw 'Consumed staging folder remains.' }
Run-Launcher @('--smoke')
Write-Output 'PASS actual packaged runtime installs and launches; old version and personal files retained'
Run-Launcher @('--rollback', '--no-launch')
if ((Get-Content (Join-Path $fixture 'current.txt') -Raw).Trim() -ne '0.2.0') { throw 'Rollback failed.' }
Write-Output 'PASS rollback selects previous app version without changing personal files'
$session = Stage-Release 'retry-after-rollback'
Run-Launcher @('--apply-update', $session, '--version', $version, '--no-launch')
if ((Get-Content (Join-Path $fixture 'current.txt') -Raw).Trim() -ne $version) { throw 'Reusing retained update failed.' }
Write-Output 'PASS updating again after rollback reuses the matching retained runtime'
$external = Join-Path $fixture 'personal-staging'
New-Item -ItemType Directory -Path $external | Out-Null
Run-Launcher @('--apply-update', $external, '--version', '0.4.0', '--no-launch') 2
if (-not (Test-Path -LiteralPath $external)) { throw 'Rejected external folder was deleted.' }
Write-Output 'PASS unrelated staging folder rejected and preserved'
$failing = Join-Path $fixture '.updates\cannot-start'
$runtime = Join-Path $failing 'payload\versions\0.4.0'
New-Item -ItemType Directory -Path $runtime -Force | Out-Null
[IO.File]::WriteAllText((Join-Path $runtime 'PotatoMapperApp.exe'), 'deliberately invalid executable')
[IO.File]::WriteAllText((Join-Path $runtime 'runtime-version.txt'), "0.4.0`n")
Run-Launcher @('--apply-update', $failing, '--version', '0.4.0', '--test-recovery', '--project', (Join-Path $fixture 'My mapping.pmap')) 2
if ((Get-Content (Join-Path $fixture 'current.txt') -Raw).Trim() -ne '0.3.0' -or (Get-Content (Join-Path $fixture 'previous.txt') -Raw).Trim() -ne '0.2.0') { throw 'Failed startup lost original version pointers.' }
if (Test-Path -LiteralPath $failing) { throw 'Failed startup staging was not cleaned up.' }
$deadline = [DateTime]::UtcNow.AddSeconds(10)
do {
    $editor = Get-Process PotatoMapperApp -ErrorAction SilentlyContinue | Where-Object { $_.Path -eq (Join-Path $fixture 'versions\0.3.0\PotatoMapperApp.exe') }
    if ($editor -and $editor.MainWindowTitle -like '*My mapping.pmap*') { break }
    Start-Sleep -Milliseconds 200
} while ([DateTime]::UtcNow -lt $deadline)
if (-not $editor -or $editor.MainWindowTitle -notlike '*My mapping.pmap*') { throw 'Previous working editor did not reopen the project.' }
$editor.CloseMainWindow() | Out-Null
if (-not $editor.WaitForExit(10000)) { throw 'Fixture editor did not close.' }
Write-Output 'PASS failed new-runtime startup restores both pointers and reopens saved mapping; staging cleaned'
Write-Output "Disposable installation: $fixture"
