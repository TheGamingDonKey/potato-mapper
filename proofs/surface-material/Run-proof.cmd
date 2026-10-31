@echo off
setlocal
for %%R in ("%~dp0..\..") do set "POTATO_PROOF_REPO=%%~fR"
set "PATH=%POTATO_PROOF_REPO%\app\versions\0.8.8;%PATH%"
set "QT_PLUGIN_PATH=%POTATO_PROOF_REPO%\app\versions\0.8.8"
if not exist "%POTATO_PROOF_REPO%\out\surface-material-proof\session" mkdir "%POTATO_PROOF_REPO%\out\surface-material-proof\session"
"%POTATO_PROOF_REPO%\out\surface-material-proof\Release\PotatoSurfaceMaterialProof.exe" --install-root "%POTATO_PROOF_REPO%\out\surface-material-proof\session" %*
