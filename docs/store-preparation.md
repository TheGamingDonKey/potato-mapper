# Microsoft Store preparation

The native Qt editor has a Store distribution profile alongside the direct-download portable distribution. Both use the same project format, rendering, media playback and editor code. This is preparation for a Windows x64 MSIX submission; it is not a published Store release or a certification result.

## Distribution behavior

`src/distribution.h` detects Windows package identity with `GetCurrentPackageFullName`. `POTATO_STORE_BUILD=1` also enables the profile before installation, which is useful for local checks. CMake's `POTATO_STORE_BUILD` option defaults to `OFF`.

For Store/profile launches, the managed project library is `QStandardPaths::DocumentsLocation/Potato Mapper Projects`. This uses the user's Windows Documents location, including a configured redirection. Project files and collected media live outside the replaceable package. Direct-download launches keep the existing installation-root `Projects` directory. The helper returns an empty library override in direct mode so the existing install-root policy remains authoritative. Opening/importing a direct-download project does not authorize deleting or moving the original.

The Store profile's update dialog explains that Microsoft Store manages its updates. It does not contact GitHub, download an update, extract ZIPs or launch the portable updater. The direct channel retains its existing launcher/update behavior. Package identity also applies to a sideloaded test MSIX; `channelName()` labels that profile `Microsoft Store` without establishing that it was acquired through the Store.

The MSIX declares `.pmap` and legacy `.hmap` file associations for the full-trust editor. Windows invokes `PotatoMapperApp.exe --project "%1"`, with the expanded path quoted so spaces remain one argument. `MultiSelectModel="Document"` requests a separate editor process per selected file. This uses the existing `--project` parser; it needs no UWP activation handler, shell script or portable launcher. Association registration happens only when an MSIX is installed. The manifest offers the app as a handler; Windows and the user retain control of default-app choices. Verify actual Explorer activation after installation, including spaces, Unicode names, multiple selected files, both extensions and an already running editor.

## Identity required from the owner

Reserve the app in Partner Center, then provide its actual **Package/Identity/Name**, **Package/Identity/Publisher**, **PublisherDisplayName**, and reserved **DisplayName**. These are mandatory packager parameters. A guessed publisher or a display name chosen only in this repository cannot establish a Store identity.

The four-part `PackageVersion` is separate from the editor's displayed semantic version. The Store package version must have a nonzero first component, each component must fit 0..65535, and the fourth component must be zero. For example, an initial `1.0.0.0` package may contain a `0.9.0` editor; select an actual version appropriate to the reserved product's submission history. Later packages for the same identity must increase that package version.

The manifest requests `runFullTrust` because this is a conventional desktop Qt/OpenGL application. Its justification and acceptance belong to the Partner Center submission. It targets Windows Desktop x64, minimum Windows 10 22H2/build 19045, with maximum tested version declared separately; that declaration is not evidence that a Windows version was tested. OpenGL 3.3 and working projector/display drivers are still required.

## Build and package

Use an existing Release runtime deployed by `package.ps1` or `build.ps1`, and the current built editor. The Store packager maps that runtime's dependencies directly into the MSIX. It does not duplicate the runtime into another staging folder. It maps only the editor, root DLLs, known Qt plugin DLLs, repository examples, third-party notices/licenses, and deployed dependency SBOMs. Portable launchers, updater helpers, installation markers, user projects and media are excluded.

For a Store-only local build, configure a separate build directory:

```powershell
$qt = 'C:\Qt\6.10.3\msvc2022_64' # use the actual existing Qt installation
cmake -S . -B out\build-store -G 'Visual Studio 17 2022' -A x64 "-DCMAKE_PREFIX_PATH=$qt" -DPOTATO_STORE_BUILD=ON
cmake --build out\build-store --config Release --target PotatoMapper --parallel 4
```

Check the storage breakdown before starting another build. A second complete build is optional: the normal editor detects identity automatically after MSIX installation. An override executable can reuse an existing compatible deployed Qt/FFmpeg runtime without deploying it again. Keep Qt, CRT, plugin and FFmpeg versions compatible with that executable. The deployment's `licenses/*-6.10.3.spdx.json` files are required; update the packager and notices when dependency versions change.

Read-only dependency preflight:

```powershell
$runtime = 'C:\absolute\existing-portable-app\versions\0.9.0'
.\packaging\store\Test-StoreRuntime.ps1 -RuntimeDirectory $runtime |
    Select-Object BinaryCount, PayloadBytes, Validation
```

After obtaining the real Partner Center values, define `$packageName`, `$publisher`, `$publisherDisplayName`, `$storeDisplayName` and `$packageVersion`, then run:

```powershell
$candidate = .\packaging\store\New-StorePackage.ps1 `
    -RuntimeDirectory $runtime `
    -PackageName $packageName -Publisher $publisher `
    -PublisherDisplayName $publisherDisplayName -StoreDisplayName $storeDisplayName `
    -PackageVersion $packageVersion
# Optional: -EditorExecutable .\out\build-store\Release\PotatoMapperApp.exe
# Optional: -PrepareOnly generates manifest, logos and mapping without creating MSIX.
$candidate
```

The script discovers an installed Windows SDK x64 `MakeAppx.exe` and Visual C++ `dumpbin.exe`, or accepts explicit paths. It installs nothing. It fails on missing Qt/FFmpeg/CRT dependencies, unresolved imports on this host, non-x64 PE files, symbolic links, incorrect logo dimensions or exhausted storage allowance. It does not take DLLs from Windows or from the developer's PATH. Windows-owned `icuuc.dll` must not be redistributed from a developer machine.

To exercise MakeAppx before public Store identity is supplied, use an explicitly synthetic QA identity with `-LocalValidationIdentity`. The JSON report and status label the result **local packaging validation only, not a Store candidate**. This does not reserve a Store identity or authorize installation/signing. For the current candidate runtime:

```powershell
.\packaging\store\New-StorePackage.ps1 `
    -RuntimeDirectory '.\out\releases\PotatoMapper-0.9.0-windows-x64\versions\0.9.0' `
    -PackageName 'PotatoMapper.LocalValidation' `
    -Publisher 'CN=PotatoMapperLocalValidation' `
    -PublisherDisplayName 'Local packaging validation only' `
    -StoreDisplayName 'Potato Mapper QA - not Store' `
    -PackageVersion '1.0.0.0' -LocalValidationIdentity
```

Output is a fresh repository `out/store/<version>-<id>` directory with generated logos, `AppxManifest.xml`, an absolute file mapping, a JSON report, and—unless `-PrepareOnly`—an **unsigned** `.msix`. MakeAppx semantic validation remains enabled. Generated files do not overwrite an existing output. The report distinguishes prepared, unsigned package, signing, installed testing and certification states. Changing source/dependencies after generating a mapping changes its inputs; regenerate the candidate and record its final hash before testing or submission.

The storage cap is **1,500,000,000 bytes** across this repository, including existing build/output files and `.git`. Before writing output, the script reserves current usage plus twice the uncompressed selected payload and 64 MiB of headroom. It checks final usage as well. It never deletes anything to make room. External Qt/tool installations are not counted by this repository limit. Concurrent unrelated writes and arbitrary SDK temporary usage cannot be guaranteed by an estimate; avoid concurrent packaging and recheck actual usage.

## Verification still required for a release

Static imports and MakeAppx validation do not prove dynamic plugin loading or installed operation on another computer. Before submission:

1. Review the real identity, display assets, desktop/full-trust capability justification, privacy/support information, age rating and dependency redistribution obligations. Store screenshots/listing art are separate from the three generated package logos; inspect the small logos visually before accepting them.
2. Sign a test copy with a certificate whose subject exactly matches the manifest publisher and trust it only on the intended test system. The packager does not create certificates, change certificate trust, install packages or store credentials. Follow Microsoft's signing/testing instructions below. A test certificate is not a published Store signature.
3. Install the signed package on a clean Windows x64 test machine. Launch from its registered Start entry. Verify images, video/audio, shaders, examples, projector selection/fullscreen output and the Store update message. Confirm no GitHub update request or package-directory writes occur. Record Windows/GPU and actual projector evidence separately.
4. Save a synthetic project with referenced media to the Documents library. Close/reopen it, update the installed package to a higher package version, then reopen again. Verify project/media bytes and references. Check interruption/recovery, direct-channel import with originals retained, and the project library after uninstall. Do not use personal work for this test.
5. Run Windows App Certification Kit and read its actual report. Its presence on this machine does not mean a candidate passed it. Example command after a test package exists:

   ```powershell
   $appcert = 'C:\Program Files (x86)\Windows Kits\10\App Certification Kit\appcert.exe'
   & $appcert reset
   & $appcert test -appxpackagepath $signedPackage -reportoutputpath $reportFile
   ```

6. Submit through the actual Partner Center product and resolve its validation/certification findings. MakeAppx creates an MSIX, not an `.msixupload` container. A local unsigned MSIX or passing local report is not proof of an accepted submission or Store availability.

## Evidence on this host

On October 29, 2026, Windows SDK `10.0.26100.0` x64 MakeAppx and Windows App Certification Kit `appcert.exe` were found. The PowerShell scripts parsed successfully. Asset conversion produced validated PNGs at 50x50, 44x44 and 150x150 (43,313 bytes total). A read-only preflight of the existing `app/versions/0.3.0` deployment validated 37 x64 binaries totaling 58,008,248 bytes; this validates the tool against an old deployment, not a current Store candidate. The source/profile requires integration checks against the current build. No real Store identity was supplied. Integration subsequently produced an unsigned local-validation MSIX from the 0.9.0 deployed runtime using SDK MakeAppx with schema validation, 37 x64 binaries, known repository examples and .pmap/.hmap associations. The synthetic QA identity is explicitly marked not a Store candidate. No package was signed, installed, WACK-tested, submitted or published. See out/store-validation/store-package-report.json and the 0.9.0 local-drop handoff.

## Microsoft references

- [Manual MSIX components and desktop manifest](https://learn.microsoft.com/en-us/windows/msix/desktop/desktop-to-uwp-manual-conversion)
- [MakeAppx file mappings and validation](https://learn.microsoft.com/en-us/windows/msix/package/create-app-package-with-makeappx-tool)
- [Package version rules](https://learn.microsoft.com/en-us/windows/apps/publish/publish-your-app/package-version-numbering?pivots=store-installer-msix)
- [Package identity detection](https://learn.microsoft.com/en-us/windows/win32/api/appmodel/nf-appmodel-getcurrentpackagefullname)
- [Desktop file associations and quoted activation parameters](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/desktop-to-uwp-extensions)
- [File association parameters and document selection model](https://learn.microsoft.com/en-us/uwp/schemas/appxpackage/uapmanifestschema/element-uap3-filetypeassociation)
- [Packaged desktop file behavior](https://learn.microsoft.com/en-us/windows/msix/desktop/desktop-to-uwp-behind-the-scenes)
- [Signing a package](https://learn.microsoft.com/en-us/windows/msix/package/sign-app-package-using-signtool)
- [Running and testing installed MSIX](https://learn.microsoft.com/en-us/windows/msix/desktop/desktop-to-uwp-debug)
- [Windows App Certification Kit](https://learn.microsoft.com/en-us/windows/uwp/debug-test-perf/windows-app-certification-kit)
