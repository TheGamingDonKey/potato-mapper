<p align="center"><img src="assets/potato-mapper.png" width="200" alt="A glowing potato being projected" /></p>

# Potato Mapper

A small Windows projection mapper for one projector. Put images, videos, or built-in animations on a surface, drag its corners or mesh points into place, and send clean output to your projector. No watermark.

## Download and run

**[Download Potato Mapper for Windows x64](https://github.com/TheGamingDonKey/potato-mapper/releases/download/v0.4.0/PotatoMapper-0.4.0-windows-x64.zip)**

1. Download the app ZIP above, right-click it, and choose **Extract All**.
2. Open the extracted folder and double-click **PotatoMapper.exe**.
3. Keep `PotatoMapper.exe`, `installation.txt`, `current.txt`, and the `versions` folder together.

No installation, Visual Studio, Qt setup, or GitHub account is required. This build targets **64-bit Intel/AMD Windows 10 (22H2) or Windows 11**, with an **OpenGL 3.3 Core** graphics driver. It has been checked locally on Windows 11; testing on other computers is still limited. It is not a macOS or Linux build.

Use the app ZIP under [Releases](https://github.com/TheGamingDonKey/potato-mapper/releases). GitHub's **Code → Download ZIP** and **Source code** downloads contain programming files, not the ready-to-run app. The other release archives provide library source code for developers; you only need the Windows app ZIP to run it.

## Why it exists

Because MadMapper's demo watermark pissed me off.

I wanted to plug in my projector at home, map a few squares onto a wall or an object, and drop media onto them. That became Potato Mapper: a working starting point we can keep improving with developers and AI coding tools.

This is an independent project, unaffiliated with MadMapper. It currently focuses on manual mapping for one projector.

## What works today

The v0.5.0 blending/pattern additions below are implemented on the development branch and awaiting publication; the download above is still v0.4.0.

- Multiple surfaces with draggable corners and a subdivided mesh.
- Images and looping video, with stretch, fit, and crop options.
- Transparent white alignment grids.
- Potato FX: animated dots, diagonal stripes, expanding rings and square waves, with speed, size/line width and pause.
- Surface ordering, duplication, visibility, position locking, undo, and redo.
- Save/open `.pmap` projects; earlier `.hmap` files still load.
- Separate fullscreen projector output, display selection, and blackout.
- Right-click a surface to load/clear media or rename it. Clearing keeps the mesh and supports Undo.
- Branded startup splash, surface content labels, File/Edit/Help menus, and an About/version display.
- Resizable sidebar, persistent surface list, collapsible settings, themed scrollbars and full-path filename tooltips.
- Per-surface brightness and opacity, with live preview, one Undo per slider drag and saved settings. Older mappings default to 100%.
- **Appearance → Blend:** Normal, Screen and Add for layering images, video and patterns. Screen/Add let suitable black-background media reveal the surfaces below.
- **Help → Check for updates** downloads and installs newer stable Windows releases, then reopens your saved mapping. The previous app version is retained for recovery.

Early prototype: expect rough edges. It has been run on Windows 11 with an AMD Radeon 840M. Automated checks cover animated editor/output rendering, pause, and saved pattern settings. A user has tried the basic mapping workflow; broad hardware and projector compatibility has not been established.

## Build on Windows

Install:

1. **Visual Studio 2022 or Build Tools 2022**, with **Desktop development with C++** and a Windows SDK.
2. **Qt 6.10 or later**, the **MSVC 2022 64-bit** build, including **Qt Multimedia**. The working development setup uses Qt **6.10.3**.
3. **CMake 3.24 or later**.

Clone this repository and run in PowerShell, substituting your Qt installation path:

```powershell
.\build.ps1 -QtPath 'C:\Qt\6.10.3\msvc2022_64'
# Open PotatoMapper.exe at the "Ready to run" path printed by the script.
```

The script builds Release and deploys a fresh runnable folder under `out/dev-app-<timestamp>`. Existing app folders and personal files are preserved. Use `-AppDirectory` to choose a new destination; it must not already exist. It also accepts `-CMakePath` and `-BuildDirectory` overrides. `QT_ROOT_DIR` or `QTDIR` can supply the Qt path. The original development machine's cached tools remain a fallback.

No Qt libraries, media files, or compiled binaries are committed to the source tree. Ready-to-run binaries and library sources are distributed through GitHub Releases.

To create a portable ZIP after building, run `./package.ps1 -QtPath 'C:\Qt\6.10.3\msvc2022_64'`. Pass the same `-BuildDirectory` if you overrode it during the build. The packager finds the installed Visual C++ x64 redistributable DLLs, deploys Qt into a fresh folder, and includes the runtime notices. Output goes under `out/releases`. The current notices and source archives correspond to Qt 6.10.3 and FFmpeg 7.1.3; update them when upgrading dependencies.

## Use it

1. Connect your projector and choose **Windows + P → Extend**.
2. Add a surface. Drop an image/video onto it, use **Load media**, or open **Potato FX** and choose an animation.
3. Drag corner handles; switch **Edit** to **Mesh points** for finer adjustments. Drag inside a surface to move it.
4. Choose the projector under **PROJECTOR OUTPUT** and click **Start output**.
5. Use **Stop**, or Escape while the output window has focus. **B** toggles blackout while a mapper window has focus.
6. Save your mapping. Media is referenced by file path, not embedded; keep those files available when moving a project.

Mouse wheel zooms the editor; middle-drag pans. Arrow keys nudge the selected handle/surface; Shift increases the step. Dropping media on an animation surface switches it back to media playback. **Clear media** returns an image, video, or animation surface to the white grid. Double-click a surface's list entry to rename it.

Drag the divider beside the surface list to resize the sidebar. Click **Media**, **Appearance**, **Potato FX** or **Mapping** headings to expand/collapse their controls. Appearance sliders dim/fade the selected surface; **Reset appearance** restores both to 100% and Blend to Normal. Screen gives a softer bright overlay; Add adds light and can clip to white. Move the overlay above the other surface using **Forward**, overlap their mapped areas, then choose Screen/Add. More patterns and controls remain on the roadmap. Earlier app versions ignore the new appearance settings and do not retain them when saving.

## Updating an existing copy

**From v0.2.0:** close the old app, extract the new ZIP, then copy the new package's contents into your existing app folder. Replace `PotatoMapper.exe` when asked and keep your mapping files and media. This one-time upgrade adds the launcher. You can also extract into a fresh folder and open existing mappings from there.

**From v0.3.0 onward:** use **Help → Check for updates**. Downloads show progress and can be cancelled. Before restarting, the app asks you to save/discard/cancel unsaved changes and stop any active projection. Your saved mapping reopens after installation. **Help → Use previous app version** switches back to the retained version; it does not undo edits to saved projects.

Save personal files outside `versions` and `.updates`, which are managed app folders. Updates replace the selected runtime rather than the whole app folder. Projects and linked media can remain in the app root or anywhere else you choose. Updates require an internet connection and a writable app folder; mapping still works offline. There is no automatic update check during projection.

## Continue development

See the [roadmap checklist](ROADMAP.md) for completed features, sensible next steps, and where development stopped. Read [DEVELOPMENT.md](DEVELOPMENT.md) for the architecture and focused checks. [AGENTS.md](AGENTS.md) gives coding agents the same working conventions.

## License

[MIT](LICENSE). You can use, modify, and redistribute the project under those terms.

The potato logo was generated with AI. Bundled Qt, FFmpeg, and Microsoft runtime libraries have their own terms; see [runtime notices](packaging/THIRD-PARTY.txt). Library sources accompany the binary release.
