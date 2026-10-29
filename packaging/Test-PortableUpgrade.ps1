param(
    [string]$PackageDirectory = (Join-Path $PSScriptRoot '..\out\releases\PotatoMapper-0.9.0-windows-x64'),
    [string]$Archive = (Join-Path $PSScriptRoot '..\out\releases\PotatoMapper-0.9.0-windows-x64.zip'),
    [string]$OldPackageDirectory = (Join-Path $PSScriptRoot '..\out\halloween-check\final\PotatoMapper-0.8.8-windows-x64'),
    [string]$CheckExecutable = (Join-Path $PSScriptRoot '..\out\build-desktop\Release\PotatoUpdateChecks.exe'),
    [string]$QtPath = (Join-Path $env:USERPROFILE '.cache\home-mapper-qt\6.10.3\msvc2022_64')
)
$ErrorActionPreference='Stop'
$repo=(Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path
$package=(Resolve-Path -LiteralPath $PackageDirectory).Path
$archive=(Resolve-Path -LiteralPath $Archive).Path
$oldPackage=(Resolve-Path -LiteralPath $OldPackageDirectory).Path
$checker=(Resolve-Path -LiteralPath $CheckExecutable).Path
$oldRuntime=Join-Path $oldPackage 'versions\0.8.8'
foreach($path in (Join-Path $oldRuntime 'PotatoMapperApp.exe'),(Join-Path $oldPackage 'PotatoMapper.exe'),(Join-Path $package 'PotatoMapper.exe')){
    if(-not(Test-Path -LiteralPath $path -PathType Leaf)){throw "Missing fixture input: $path"}
}
if(([IO.File]::ReadAllText((Join-Path $package 'current.txt'))).Trim() -ne '0.9.0'){throw 'This verification requires the final 0.9.0 package.'}
$beforeBytes=[long]((Get-ChildItem -LiteralPath $repo -Recurse -File -Force | Measure-Object Length -Sum).Sum)
if($beforeBytes+220000000 -gt 1500000000){throw "Insufficient 1.5 GB budget: repository $beforeBytes bytes plus 220,000,000 reserved."}
$root=Join-Path $repo ('out\verification\upgrade-fixture-'+(Get-Date -Format 'yyyyMMdd-HHmmss')+'-'+[guid]::NewGuid().ToString('N').Substring(0,8))
$allowed=Join-Path $repo 'out\verification'
$root=[IO.Path]::GetFullPath($root)
if(-not $root.StartsWith($allowed+'\',[StringComparison]::OrdinalIgnoreCase) -or (Test-Path -LiteralPath $root)){throw 'Fixture root must be a fresh child of out/verification.'}
[IO.Directory]::CreateDirectory($root)|Out-Null
$install=Join-Path $root 'install'
$robustness=Join-Path $root 'robustness'
[IO.Directory]::CreateDirectory((Join-Path $install 'versions'))|Out-Null
[IO.Directory]::CreateDirectory($robustness)|Out-Null
$priorPath=$env:PATH
$priorSession=$env:POTATO_VERIFICATION_SESSION
$env:PATH=(Join-Path $QtPath 'bin')+';'+$env:PATH
$env:POTATO_VERIFICATION_SESSION='1'
$report=[ordered]@{Status='running';FixtureRoot=$root;InstallRoot=$install;SourceArchive=$archive;ArchiveSha256=(Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash;StorageBeforeBytes=$beforeBytes;ReservedBytes=220000000;Processes=@();Checks=@();ProjectorActivated=$false;Network='Deterministic mock HTTPS metadata/archive; actual ZIP extraction and portable launcher/editor used.'}
function Assert-Fixture([bool]$condition,[string]$label){
    if(-not $condition){throw $label}
    $report.Checks+= $label
    Write-Output ('PASS '+$label)
}
function Hash-Tree([string]$path){
    $lines=@(Get-ChildItem -LiteralPath $path -Recurse -File | Sort-Object FullName | ForEach-Object { $_.FullName.Substring($path.Length+1)+' '+(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash })
    $sha=[Security.Cryptography.SHA256]::Create()
    try{[Convert]::ToHexString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes(($lines -join "`n"))))}finally{$sha.Dispose()}
}
function Run-Checker([string]$destination,[string]$label,[bool]$handoff){
    $arguments=@(('"'+$archive+'"'),('"'+$package+'"'),'--install-root',('"'+$destination+'"'))
    if($handoff){$arguments+=@('--handoff','--launcher',('"'+(Join-Path $oldPackage 'PotatoMapper.exe')+'"'))}
    $process=Start-Process -FilePath $checker -ArgumentList $arguments -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $root ($label+'.stdout.txt')) -RedirectStandardError (Join-Path $root ($label+'.stderr.txt'))
    $report.Processes+=@{Role=$label;Id=$process.Id;Executable=$checker}
    if(-not $process.WaitForExit(45000)){throw "$label checker timed out (PID $($process.Id))"}
    Get-Content -LiteralPath (Join-Path $root ($label+'.stdout.txt'))
    Get-Content -LiteralPath (Join-Path $root ($label+'.stderr.txt'))
    Assert-Fixture ($process.ExitCode -eq 0) "$label update checker exit 0"
}
function Fixture-Editor([string]$version){
    $expected=Join-Path $install "versions\$version\PotatoMapperApp.exe"
    @(Get-Process PotatoMapperApp -ErrorAction SilentlyContinue | Where-Object {$_.Path -eq $expected})
}
function Await-Editor([string]$version){
    for($attempt=0;$attempt -lt 80;$attempt++){
        $editor=@(Fixture-Editor $version | Where-Object {$_.MainWindowTitle -like '*My mapping.pmap*'})
        if($editor.Count -eq 1){return $editor[0]}
        Start-Sleep -Milliseconds 500
    }
    throw "Fixture editor $version did not reopen My mapping.pmap."
}
function Close-FixtureEditor($editor){
    if(-not $editor.Path.StartsWith($install+'\',[StringComparison]::OrdinalIgnoreCase)){throw 'Refusing to close a process outside the disposable fixture.'}
    Assert-Fixture ($editor.CloseMainWindow()) "normal close requested for fixture editor PID $($editor.Id)"
    Assert-Fixture ($editor.WaitForExit(12000)) "fixture editor PID $($editor.Id) closed normally"
}
try{
    Run-Checker $robustness 'interruption-and-cancellation' $false
    # Deliberately copy only the historical runtime. Its Projects directory and
    # every original user mapping/media remain outside the test boundary.
    Copy-Item -LiteralPath $oldRuntime -Destination (Join-Path $install 'versions') -Recurse
    $oldTree=Hash-Tree (Join-Path $install 'versions\0.8.8')
    $originalTree=Hash-Tree $oldRuntime
    $oldLauncherHash=(Get-FileHash -LiteralPath (Join-Path $oldPackage 'PotatoMapper.exe') -Algorithm SHA256).Hash
    $expectedLauncherHash=(Get-FileHash -LiteralPath (Join-Path $package 'PotatoMapper.exe') -Algorithm SHA256).Hash
    $projectBytes=[Text.Encoding]::UTF8.GetBytes('{"format":"PotatoMapper","version":1,"width":1280,"height":720,"surfaces":[]}')
    $mediaBytes=[Text.Encoding]::UTF8.GetBytes('personal media fixture')
    Run-Checker $install 'handoff' $true
    $editor=Await-Editor '0.9.0'
    $report.Processes+=@{Role='upgraded-editor';Id=$editor.Id;Executable=$editor.Path;WindowTitle=$editor.MainWindowTitle}
    Assert-Fixture (([IO.File]::ReadAllText((Join-Path $install 'current.txt'))).Trim() -eq '0.9.0') 'current pointer selects 0.9.0'
    Assert-Fixture (([IO.File]::ReadAllText((Join-Path $install 'previous.txt'))).Trim() -eq '0.8.8') 'previous pointer retains 0.8.8'
    Assert-Fixture ((Hash-Tree (Join-Path $install 'versions\0.8.8')) -eq $oldTree) 'entire retained 0.8.8 runtime unchanged'
    Assert-Fixture ([Convert]::ToBase64String([IO.File]::ReadAllBytes((Join-Path $install 'My mapping.pmap'))) -eq [Convert]::ToBase64String($projectBytes)) 'original synthetic mapping exact bytes retained'
    Assert-Fixture ([Convert]::ToBase64String([IO.File]::ReadAllBytes((Join-Path $install 'media\clip.mp4'))) -eq [Convert]::ToBase64String($mediaBytes)) 'original synthetic media exact bytes retained'
    $managed=@(Get-ChildItem -LiteralPath (Join-Path $install 'Projects') -Recurse -File -Filter '*.pmap')
    Assert-Fixture ($managed.Count -gt 0) 'managed mapping collected into fixture Projects'
    $report.ManagedProjects=@($managed.FullName)
    for($attempt=0;$attempt -lt 40;$attempt++){
        if((Get-FileHash -LiteralPath (Join-Path $install 'PotatoMapper.exe') -Algorithm SHA256).Hash -eq $expectedLauncherHash){break}
        Start-Sleep -Milliseconds 250
    }
    Assert-Fixture ((Get-FileHash -LiteralPath (Join-Path $install 'PotatoMapper.exe') -Algorithm SHA256).Hash -eq $expectedLauncherHash) 'root launcher refreshed to final packaged launcher'
    $backup=@(Get-ChildItem -LiteralPath (Join-Path $install '.maintenance') -Recurse -File -Filter 'previous-PotatoMapper.exe')
    Assert-Fixture (@($backup | Where-Object {(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash -eq $oldLauncherHash}).Count -gt 0) 'original 0.8.8 launcher retained in maintenance backup'
    Close-FixtureEditor $editor
    $rollback=Start-Process -FilePath (Join-Path $install 'PotatoMapper.exe') -ArgumentList @('--rollback','--no-launch') -WindowStyle Hidden -PassThru -RedirectStandardOutput (Join-Path $root 'rollback.stdout.txt') -RedirectStandardError (Join-Path $root 'rollback.stderr.txt')
    $report.Processes+=@{Role='rollback-launcher';Id=$rollback.Id;Executable=$rollback.Path}
    Assert-Fixture ($rollback.WaitForExit(15000) -and $rollback.ExitCode -eq 0) 'rollback --no-launch completes successfully'
    Assert-Fixture (([IO.File]::ReadAllText((Join-Path $install 'current.txt'))).Trim() -eq '0.8.8') 'rollback current pointer selects 0.8.8'
    Assert-Fixture (([IO.File]::ReadAllText((Join-Path $install 'previous.txt'))).Trim() -eq '0.9.0') 'rollback retains 0.9.0 as alternate runtime'
    $legacyProject=Join-Path $install 'My mapping.pmap'
    $reopen=Start-Process -FilePath (Join-Path $install 'PotatoMapper.exe') -ArgumentList @('--project',('"'+$legacyProject+'"'),'--no-splash') -WindowStyle Hidden -PassThru
    $report.Processes+=@{Role='rollback-reopen-launcher';Id=$reopen.Id;Executable=(Join-Path $install 'PotatoMapper.exe')}
    $oldEditor=Await-Editor '0.8.8'
    $report.Processes+=@{Role='rolled-back-editor';Id=$oldEditor.Id;Executable=$oldEditor.Path;WindowTitle=$oldEditor.MainWindowTitle}
    Assert-Fixture ($oldEditor.MainWindowTitle -like '*My mapping.pmap*') 'actual retained 0.8.8 editor reopens safe v1 fixture'
    Close-FixtureEditor $oldEditor
    Assert-Fixture ((Hash-Tree $oldRuntime) -eq $originalTree -and (Hash-Tree (Join-Path $install 'versions\0.8.8')) -eq $oldTree) 'original old package and retained runtime remain unchanged after rollback reopen'
    Assert-Fixture ((Get-FileHash -LiteralPath (Join-Path $oldPackage 'PotatoMapper.exe') -Algorithm SHA256).Hash -eq $oldLauncherHash) 'original old package launcher unchanged'
    Assert-Fixture ([Convert]::ToBase64String([IO.File]::ReadAllBytes($legacyProject)) -eq [Convert]::ToBase64String($projectBytes) -and [Convert]::ToBase64String([IO.File]::ReadAllBytes((Join-Path $install 'media\clip.mp4'))) -eq [Convert]::ToBase64String($mediaBytes)) 'synthetic project and media remain exact after rollback reopen'
    $report.OldRuntimeTreeSha256=$oldTree
    $report.FinalLauncherSha256=$expectedLauncherHash
    $report.Status='pass'
}catch{
    $report.Status='fail';$report.Error=$_.Exception.Message
    Write-Error $_ -ErrorAction Continue
}finally{
    foreach($version in '0.9.0','0.8.8'){
        foreach($process in @(Fixture-Editor $version)){if(-not $process.HasExited){$null=$process.CloseMainWindow();$null=$process.WaitForExit(10000)}}
    }
    $env:PATH=$priorPath
    $env:POTATO_VERIFICATION_SESSION=$priorSession
    $report.StorageAfterBytes=[long]((Get-ChildItem -LiteralPath $repo -Recurse -File -Force | Measure-Object Length -Sum).Sum)
    if($report.StorageAfterBytes -gt 1500000000){$report.Status='fail';$report.Error='Repository storage cap exceeded.'}
    $report.RemainingFixtureEditors=@(foreach($version in '0.9.0','0.8.8'){Fixture-Editor $version | Select-Object Id,Path,MainWindowTitle})
    [IO.File]::WriteAllText((Join-Path $root 'proof.json'),($report|ConvertTo-Json -Depth 6),[Text.UTF8Encoding]::new($false))
    $summary="Portable update verification: $($report.Status)`r`nFixture: $root`r`nChecks: $($report.Checks.Count)`r`nRepository bytes: $($report.StorageAfterBytes)`r`nProof: $(Join-Path $root 'proof.json')`r`n"
    [IO.File]::WriteAllText((Join-Path $root 'report.txt'),$summary,[Text.UTF8Encoding]::new($false))
    Write-Output $summary
}
if($report.Status -ne 'pass'){throw "Portable update verification failed. See $(Join-Path $root 'proof.json')"}
