param([long]$ReserveBytes=0)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$bytes=(Get-ChildItem -LiteralPath $root -File -Recurse -Force | Measure-Object Length -Sum).Sum
if($bytes+$ReserveBytes -gt 1500000000){throw "Project storage limit: $bytes bytes used + $ReserveBytes reserved exceeds 1,500,000,000. Preserve projects/media; remove only verified replaceable outputs before proceeding."}
Write-Output "Storage: $bytes bytes used; $ReserveBytes bytes reserved; cap 1500000000."
