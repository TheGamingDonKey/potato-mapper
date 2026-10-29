param(
    [string]$SourceImage = (Join-Path $PSScriptRoot '..\..\assets\potato-mapper.png'),
    [Parameter(Mandatory)][string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
$source = [Drawing.Image]::FromFile([IO.Path]::GetFullPath($SourceImage))
try {
    if (Test-Path -LiteralPath $OutputDirectory) { throw "Asset destination already exists: $OutputDirectory" }
    New-Item -ItemType Directory -Path $OutputDirectory | Out-Null
    foreach ($item in @(@('StoreLogo.png',50), @('Square44x44Logo.png',44), @('Square150x150Logo.png',150))) {
        $size = [int]$item[1]
        $bitmap = [Drawing.Bitmap]::new($size, $size, [Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $graphics = [Drawing.Graphics]::FromImage($bitmap)
        try {
            $graphics.Clear([Drawing.Color]::Transparent)
            $graphics.InterpolationMode = [Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $graphics.PixelOffsetMode = [Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            # Preserve aspect ratio and keep the complete existing artwork.
            $scale = [Math]::Min($size / $source.Width, $size / $source.Height)
            $width = [int][Math]::Round($source.Width * $scale)
            $height = [int][Math]::Round($source.Height * $scale)
            $graphics.DrawImage($source, [Drawing.Rectangle]::new([int](($size-$width)/2), [int](($size-$height)/2), $width, $height))
            $bitmap.Save((Join-Path $OutputDirectory $item[0]), [Drawing.Imaging.ImageFormat]::Png)
        } finally { $graphics.Dispose(); $bitmap.Dispose() }
    }
} finally { $source.Dispose() }
Write-Output "Generated Store logos: $OutputDirectory"
