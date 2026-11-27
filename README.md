<p align="center"><img src="assets/potato-mapper.png" width="200" alt="A glowing potato being projected" /></p>

# Potato Mapper

A small Windows projection mapper for one projector. Put images, videos, or built-in animations on a surface, drag its corners or mesh points into place, and send clean output to your projector. No watermark.

## Download and run

**[Download Potato Mapper 0.9.14 for Windows](https://github.com/TheGamingDonKey/potato-mapper/releases/download/v0.9.14/PotatoMapper-0.9.14-windows-x64.exe)** · [Portable ZIP](https://github.com/TheGamingDonKey/potato-mapper/releases/download/v0.9.14/PotatoMapper-0.9.14-windows-x64.zip) · [All releases](https://github.com/TheGamingDonKey/potato-mapper/releases)

1. Double-click the Windows download in **Downloads**. It creates a complete app folder beside the file and opens Potato Mapper. For the ZIP, choose **Extract All**, then open **PotatoMapper.exe** in the extracted folder.
2. Choose **File → Examples** for a ready-made Christmas scene. Edit the animation on the selected surface in **Potato FX**.
3. Connect one projector, choose **Windows + P → Extend**, select the projector under **Projector Output**, and click **Start output**.

Keep the complete app folder together when moving it. **Projects** holds your saved work. Existing users can use **Help → Check for updates** in their installed app to retain their projects and previous runtime.

Supports 64-bit Intel/AMD Windows 10 (22H2) or Windows 11, with an OpenGL 3.3 Core graphics driver. The download includes the app's Qt and Visual C++ runtime libraries. GitHub's **Code → Download ZIP** contains the developer source. Use one of the app downloads above to run Potato Mapper.

## Christmas studio

Choose **File → Examples → Santa's Sleigh Ride** for one Santa and two reindeer flying across two mapped panels. Drag the panels onto the objects you want to light. To use your own mapping, apply the same animation in **Potato FX** to each participating surface. They share a 24-second flight and one set of motion controls. Santa follows changes in panel height, continues through gaps and appears again when he reaches the next surface. Stack panels vertically for a top-to-bottom flight or use **Reverse movement** to change direction.

**North Pole Express**, **Evergreen Overture**, **Clocktower Carols** and **Nutcracker Waltz** each offer **Potato FX → Subject only**. Turn it on to project the moving train, decorated tree, bell tower or dancer with open space around the artwork. Existing projects open with their original scenery. Transparent areas reveal any lower layer. With an empty stage, the projector receives black in those areas. A projector’s own black level and the room lighting determine how visible its footprint is on your wall.

[Watch the native projection previews](https://potato-mapper.batibandit.chatgpt.site) · [Download Santa on black as a looping MP4](https://github.com/TheGamingDonKey/potato-mapper/releases/download/v0.9.14/santa-flight.mp4)

## Why it exists

Because MadMapper's demo watermark pissed me off.

I wanted to plug in my projector at home, map a few squares onto a wall or an object, and drop media onto them. That became Potato Mapper: a working starting point we can keep improving with developers and AI coding tools.

This is an independent project, unaffiliated with MadMapper. It currently focuses on manual mapping for one projector.

## What works today

- Multiple surfaces with draggable corners and a subdivided mesh.
- Images and looping video, with stretch, fit, and crop options.
- Transparent white alignment grids.
- Potato FX includes flowing SquareWave cells, Diagonals, CubicCircles and SquareArray. Speed, size/line width, density, colour, angle, reverse and optional soft edges; Flow adds variation to the flowing effects. **Maple Supper** and **Harvest Lanterns** add warm Thanksgiving scenes with drifting foliage, pumpkins and soft lantern light.
- Surface ordering, duplication, visibility, position locking, undo, and redo.
- Save/open `.pmap` projects; earlier `.hmap` files still load.
- Separate fullscreen projector output, display selection, and blackout.
- **Potato Scan:** an eight-second entrance that traces the mapped edges, assembles a scan grid, locks the panels and smoothly reveals their media or FX. Replay and Skip controls, with an optional entrance on output start.
- **Dynamic FX:** Hot-Ass Potato sends a curved, tapered snake between mapped surfaces. Nearby cells unfold, stretch and rotate, then settle into coordinated motion. Snake width and Cell size use projector pixels, so smaller panels contain fewer cells. Potato Focus provides a coordinated holographic field.
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

Ready-to-run Windows binaries and corresponding library sources are distributed through GitHub Releases. The website includes scenes captured from the native renderer. Personal mappings and media stay in the local app folder.

To create the portable ZIP and self-extracting Windows EXE after building, run `./package.ps1 -QtPath 'C:\Qt\6.10.3\msvc2022_64'`. Pass the same `-BuildDirectory` if you overrode it during the build. The packager finds the Visual C++ x64 redistributable DLLs, deploys Qt into a fresh folder, and includes the runtime notices. It embeds that exact ZIP in the portable EXE using the Windows .NET Framework compiler. Output goes under `out/releases`. The notices and source archives correspond to Qt 6.10.3 and FFmpeg 7.1.3.

## Use it

1. Connect your projector and choose **Windows + P → Extend**.
2. Add a surface. Drop an image/video onto it, use **Load media**, or open **Potato FX** and choose an animation.
3. Drag corner handles; switch **Edit** to **Mesh points** for finer adjustments. Drag inside a surface to move it.
4. Choose the projector under **PROJECTOR OUTPUT** and click **Start output**.
5. Use **Stop**, or Escape while the output window has focus. **B** toggles blackout while a mapper window has focus.
6. Save your mapping. **Projects** in the app folder is the default location. Media stays linked; opening a project collects a portable copy with its referenced media when those files are available.

Mouse wheel zooms the editor; middle-drag pans. Arrow keys nudge the selected handle/surface; Shift increases the step. Dropping media on an animation surface switches it back to media playback. **Clear media** returns an image, video, or animation surface to the white grid. Double-click a surface's list entry to rename it.

Drag the divider beside the surface list to resize the sidebar. Click **Media**, **Appearance**, **Potato FX** or **Mapping** headings to expand/collapse their controls. Appearance sliders dim/fade the selected surface; **Reset appearance** restores both to 100% and Blend to Normal. Screen gives a softer bright overlay; Add adds light and can clip to white. Move the overlay above the other surface using **Forward**, overlap their mapped areas, then choose Screen/Add. Open Potato FX and choose a flowing effect. Start with White, Soft edge 0%, Speed 100% and Flow 70%. Increase Density for smaller elements, then adjust Cell size or Line width. SquareWave uses dense stretching cells; Diagonals uses rotating strokes; CubicCircles morphs filled cells into circles/rings; SquareArray moves rows of changing blocks. Waterlight draws a shimmering light web; Contour Flow drifts landscape-like lines; Curve Maze joins curved paths; Depth Tunnel creates a perspective grid. Older dots, stripes, rings and the former square waves (now labelled Stepped waves) remain available. Earlier app versions ignore the new appearance settings and do not retain them when saving.

### Halloween and fall scenes

Open **File → Examples → October Porch** for a mixed three-panel Halloween/autumn composition. **Pumpkin Mischief**, **Ghost Parade** and **Rainlit Window** also have individual examples and appear in **Potato FX → Animation** on your existing surfaces.

- **Pumpkin Mischief:** three glowing jack-o'-lanterns trade glances, a small pumpkin hiccups embers, and the large one answers with a wink.
- **Ghost Parade:** a lantern guide leads pale ghosts around a gentle closed route; a shy follower lags and catches up while their cloth flutters.
- **Rainlit Window:** layered rain, an amber reflection and copper leaves on a dark sill create a quieter October/November evening.

Start with **Original** colour and **Speed 100%**. Scene size defaults to **24%**; Density defaults to **24 / 32 / 24** and Flow to **60 / 60 / 50** respectively. Density controls embers, ghost count or rain spacing; Flow changes expressive movement, flutter or gusts. Flow 0 retains basic animation. Pause and Reverse work with the same smooth twelve-second choreography. Choose each effect on any surface; the ghost route stays within that panel.

The examples need no media downloads and arrive with the update. These three presets require **0.8.8 or later**. Earlier apps can replace unrecognized new effects if you save there; keep the original mapping when trying an older runtime.

### Thanksgiving and autumn scenes

Open **File → Examples → Maple Supper** for drifting red/gold maple leaves, uncarved pumpkins and a friendly scarf-wearing potato. **Harvest Lanterns** offers a quieter scene with candle lanterns and gently breathing amber halos. Both also appear in the **Potato FX → Animation** picker for any surface you already mapped.

Start with **Original** colours, **Speed 100%**, **Scene size 24%** and **Flow 70%**. Scene size changes the illustrated elements, Density changes the amount of foliage, and Flow changes sway and glow. Each scene repeats over twelve seconds at 100% speed; Pause and Reverse work as usual. Colour choices retain shading, so faces and lantern details remain readable. Brightness, opacity, Screen/Add and soft edges still apply.

The examples are self-contained and travel with the update; no media download is needed. These presets require 0.8.7 or later. Earlier apps do not recognize the new IDs and can replace them with an older effect if you save there. Existing effect IDs and project formats remain unchanged.

### Potato Scan entrance

Shipped in [0.8.4](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.4).

Under **PROJECTOR OUTPUT**, click **Play entrance** to preview or replay Potato Scan across your visible mapped surfaces. **On output start** is enabled by default; clear it if Start output should immediately show your normal projection. **Skip** restores the projection immediately. Blackout hides the scan and pauses its clock; Stop or Escape ends it. Opening a different project also ends it.

Starting an entrance turns blackout off automatically, so **Play entrance** can reveal the scan in one click. You can still switch blackout on during the entrance to pause it. With **On output start** disabled, Start output retains your blackout setting.

The scan follows your actual corners and mesh deformation. It is a theatrical entrance using the mappings you already made; it does not use a camera or recognize walls. Your media, individual FX and Dynamic FX remain assigned, and the entrance does not edit the saved project. Animation clocks continue behind the entrance so the revealed content is current.

### Dynamic FX

**Shipped in [0.8.1](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.1).** Open **File → Examples** for a three-surface demo, or click **Dynamic FX** and choose participating surfaces. **Hot-Ass Potato** explores their perimeters with a polygon snake, sheds fragments, performs a staged glitch/reboot, then settles into coordinated ambient geometry. **Potato Focus** adds holographic scanning bands and polygon networks. Speed, shape count, six colours and pause apply to the whole group; Replay intro and Skip to ambient control the snake sequence. Switch the effect to Off to restore the surfaces' original media or individual FX.

[0.8.2](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.2) reduces geometry preparation work, shares it between editor and projector, and reuses geometry and GPU buffers while paused. The animation look is retained; performance still depends on hardware and mesh complexity.

These effects use the mapped mesh and one shared clock for editor/projector. The snake visits nearby surfaces and uses the centre of their widest facing overlap, with its tail continuing across the handoff. Overlapping panels can share one crossing point. With no facing opening it falls back to the closest boundary; separate gaps stay dark. Routing uses the corner/projective outline, so heavily deformed outer mesh edges remain approximate. Cameras and automatic physical-surface recognition remain future work.

## Updating an existing copy

**From v0.2.0:** close the old app, extract the new ZIP, then copy the new package's contents into your existing app folder. Replace `PotatoMapper.exe` when asked and keep your mapping files and media. This one-time upgrade adds the launcher. You can also extract into a fresh folder and open existing mappings from there.

**From v0.3.0 onward:** use **Help → Check for updates**. Downloads show progress and can be cancelled. Before restarting, the app asks you to save/discard/cancel unsaved changes and stop any active projection. Your saved mapping reopens after installation. **Help → Use previous app version** switches back to the retained version; it does not undo edits to saved projects.

If an older copy says it was opened outside the portable launcher layout, save and close it, then open **PotatoMapper.exe in the main extracted folder** and check again. A shortcut should point to that file. Only extract another full app ZIP if the launcher files are missing. From v0.7.1, directly opening the inner editor also detects its enclosing portable folder.

Save personal files outside `versions`, `.updates` and `.maintenance`, which are managed app folders. The root launcher selects the current editor in `versions`; its name and your outer folder can remain **Potato Mapper** through updates. Updates require an internet connection and a writable app folder; mapping still works offline. There is no automatic update check during projection.

The 0.8.1 maintenance payload can refresh an older root launcher automatically and retains its previous executable under `.maintenance`. The launcher repairs a missing/invalid current pointer using a complete retained version; new runtimes also retain an inventory so a damaged selected runtime can fall back. A completely deleted launcher still needs a replacement from a full release ZIP. Keep the existing Projects folder and media when repairing an installation.

On normal startup, mappings immediately inside the app root or a version folder are copied with available media into **Projects**; originals stay in place. Opening an external mapping uses the same collection process. Edited managed copies are retained, collisions get separate names, and **File → Recent projects / Open Projects folder** make them accessible. Saves keep the previous mapping beside it as `.bak`. New media added later remains linked until the mapping is opened and collected again. Keep a separate backup of Projects: deleting the entire app folder also deletes projects stored there.

## Continue development

See the [roadmap checklist](ROADMAP.md) for completed features, sensible next steps, and where development stopped. Read [DEVELOPMENT.md](DEVELOPMENT.md) for the architecture and focused checks. [AGENTS.md](AGENTS.md) gives coding agents the same working conventions.

## License

[MIT](LICENSE). You can use, modify, and redistribute the project under those terms.

The potato logo was generated with AI. Bundled Qt, FFmpeg, and Microsoft runtime libraries have their own terms; see [runtime notices](packaging/THIRD-PARTY.txt). Library sources accompany the binary release.
