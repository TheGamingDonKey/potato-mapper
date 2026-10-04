$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.IO.Compression.FileSystem
$archive = [IO.Compression.ZipFile]::OpenRead($env:POTATO_UPDATE_ZIP)
try {
    if ($archive.Entries.Count -gt 20000) { throw 'Too many files in update.' }
    $prefix = $null
    $total = [long]0
    foreach ($entry in $archive.Entries) {
        $name = $entry.FullName.Replace('\', '/')
        $parts = $name.TrimEnd('/').Split('/')
        if ($parts.Count -lt 1 -or $name.StartsWith('/') -or $name.Contains(':') -or
            @($parts | Where-Object { $_ -eq '..' -or $_ -eq '.' -or $_ -eq '' }).Count -gt 0) {
            throw 'Invalid path in update ZIP.'
        }
        if (($entry.ExternalAttributes -shr 16 -band 0xF000) -eq 0xA000) { throw 'Links are not allowed in update ZIP.' }
        if (-not $prefix) { $prefix = $parts[0] }
        if ($parts[0] -ne $prefix) { throw 'The update must contain one application folder.' }
        $total += $entry.Length
        if ($total -gt 2GB) { throw 'Update expands beyond the supported size.' }
    }
    $destination = [IO.Path]::GetFullPath($env:POTATO_UPDATE_STAGE)
    [IO.Directory]::CreateDirectory($destination) | Out-Null
    foreach ($entry in $archive.Entries) {
        $name = $entry.FullName.Replace('\', '/')
        $separator = $name.IndexOf('/')
        if ($separator -lt 0 -or $separator -eq $name.Length - 1) { continue }
        $relative = $name.Substring($separator + 1)
        $target = [IO.Path]::GetFullPath([IO.Path]::Combine($destination, $relative))
        if (-not $target.StartsWith($destination.TrimEnd('\') + '\', [StringComparison]::OrdinalIgnoreCase)) { throw 'Update path escapes staging.' }
        if ($name.EndsWith('/')) { [IO.Directory]::CreateDirectory($target) | Out-Null; continue }
        [IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($target)) | Out-Null
        $inputStream = $entry.Open()
        try {
            $outputStream = [IO.File]::Open($target, [IO.FileMode]::CreateNew)
            try { $inputStream.CopyTo($outputStream) } finally { $outputStream.Dispose() }
        } finally { $inputStream.Dispose() }
    }
} finally { $archive.Dispose() }
