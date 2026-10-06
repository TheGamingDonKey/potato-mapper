param(
    [string]$QtPath = $env:QT_ROOT_DIR,
    [string]$BuildDirectory = (Join-Path $PSScriptRoot 'out\build'),
    [string]$CrtPath,
    [string]$OutputDirectory = (Join-Path $PSScriptRoot 'out\releases'),
    [string]$DestinationDirectory,
    [switch]$SkipZip
)
$ErrorActionPreference = 'Stop'
if (-not $QtPath) { $QtPath = $env:QTDIR }
if (-not $QtPath) { $QtPath = Join-Path $env:USERPROFILE '.cache\home-mapper-qt\6.10.3\msvc2022_64' }
if (-not $CrtPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Pass -CrtPath pointing to the Visual Studio x64 Microsoft.VC143.CRT redistributable directory.' }
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $redist = Join-Path $vs 'VC\Redist\MSVC'
    $crt = Get-ChildItem -LiteralPath $redist -Directory | Where-Object { $_.Name -match '^\d+\.\d+\.\d+$' } |
        Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
    if (-not $crt) { throw 'Visual C++ redistributable files not found.' }
    $CrtPath = Join-Path $crt.FullName 'x64\Microsoft.VC143.CRT'
}
$exe = Join-Path $BuildDirectory 'Release\PotatoMapperApp.exe'
$launcher = Join-Path $BuildDirectory 'Release\PotatoMapper.exe'
$refresh = Join-Path $BuildDirectory 'Release\PotatoLauncherRefresh.exe'
foreach ($required in $exe, $launcher, $refresh, (Join-Path $QtPath 'bin\windeployqt.exe'), (Join-Path $CrtPath 'vcruntime140.dll')) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Missing: $required" }
}
$version = (Get-Item -LiteralPath $exe).VersionInfo.FileVersion
if (-not $version) { throw 'Executable has no file version.' }
if ($version -notmatch '^\d+\.\d+\.\d+$') { throw 'Executable version must be X.Y.Z.' }
$name = "PotatoMapper-$version-windows-x64"
# Always stage in a fresh directory; never package the developer's app folder.
if ($DestinationDirectory) {
    $stage = [IO.Path]::GetFullPath($DestinationDirectory)
    if (Test-Path -LiteralPath $stage) { throw "Destination already exists: $stage. Use a new folder; existing app data will not be replaced." }
} else {
    $stageRoot = Join-Path $OutputDirectory ('stage-' + [guid]::NewGuid().ToString('N'))
    $stage = Join-Path $stageRoot $name
}
$stage = [IO.Path]::GetFullPath($stage)
$runtime = Join-Path $stage "versions\$version"
New-Item -ItemType Directory -Path $runtime -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $stage 'Projects') | Out-Null
New-Item -ItemType Directory -Path (Join-Path $runtime 'launcher') | Out-Null
Copy-Item -LiteralPath $launcher -Destination $stage
Copy-Item -LiteralPath $exe -Destination $runtime
# Protocol 1 only transfers versioned files. Retained older launchers therefore
# deliver both maintenance executables without any changes to the protocol.
Copy-Item -LiteralPath $launcher -Destination (Join-Path $runtime 'launcher\PotatoMapper.exe')
Copy-Item -LiteralPath $refresh -Destination $runtime
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'examples') -Destination $runtime -Recurse
[IO.File]::WriteAllText((Join-Path $runtime 'launcher\launcher-sha256.txt'), ((Get-FileHash -LiteralPath $launcher -Algorithm SHA256).Hash.ToLowerInvariant() + "`n"), [Text.Encoding]::ASCII)
& (Join-Path $QtPath 'bin\windeployqt.exe') --release --no-translations --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler --no-opengl-sw (Join-Path $runtime 'PotatoMapperApp.exe')
if ($LASTEXITCODE -ne 0) { throw 'Qt runtime deployment failed.' }
# ICU is supplied by supported Windows versions. Do not redistribute a copy
# taken from the build machine's Windows directory.
$systemIcu = Join-Path $runtime 'icuuc.dll'
if (Test-Path -LiteralPath $systemIcu) { Remove-Item -LiteralPath $systemIcu }
Get-ChildItem -LiteralPath $CrtPath -Filter '*.dll' | Copy-Item -Destination $runtime
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'LICENSE') -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\START-HERE.txt') -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\THIRD-PARTY.txt') -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\THIRD-PARTY.txt') -Destination $runtime
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\licenses') -Destination $runtime -Recurse
foreach ($module in 'qtbase', 'qtmultimedia', 'qtsvg') {
    Copy-Item -LiteralPath (Join-Path $QtPath "sbom\$module-6.10.3.spdx.json") -Destination (Join-Path $runtime 'licenses')
}
[IO.File]::WriteAllText((Join-Path $stage 'installation.txt'), "PotatoMapper/1`n", [Text.Encoding]::ASCII)
[IO.File]::WriteAllText((Join-Path $stage 'current.txt'), "$version`n", [Text.Encoding]::ASCII)
[IO.File]::WriteAllText((Join-Path $runtime 'runtime-version.txt'), "$version`n", [Text.Encoding]::ASCII)
# A retained copy needs its own digest inventory: old launchers do not retain
# the package-root update manifest when installing a version.
$runtimeInventory = @(Get-ChildItem -LiteralPath $runtime -File -Recurse | Sort-Object FullName | ForEach-Object {
    $relative = $_.FullName.Substring($runtime.Length + 1).Replace('\', '/')
    if ($relative -match "[`t`r`n]") { throw "Unsupported runtime filename: $relative" }
    (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() + "`t" + $relative
})
[IO.File]::WriteAllText((Join-Path $runtime 'runtime-files.sha256'), (($runtimeInventory -join "`n") + "`n"), [Text.UTF8Encoding]::new($false))
$files = @(Get-ChildItem -LiteralPath $runtime -File -Recurse | Sort-Object FullName | ForEach-Object {
    @{ path = $_.FullName.Substring($stage.Length + 1).Replace('\', '/'); sha256 = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant() }
})
$manifest = @{ format = 'PotatoMapperUpdate'; protocol = 1; version = $version; files = $files } | ConvertTo-Json -Depth 5
[IO.File]::WriteAllText((Join-Path $stage 'update-manifest.json'), $manifest, [Text.UTF8Encoding]::new($false))
Write-Output "Ready to run: $stage\PotatoMapper.exe"
if ($SkipZip) { return }
New-Item -ItemType Directory -Path $OutputDirectory -Force | Out-Null
$zip = Join-Path $OutputDirectory "$name.zip"
if (Test-Path -LiteralPath $zip) { throw "Package already exists: $zip. Choose another OutputDirectory." }
Compress-Archive -LiteralPath $stage -DestinationPath $zip -CompressionLevel Optimal
Write-Output "Ready: $zip"
Write-Output "SHA256: $((Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash)"
