param([long]$ReserveBytes=0)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$bytes=(Get-ChildItem -LiteralPath $root -File -Recurse -Force | Measure-Object Length -Sum).Sum
$conceptManifest=Join-Path $root 'out\ui-redesign\concepts.json'
if(Test-Path -LiteralPath $conceptManifest){
    $seen=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach($preview in (Get-Content -LiteralPath $conceptManifest -Raw | ConvertFrom-Json).Concepts){
        $previewPath=[IO.Path]::GetFullPath($preview)
        if(!$previewPath.StartsWith($root+'\',[StringComparison]::OrdinalIgnoreCase) -and $seen.Add($previewPath) -and (Test-Path -LiteralPath $previewPath -PathType Leaf)){$bytes+=(Get-Item -LiteralPath $previewPath).Length}
    }
}
if($bytes+$ReserveBytes -gt 1500000000){throw "Project storage limit: $bytes bytes used + $ReserveBytes reserved exceeds 1,500,000,000. Preserve projects/media; remove only verified replaceable outputs before proceeding."}
Write-Output "Storage: $bytes bytes used; $ReserveBytes bytes reserved; cap 1500000000."
