<p align="center"><img src="assets/potato-mapper.png" width="200" alt="A glowing potato being projected" /></p>

# Potato Mapper

A small Windows projection mapper for one projector. Put images, videos, or animated dots on a surface, drag its corners or mesh points into place, and send clean output to your projector. No watermark.

## Download and run

**[Download Potato Mapper for Windows x64](https://github.com/TheGamingDonKey/potato-mapper/releases/download/v0.2.0/PotatoMapper-0.2.0-windows-x64.zip)**

1. Download the app ZIP above, right-click it, and choose **Extract All**.
2. Open the extracted folder and double-click **PotatoMapper.exe**.
3. Keep the included DLL files and folders beside the executable.

No installation, Visual Studio, Qt setup, or GitHub account is required. This build targets **64-bit Intel/AMD Windows 10 (22H2) or Windows 11**, with an **OpenGL 3.3 Core** graphics driver. It has been checked locally on Windows 11; testing on other computers is still limited. It is not a macOS or Linux build.

Use the app ZIP under [Releases](https://github.com/TheGamingDonKey/potato-mapper/releases). GitHub's **Code → Download ZIP** and **Source code** downloads contain programming files, not the ready-to-run app. The other release archives provide library source code for developers; you only need the Windows app ZIP to run it.

## Why it exists

Because MadMapper's demo watermark pissed me off.

I wanted to plug in my projector at home, map a few squares onto a wall or an object, and drop media onto them. That became Potato Mapper: a working starting point we can keep improving with developers and AI coding tools.

This is an independent project, unaffiliated with MadMapper. It currently focuses on manual mapping for one projector.

## What works today

- Multiple surfaces with draggable corners and a subdivided mesh.
- Images and looping video, with stretch, fit, and crop options.
- Transparent white alignment grids.
- Animated cyan-and-white dots with speed, size, and pause controls.
- Surface ordering, duplication, visibility, position locking, undo, and redo.
- Save/open `.pmap` projects; earlier `.hmap` files still load.
- Separate fullscreen projector output, display selection, and blackout.

Early prototype: expect rough edges. It has been run on Windows 11 with an AMD Radeon 840M. Automated checks cover animated editor/output rendering, pause, and saved pattern settings. A user has tried the basic mapping workflow; broad hardware and projector compatibility has not been established.

## Build on Windows

Install:

1. **Visual Studio 2022 or Build Tools 2022**, with **Desktop development with C++** and a Windows SDK.
2. **Qt 6.10 or later**, the **MSVC 2022 64-bit** build, including **Qt Multimedia**. The working development setup uses Qt **6.10.3**.
3. **CMake 3.24 or later**.

Clone this repository and run in PowerShell, substituting your Qt installation path:

```powershell
.\build.ps1 -QtPath 'C:\Qt\6.10.3\msvc2022_64'
.\app\PotatoMapper.exe
```

The script builds Release and uses `windeployqt` to copy runtime dependencies into `app`. It accepts `-CMakePath` and `-BuildDirectory` overrides. `QT_ROOT_DIR` or `QTDIR` can supply the Qt path. The original development machine's cached tools remain a fallback.

No Qt libraries, media files, or compiled binaries are committed to the source tree. Ready-to-run binaries and library sources are distributed through GitHub Releases.

To create a portable ZIP after building, run `./package.ps1 -QtPath 'C:\Qt\6.10.3\msvc2022_64'`. Pass the same `-BuildDirectory` if you overrode it during the build. The packager finds the installed Visual C++ x64 redistributable DLLs, deploys Qt into a fresh folder, and includes the runtime notices. Output goes under `out/releases`. The current notices and source archives correspond to Qt 6.10.3 and FFmpeg 7.1.3; update them when upgrading dependencies.

## Use it

1. Connect your projector and choose **Windows + P → Extend**.
2. Add a surface. Drop an image/video onto it, use **Load media**, or select **Animated dots**.
3. Drag corner handles; switch **Edit** to **Mesh points** for finer adjustments. Drag inside a surface to move it.
4. Choose the projector under **PROJECTOR OUTPUT** and click **Start output**.
5. Use **Stop**, or Escape while the output window has focus. **B** toggles blackout while a mapper window has focus.
6. Save your mapping. Media is referenced by file path, not embedded; keep those files available when moving a project.

Mouse wheel zooms the editor; middle-drag pans. Arrow keys nudge the selected handle/surface; Shift increases the step. Dropping media on a dots surface switches it back to media playback. **Back to white grid** removes the dots source.

## Continue development

See the [roadmap checklist](ROADMAP.md) for completed features, sensible next steps, and where development stopped. Read [DEVELOPMENT.md](DEVELOPMENT.md) for the architecture and focused checks. [AGENTS.md](AGENTS.md) gives coding agents the same working conventions.

## License

[MIT](LICENSE). You can use, modify, and redistribute the project under those terms.

The potato logo was generated with AI. Bundled Qt, FFmpeg, and Microsoft runtime libraries have their own terms; see [runtime notices](packaging/THIRD-PARTY.txt). Library sources accompany the binary release.
