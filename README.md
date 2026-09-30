<p align="center"><img src="assets/potato-mapper.png" width="200" alt="A glowing potato being projected" /></p>

# Potato Mapper

A small Windows projection mapper for one projector. Put images, videos, or animated dots on a surface, drag its corners or mesh points into place, and send clean output to your projector. No watermark.

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

No Qt libraries, media files, or compiled binaries are committed. The application requires a graphics driver that supports OpenGL 3.3 Core. There is no installer or prebuilt release yet.

## Use it

1. Connect your projector and choose **Windows + P → Extend**.
2. Add a surface. Drop an image/video onto it, use **Load media**, or select **Animated dots**.
3. Drag corner handles; switch **Edit** to **Mesh points** for finer adjustments. Drag inside a surface to move it.
4. Choose the projector under **PROJECTOR OUTPUT** and click **Start output**.
5. Use **Stop**, or Escape while the output window has focus. **B** toggles blackout while a mapper window has focus.
6. Save your mapping. Media is referenced by file path, not embedded; keep those files available when moving a project.

Mouse wheel zooms the editor; middle-drag pans. Arrow keys nudge the selected handle/surface; Shift increases the step. Dropping media on a dots surface switches it back to media playback. **Back to white grid** removes the dots source.

## Continue development

Read [DEVELOPMENT.md](DEVELOPMENT.md) for the architecture, focused checks, and next steps. [AGENTS.md](AGENTS.md) gives coding agents the same working conventions.

Useful next additions include Screen/Add blending for black-background videos, more animated sources, colour controls, and a clearer media panel. Audio-reactive patterns, automatic calibration, 3D model import, edge blending, and multiple projector outputs are future work.

## License

[MIT](LICENSE). You can use, modify, and redistribute the project under those terms.

The potato logo was generated with AI. Qt and the media libraries used at runtime have their own licenses; this source repository does not bundle those dependencies.
