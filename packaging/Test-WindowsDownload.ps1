#requires -Version 7.2
param([Parameter(Mandatory)][string]$Archive)
$ErrorActionPreference='Stop'
$zip=Get-Item -LiteralPath $Archive
if($zip.Name -notmatch '^PotatoMapper-(\d+\.\d+\.\d+)-windows-x64\.zip$'){throw 'Expected a versioned Windows ZIP.'}
$version=$Matches[1]
$download=[IO.Path]::ChangeExtension($zip.FullName,'.exe')
$folder=Join-Path $zip.DirectoryName $zip.BaseName
if(Test-Path -LiteralPath $folder){throw 'Run this first-launch check beside fresh downloads.'}
$editorPath=Join-Path $folder "versions\$version\PotatoMapperApp.exe"
$launcher=Join-Path $folder 'PotatoMapper.exe'
function Run-DownloadProcess([string]$exe,[string[]]$arguments=@()){
    $info=[Diagnostics.ProcessStartInfo]::new($exe)
    $info.WorkingDirectory=$env:TEMP
    $info.UseShellExecute=$false
    $info.Environment['PATH']="$env:SystemRoot\System32;$env:SystemRoot"
    foreach($key in 'QT_PLUGIN_PATH','QT_QPA_PLATFORM_PLUGIN_PATH','QT_QPA_PLATFORM','QML2_IMPORT_PATH','QT_OPENGL','QT_SCALE_FACTOR','POTATO_VERIFICATION_SESSION'){
        $null=$info.Environment.Remove($key)
    }
    foreach($arg in $arguments){$info.ArgumentList.Add($arg)}
    $process=[Diagnostics.Process]::Start($info)
    try{
        if(-not $process.WaitForExit(60000)){throw "Timed out opening $exe"}
        if($process.ExitCode -ne 0){throw "$exe exited with $($process.ExitCode)"}
    }finally{$process.Dispose()}
}
Run-DownloadProcess $download
$deadline=[DateTime]::UtcNow.AddSeconds(40)
do{
    $editor=Get-Process PotatoMapperApp -ErrorAction SilentlyContinue |
        Where-Object {$_.Path -eq $editorPath -and $_.MainWindowTitle.Trim() -eq 'Potato Mapper'} | Select-Object -First 1
    if(-not $editor){Start-Sleep -Milliseconds 250}
}until($editor -or [DateTime]::UtcNow -ge $deadline)
if(-not $editor){throw 'Double-click path did not open the extracted Potato Mapper editor.'}
Write-Output "Opened editor on $env:COMPUTERNAME from $($editor.Path): $($editor.MainWindowTitle)"
if(-not $editor.CloseMainWindow() -or -not $editor.WaitForExit(15000)){throw 'The extracted editor did not close normally.'}

# Compare every shipped file against the ZIP used by the updater.
$reader=[IO.Compression.ZipFile]::OpenRead($zip.FullName)
$sha=[Security.Cryptography.SHA256]::Create()
try{
    $count=0
    foreach($entry in $reader.Entries){
        if(-not $entry.Name){continue}
        $item=Join-Path $zip.DirectoryName $entry.FullName
        $stream=$entry.Open()
        try{$expected=[Convert]::ToHexString($sha.ComputeHash($stream))}finally{$stream.Dispose()}
        if((Get-FileHash -LiteralPath $item).Hash -ne $expected){throw "Downloaded payload differs: $($entry.FullName)"}
        $count++
    }
    Write-Output "Verified all $count packaged files against the ZIP."
}finally{$reader.Dispose();$sha.Dispose()}

$fixture=Join-Path $folder 'Projects\download-test.txt'
[IO.File]::WriteAllText($fixture,'Saved-work fixture')
$stamp=(Get-Item -LiteralPath $editorPath).LastWriteTimeUtc
Run-DownloadProcess $download @('--extract-only')
if([IO.File]::ReadAllText($fixture) -ne 'Saved-work fixture' -or (Get-Item $editorPath).LastWriteTimeUtc -ne $stamp){
    throw 'Reopening the download changed the saved fixture or replaced its editor.'
}
Remove-Item -LiteralPath $fixture
Run-DownloadProcess $launcher @('--test-splash')
if(-not (Select-String -LiteralPath (Join-Path $folder 'mapper.log') -SimpleMatch 'SPLASH CHECK true')){throw 'Root launcher did not render the packaged splash.'}
Write-Output 'PASS double-click, editor, ZIP contents, repeat open, saved file and root launcher.'
