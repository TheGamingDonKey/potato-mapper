param(
    [ValidateRange(0,1800000000)][long]$ReserveBytes=0,
    [string]$DesignWorkspace=(Join-Path $env:USERPROFILE 'OneDrive\Music\Documents\ChatGPT\Potato mapper'),
    [switch]$PassThru
)
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$cap=1800000000L # Explicitly authorized by Shane on 2026-11-01.
$countedRoots=[Collections.Generic.List[string]]::new()
$countedRoots.Add($root.TrimEnd('\','/'))
$bytes=(Get-ChildItem -LiteralPath $root -File -Recurse -Force | Measure-Object Length -Sum).Sum
if($DesignWorkspace -and (Test-Path -LiteralPath $DesignWorkspace -PathType Container)){
    $designRoot=[IO.Path]::GetFullPath($DesignWorkspace).TrimEnd('\','/')
    if(!$designRoot.Equals($root,[StringComparison]::OrdinalIgnoreCase) -and !$designRoot.StartsWith($root+'\',[StringComparison]::OrdinalIgnoreCase)){
        if($root.StartsWith($designRoot+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Design workspace must not contain the source checkout; choose its actual design folder.'}
        $bytes+=(Get-ChildItem -LiteralPath $designRoot -File -Recurse -Force | Measure-Object Length -Sum).Sum
        $countedRoots.Add($designRoot)
    }
}
$conceptManifest=Join-Path $root 'out\ui-redesign\concepts.json'
if(Test-Path -LiteralPath $conceptManifest){
    $seen=[Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
    foreach($preview in (Get-Content -LiteralPath $conceptManifest -Raw | ConvertFrom-Json).Concepts){
        $previewPath=[IO.Path]::GetFullPath($preview)
        $alreadyCounted=$false
        foreach($countedRoot in $countedRoots){if($previewPath.StartsWith($countedRoot+'\',[StringComparison]::OrdinalIgnoreCase)){$alreadyCounted=$true;break}}
        if(!$alreadyCounted -and $seen.Add($previewPath) -and (Test-Path -LiteralPath $previewPath -PathType Leaf)){$bytes+=(Get-Item -LiteralPath $previewPath).Length}
    }
}
if($bytes+$ReserveBytes -gt $cap){throw "Project storage limit: $bytes bytes used + $ReserveBytes reserved exceeds 1,800,000,000. Preserve projects/media; remove only verified replaceable outputs before proceeding."}
if($PassThru){[pscustomobject]@{Bytes=[long]$bytes;ReserveBytes=$ReserveBytes;CapBytes=$cap;Roots=$countedRoots.ToArray()}}
else{Write-Output "Storage: $bytes bytes used; $ReserveBytes bytes reserved; cap $cap."}
