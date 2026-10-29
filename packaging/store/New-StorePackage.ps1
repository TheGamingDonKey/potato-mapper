param(
    [Parameter(Mandatory)][string]$RuntimeDirectory,
    [string]$EditorExecutable,
    [Parameter(Mandatory)][ValidatePattern('^[A-Za-z0-9][A-Za-z0-9.-]{2,49}$')][string]$PackageName,
    [Parameter(Mandatory)][ValidateNotNullOrEmpty()][string]$Publisher,
    [Parameter(Mandatory)][ValidateNotNullOrEmpty()][string]$PublisherDisplayName,
    [Parameter(Mandatory)][ValidateNotNullOrEmpty()][string]$StoreDisplayName,
    [Parameter(Mandatory)][ValidatePattern('^[1-9][0-9]*\.[0-9]+\.[0-9]+\.0$')][string]$PackageVersion,
    [string]$Description = 'Projection mapping for images, videos and animated scenes.',
    [string]$MaxVersionTested = '10.0.26100.0',
    [string]$OutputDirectory,
    [string]$MakeAppxPath,
    [string]$DumpbinPath,
    [long]$StorageLimitBytes = 1500000000,
    [switch]$LocalValidationIdentity,
    [switch]$PrepareOnly
)
$ErrorActionPreference = 'Stop'
$repo = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..')).TrimEnd('\','/')
foreach ($value in $PackageName,$Publisher,$PublisherDisplayName,$StoreDisplayName) {
    if ($value -match '(?i)(placeholder|REPLACE_ME|YOUR[_ -]|<|>)') { throw 'Supply real Partner Center identity values; template identity is not a Store candidate.' }
}
if ($LocalValidationIdentity) {
    Write-Warning 'Synthetic identity: this output is only a local packaging validation artifact and is not a Microsoft Store candidate.'
}
$version = [version]$PackageVersion
if (@($version.Major,$version.Minor,$version.Build,$version.Revision | Where-Object { $_ -gt 65535 }).Count) { throw 'Package version fields must be within 0..65535.' }
if ($MaxVersionTested -notmatch '^10\.0\.[0-9]+\.[0-9]+$') { throw 'MaxVersionTested must be a four-part Windows version.' }
if ([version]$MaxVersionTested -lt [version]'10.0.19045.0') { throw 'MaxVersionTested cannot be less than MinVersion.' }
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $repo ('out\store\' + $PackageVersion + '-' + [guid]::NewGuid().ToString('N')) }
$output = [IO.Path]::GetFullPath($OutputDirectory).TrimEnd('\','/')
if (-not $output.StartsWith($repo + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Store output must be inside this repository so the shared storage limit covers it.' }
if (Test-Path -LiteralPath $output) { throw "Output already exists; use a new destination: $output" }
if ($StorageLimitBytes -gt 1500000000 -or $StorageLimitBytes -le 0) { throw 'Storage limit must be positive and cannot exceed 1.5 GB.' }
if (-not $MakeAppxPath) {
    $sdk = Join-Path ${env:ProgramFiles(x86)} 'Windows Kits\10\bin'
    $MakeAppxPath = Get-ChildItem -LiteralPath $sdk -Directory | Where-Object { $_.Name -match '^10\.0\.\d+\.0$' } |
        Sort-Object { [version]$_.Name } -Descending | ForEach-Object { Join-Path $_.FullName 'x64\makeappx.exe' } |
        Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
}
if (-not $MakeAppxPath -or -not (Test-Path -LiteralPath $MakeAppxPath)) { throw 'Windows SDK MakeAppx.exe was not found. Supply -MakeAppxPath; this script does not install tools.' }
$runtime = & (Join-Path $PSScriptRoot 'Test-StoreRuntime.ps1') -RuntimeDirectory $RuntimeDirectory -EditorExecutable $EditorExecutable -DumpbinPath $DumpbinPath
$files = [Collections.Generic.List[object]]::new()
foreach ($entry in $runtime.Files) { $files.Add($entry) }
# Only repository-owned examples/notices. Personal mappings and media from the
# deployment directory never enter this file mapping.
foreach ($relative in @('LICENSE','packaging\THIRD-PARTY.txt')) {
    $source = Join-Path $repo $relative
    if (-not (Test-Path -LiteralPath $source -PathType Leaf)) { throw "Missing notice: $source" }
    $files.Add([pscustomobject]@{Source=$source; Destination=(Split-Path $relative -Leaf)})
}
foreach ($folder in 'examples','packaging\licenses') {
    foreach ($file in Get-ChildItem -LiteralPath (Join-Path $repo $folder) -File -Recurse) {
        if ($folder -eq 'examples' -and $file.Extension -notin '.pmap','.hmap') { throw "Only built-in mapping documents may be bundled from examples; review unexpected file: $($file.FullName)" }
        $destination = if ($folder -eq 'examples') { $file.FullName.Substring($repo.Length+1) } else { 'licenses\' + $file.FullName.Substring((Join-Path $repo $folder).Length+1) }
        $files.Add([pscustomobject]@{Source=$file.FullName; Destination=$destination})
    }
}
foreach ($module in 'qtbase','qtmultimedia','qtsvg') {
    $sbom = Join-Path $runtime.RuntimeDirectory "licenses\$module-6.10.3.spdx.json"
    if (-not (Test-Path -LiteralPath $sbom)) { throw "Missing deployed dependency provenance: $sbom" }
    $files.Add([pscustomobject]@{Source=$sbom; Destination=('licenses\' + (Split-Path $sbom -Leaf))})
}
$payloadBytes = [long](($files | ForEach-Object { (Get-Item -LiteralPath $_.Source).Length } | Measure-Object -Sum).Sum)
function Get-WorkspaceBytes {
    [long]( (Get-ChildItem -LiteralPath $repo -Recurse -File -Force | Measure-Object -Property Length -Sum).Sum )
}
$before = Get-WorkspaceBytes
# Reserve both a worst-case uncompressed package and SDK temporary headroom.
$projected = $before + 2 * $payloadBytes + 64MB
if ($projected -gt $StorageLimitBytes) { throw "Storage budget blocked: current $before bytes; projected $projected; cap $StorageLimitBytes. No package files have been created." }
New-Item -ItemType Directory -Path $output -Force | Out-Null
& (Join-Path $PSScriptRoot 'New-StoreAssets.ps1') -OutputDirectory (Join-Path $output 'Assets') | Write-Host
$template = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'AppxManifest.xml.in'))
$tokens = @{
    PACKAGE_NAME=$PackageName; PUBLISHER=$Publisher; PACKAGE_VERSION=$PackageVersion
    DISPLAY_NAME=$StoreDisplayName; PUBLISHER_DISPLAY_NAME=$PublisherDisplayName
    DESCRIPTION=$Description; MAX_VERSION_TESTED=$MaxVersionTested
}
foreach ($key in $tokens.Keys) { $template = $template.Replace('@'+$key+'@', [Security.SecurityElement]::Escape([string]$tokens[$key])) }
$manifestPath = Join-Path $output 'AppxManifest.xml'
[IO.File]::WriteAllText($manifestPath, $template, [Text.UTF8Encoding]::new($false))
$null = [xml]$template
$files.Add([pscustomobject]@{Source=$manifestPath; Destination='AppxManifest.xml'})
Add-Type -AssemblyName System.Drawing
foreach ($item in @(@('StoreLogo.png',50),@('Square44x44Logo.png',44),@('Square150x150Logo.png',150))) {
    $source = Join-Path $output ('Assets\'+$item[0])
    $image = [Drawing.Image]::FromFile($source)
    try { if ($image.Width -ne $item[1] -or $image.Height -ne $item[1] -or $image.RawFormat.Guid -ne [Drawing.Imaging.ImageFormat]::Png.Guid) { throw "Invalid Store logo: $source" } } finally { $image.Dispose() }
    $files.Add([pscustomobject]@{Source=$source; Destination=('Assets\'+$item[0])})
}
$mapping = [Collections.Generic.List[string]]::new()
$mapping.Add('[Files]')
$seen = @{}
foreach ($file in $files) {
    if ($file.Source -match '["\r\n]' -or $file.Destination -match '["\r\n]' -or $seen.ContainsKey($file.Destination)) { throw "Unsafe or duplicate package path: $($file.Destination)" }
    if ($file.Destination -match '^(Projects|versions|launcher)([\\/]|$)' -or ($file.Destination -match '\.exe$' -and $file.Destination -ne 'PotatoMapperApp.exe')) { throw "Portable app maintenance or user storage cannot enter MSIX: $($file.Destination)" }
    $seen[$file.Destination]=$true
    $mapping.Add('"' + $file.Source + '" "' + $file.Destination + '"')
}
$mappingPath = Join-Path $output 'package.map.txt'
[IO.File]::WriteAllLines($mappingPath,$mapping,[Text.UTF8Encoding]::new($false))
$package = Join-Path $output "PotatoMapper-$PackageVersion-x64.msix"
$report = [ordered]@{
    Status='prepared-not-packed'; Identity=$tokens; EditorExecutable=$runtime.EditorExecutable
    Purpose=$(if ($LocalValidationIdentity) { 'local-packaging-validation-only' } else { 'owner-supplied-identity-pending-installed-tests-and-submission' })
    SyntheticIdentity=[bool]$LocalValidationIdentity; PublicStoreIdentityVerified=$false
    BinaryCount=$runtime.BinaryCount; MappedFiles=$files.Count; PayloadBytes=$payloadBytes
    WorkspaceBytesBefore=$before; ReservedPeakBytes=$projected; StorageLimitBytes=$StorageLimitBytes
    MakeAppx=$MakeAppxPath; Package=$package; Signed=$false; Certification='not run'; InstalledTest='not run'
}
if (-not $PrepareOnly) {
    & $MakeAppxPath pack /f $mappingPath /p $package /no
    if ($LASTEXITCODE -ne 0) { throw "MakeAppx validation or packaging failed ($LASTEXITCODE). Diagnostic files remain in $output" }
    $report.Status=if ($LocalValidationIdentity) { 'unsigned-local-validation-msix-created-not-store-candidate' } else { 'unsigned-msix-created' }
    $report.PackageBytes=(Get-Item -LiteralPath $package).Length
    $report.Sha256=(Get-FileHash -LiteralPath $package -Algorithm SHA256).Hash.ToLowerInvariant()
}
$report.WorkspaceBytesAfter=Get-WorkspaceBytes
[IO.File]::WriteAllText((Join-Path $output 'store-package-report.json'),($report | ConvertTo-Json -Depth 5),[Text.UTF8Encoding]::new($false))
if ($report.WorkspaceBytesAfter -gt $StorageLimitBytes) { throw "Storage cap exceeded ($($report.WorkspaceBytesAfter) bytes). No files were automatically deleted. Review $output before continuing." }
[pscustomobject]$report
