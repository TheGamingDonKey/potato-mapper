param(
    [string]$QtPath = $env:QT_ROOT_DIR,
    [string]$BuildDirectory = (Join-Path $PSScriptRoot 'out\build'),
    [string]$CrtPath,
    [string]$OutputDirectory = (Join-Path $PSScriptRoot 'out\releases')
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
$exe = Join-Path $BuildDirectory 'Release\PotatoMapper.exe'
foreach ($required in $exe, (Join-Path $QtPath 'bin\windeployqt.exe'), (Join-Path $CrtPath 'vcruntime140.dll')) {
    if (-not (Test-Path -LiteralPath $required)) { throw "Missing: $required" }
}
$version = (Get-Item -LiteralPath $exe).VersionInfo.FileVersion
if (-not $version) { throw 'Executable has no file version.' }
$name = "PotatoMapper-$version-windows-x64"
# Always stage in a fresh directory; never package the developer's app folder.
$stageRoot = Join-Path $OutputDirectory ('stage-' + [guid]::NewGuid().ToString('N'))
$stage = Join-Path $stageRoot $name
New-Item -ItemType Directory -Path $stage -Force | Out-Null
Copy-Item -LiteralPath $exe -Destination $stage
& (Join-Path $QtPath 'bin\windeployqt.exe') --release --no-translations --no-compiler-runtime --no-system-d3d-compiler --no-system-dxc-compiler --no-opengl-sw (Join-Path $stage 'PotatoMapper.exe')
if ($LASTEXITCODE -ne 0) { throw 'Qt runtime deployment failed.' }
# ICU is supplied by supported Windows versions. Do not redistribute a copy
# taken from the build machine's Windows directory.
$systemIcu = Join-Path $stage 'icuuc.dll'
if (Test-Path -LiteralPath $systemIcu) { Remove-Item -LiteralPath $systemIcu }
Get-ChildItem -LiteralPath $CrtPath -Filter '*.dll' | Copy-Item -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'LICENSE') -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\START-HERE.txt') -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\THIRD-PARTY.txt') -Destination $stage
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'packaging\licenses') -Destination $stage -Recurse
foreach ($module in 'qtbase', 'qtmultimedia', 'qtsvg') {
    Copy-Item -LiteralPath (Join-Path $QtPath "sbom\$module-6.10.3.spdx.json") -Destination (Join-Path $stage 'licenses')
}
$zip = Join-Path $OutputDirectory "$name.zip"
if (Test-Path -LiteralPath $zip) { throw "Package already exists: $zip. Choose another OutputDirectory." }
Compress-Archive -LiteralPath $stage -DestinationPath $zip -CompressionLevel Optimal
Write-Output "Ready: $zip"
Write-Output "SHA256: $((Get-FileHash -LiteralPath $zip -Algorithm SHA256).Hash)"
