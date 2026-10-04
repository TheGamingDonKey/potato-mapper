param(
    [string]$QtPath = $env:QT_ROOT_DIR,
    [string]$CMakePath,
    [string]$BuildDirectory = (Join-Path $PSScriptRoot 'out\build'),
    [string]$AppDirectory
)
$ErrorActionPreference = 'Stop'
if (-not $QtPath) { $QtPath = $env:QTDIR }
if (-not $QtPath) { $QtPath = Join-Path $env:USERPROFILE '.cache\home-mapper-qt\6.10.3\msvc2022_64' }
if (-not (Test-Path -LiteralPath (Join-Path $QtPath 'lib\cmake\Qt6\Qt6Config.cmake'))) {
    throw 'Qt not found. Pass -QtPath pointing to your Qt MSVC 2022 x64 installation (with Qt Multimedia).'
}
if (-not $CMakePath) {
    $cmakeCommand = Get-Command cmake -ErrorAction SilentlyContinue
    if ($cmakeCommand) { $CMakePath = $cmakeCommand.Source }
    else { $CMakePath = Join-Path $env:USERPROFILE '.cache\home-mapper-tools\Scripts\cmake.exe' }
}
if (-not (Test-Path -LiteralPath $CMakePath)) { throw 'CMake not found. Install CMake or pass -CMakePath.' }
$mapperCmake = $CMakePath
$mapperQt = $QtPath
$mapperBuild = $BuildDirectory
& $mapperCmake -S $PSScriptRoot -B $mapperBuild -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$mapperQt"
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed.' }
& $mapperCmake --build $mapperBuild --config Release --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Compilation failed.' }
if (-not $AppDirectory) { $AppDirectory = Join-Path $PSScriptRoot ('out\dev-app-' + (Get-Date -Format 'yyyyMMdd-HHmmss')) }
& (Join-Path $PSScriptRoot 'package.ps1') -QtPath $mapperQt -BuildDirectory $mapperBuild -DestinationDirectory $AppDirectory -SkipZip
