param(
    [Parameter(Mandatory)][string]$RuntimeDirectory,
    [string]$EditorExecutable,
    [string]$DumpbinPath
)
$ErrorActionPreference = 'Stop'
$runtime = (Resolve-Path -LiteralPath $RuntimeDirectory).Path.TrimEnd('\','/')
if (-not $EditorExecutable) { $EditorExecutable = Join-Path $runtime 'PotatoMapperApp.exe' }
$editor = (Resolve-Path -LiteralPath $EditorExecutable).Path
if (-not $DumpbinPath) {
    $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
    if (-not (Test-Path -LiteralPath $vswhere)) { throw 'Pass -DumpbinPath from the Visual C++ x64 build tools.' }
    $vs = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
    $DumpbinPath = Get-ChildItem -Path (Join-Path $vs 'VC\Tools\MSVC\*\bin\Hostx64\x64\dumpbin.exe') |
        Sort-Object FullName -Descending | Select-Object -First 1 -ExpandProperty FullName
}
if (-not $DumpbinPath -or -not (Test-Path -LiteralPath $DumpbinPath)) { throw 'dumpbin.exe is required for architecture and import validation.' }
$all = @(Get-ChildItem -LiteralPath $runtime -Recurse -Force)
if ($all | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }) { throw 'Runtime must not contain symbolic links or junctions.' }
$entries = [Collections.Generic.List[object]]::new()
$entries.Add([pscustomobject]@{ Source=$editor; Destination='PotatoMapperApp.exe' })
foreach ($file in $all | Where-Object { -not $_.PSIsContainer -and $_.Extension -ieq '.dll' }) {
    $relative = $file.FullName.Substring($runtime.Length + 1)
    # Only known Qt runtime directories; do not collect projects, saved media,
    # launcher maintenance tools, or an entire portable installation.
    if ($relative -notmatch '^[^\\/]+\.dll$' -and $relative -notmatch '^(generic|iconengines|imageformats|multimedia|networkinformation|platforms|styles|tls)\\[^\\]+\.dll$') { continue }
    if ($file.Name -ieq 'icuuc.dll') { throw 'Remove the Windows-owned icuuc.dll from deployment; Windows supplies this system component.' }
    $entries.Add([pscustomobject]@{ Source=$file.FullName; Destination=$relative })
}
$destinations = @{}
foreach ($entry in $entries) { $destinations[$entry.Destination] = $entry.Source }
foreach ($required in 'PotatoMapperApp.exe','Qt6Core.dll','Qt6Gui.dll','Qt6Widgets.dll','Qt6OpenGL.dll','Qt6OpenGLWidgets.dll','Qt6Network.dll','Qt6Multimedia.dll','platforms\qwindows.dll','multimedia\ffmpegmediaplugin.dll','vcruntime140.dll','vcruntime140_1.dll','msvcp140.dll') {
    if (-not $destinations.ContainsKey($required)) { throw "Missing deployed runtime dependency: $required" }
}
foreach ($pattern in 'avcodec-*.dll','avformat-*.dll','avutil-*.dll','swresample-*.dll','swscale-*.dll') {
    if (-not ($destinations.Keys | Where-Object { $_ -like $pattern })) { throw "Missing deployed FFmpeg dependency: $pattern" }
}
$systemDirectory = Join-Path $env:SystemRoot 'System32'
foreach ($entry in $entries) {
    $headers = & $DumpbinPath /nologo /headers $entry.Source 2>&1
    if ($LASTEXITCODE -ne 0 -or -not ($headers -match '8664 machine \(x64\)')) { throw "Not a valid Windows x64 binary: $($entry.Source)" }
    $imports = & $DumpbinPath /nologo /dependents $entry.Source 2>&1
    if ($LASTEXITCODE -ne 0) { throw "Could not read imports: $($entry.Source)" }
    foreach ($line in $imports) {
        if ($line -notmatch '^\s+([\w.-]+\.dll)\s*$') { continue }
        $dependency = $Matches[1]
        $parent = Split-Path $entry.Destination -Parent
        $local = if ($parent) { Join-Path $parent $dependency } else { $dependency }
        if ($destinations.ContainsKey($dependency) -or $destinations.ContainsKey($local)) { continue }
        # CRT files must come from deployment even when this development PC has
        # a redist installed. The UCRT and API-set contracts are OS components.
        if ($dependency -match '^(vcruntime|msvcp|concrt|vcomp)\d.*\.dll$') { throw "Missing app-local VC runtime $dependency imported by $($entry.Destination)" }
        if ($dependency -match '^(api-ms-win-|ext-ms-win-)') { continue }
        if (Test-Path -LiteralPath (Join-Path $systemDirectory $dependency)) { continue }
        throw "Unresolved import $dependency in $($entry.Destination)"
    }
}
$bytes = ($entries | ForEach-Object { (Get-Item -LiteralPath $_.Source).Length } | Measure-Object -Sum).Sum
[pscustomobject]@{
    RuntimeDirectory=$runtime; EditorExecutable=$editor; BinaryCount=$entries.Count
    PayloadBytes=[long]$bytes; Files=$entries.ToArray(); DumpbinPath=$DumpbinPath
    Validation='x64 PE headers, required Qt/FFmpeg/CRT files, and static/delay DLL imports on this host. Dynamic loading and clean-machine behavior still require installed-package testing.'
}
