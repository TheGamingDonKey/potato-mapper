param([Parameter(Mandatory)][string]$Archive)
$ErrorActionPreference='Stop'
$zip=Get-Item -LiteralPath $Archive
if($zip.Name -notmatch '^PotatoMapper-(\d+\.\d+\.\d+)-windows-x64\.zip$'){throw 'Select a versioned Potato Mapper Windows ZIP.'}
$version=$Matches[1]
$output=[IO.Path]::ChangeExtension($zip.FullName,'.exe')
if(Test-Path -LiteralPath $output){throw "Download already exists: $output"}
$csc=Join-Path $env:SystemRoot 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$metadata=Join-Path $zip.DirectoryName ("version-"+[guid]::NewGuid().ToString('N')+".cs")
try{
    [IO.File]::WriteAllText($metadata, "[assembly: System.Reflection.AssemblyVersion(`"$version.0`")]")
    $icon=Join-Path $PSScriptRoot '../assets/potato-mapper.ico'
    & $csc /nologo /target:winexe /platform:x64 /optimize+ /codepage:65001 "/win32icon:$icon" "/out:$output" "/resource:$($zip.FullName),PortablePackage.zip" /reference:System.IO.Compression.dll /reference:System.IO.Compression.FileSystem.dll /reference:System.Windows.Forms.dll /reference:System.Drawing.dll $metadata (Join-Path $PSScriptRoot 'PortableDownload.cs')
    if($LASTEXITCODE -ne 0){throw 'Could not build the self-extracting Windows download.'}
}finally{
    if(Test-Path -LiteralPath $metadata){Remove-Item -LiteralPath $metadata}
}
Write-Output "Portable download: $output"
