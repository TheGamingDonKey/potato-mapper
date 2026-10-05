# Development handoff

Read [ROADMAP.md](ROADMAP.md) for current progress and proposed work. Keep its checklist and release status current as described in [AGENTS.md](AGENTS.md).

## Stack and source map

C++17, Qt Widgets, Qt Multimedia, OpenGL 3.3 Core, and GLSL 330. CMake builds a native Windows desktop app. No server or account is required to run it.

| File | Responsibility |
| --- | --- |
| `src/main.cpp` | Application setup, toolbars, media controls, display selection, project actions, diagnostic CLI modes |
| `src/model.h`, `src/model.cpp` | Surfaces, JSON persistence, undo/redo snapshots, media decoding, animation timing |
| `src/canvas.h`, `src/canvas.cpp` | Shared editor/output renderer, mesh interaction, drag-and-drop |
| `assets/patterns.frag`, `src/patterns.h` | Shared GLSL image/video/pattern shader, stable pattern IDs and defaults |
| `src/updates.h`, `src/updates.cpp` | GitHub release checks, streamed download, verification, extraction and restart handoff |
| `src/launcher.cpp` | Small Windows launcher, version selection, update installation, rollback and startup recovery |
| `src/branding.h` | Startup splash drawn with the existing potato logo |
| `src/ui.h`, `src/style.h` | Collapsible inspector sections, filename elision and the shared widget theme |
| `assets/` | Potato logo, Windows icon and resource metadata |
| `build.ps1` | Configure, build Release, and deploy runtime DLLs locally |
| `package.ps1` | Package the built executable and runtime dependencies into a portable Windows x64 ZIP |
| `packaging/` | Quick-start instructions and runtime license notices |

The editor and projector use separate Canvas instances with the same Scene. Surface corners are normalized output coordinates. A projective transform maps the unit square to the four corners, and a triangulated mesh provides local deformation. Keep editor overlays out of the output Canvas.

Qt Multimedia decodes video through QVideoSink. Common unrotated SDR NV12 frames (BT.709, or BT.601 video range) retain one mapped frame per source. The renderer uploads the Y and UV planes and converts to RGB in its shader, avoiding a full RGB image conversion on the UI thread. Row strides and colour range are respected. Other formats, rotated/cropped frames, HDR and full-range BT.601 use Qt's image conversion fallback. This is not zero-copy decoding: native frame mapping and texture uploads still cost time. Media sources are shared by path.

Dots are procedural fragment-shader output, with per-surface phase advanced by Scene's timer. Both canvases read the same phase. Animation frames emit repaint notifications without marking the project dirty. Speed, size, and pause are serialized; animation phase restarts on project load. Pattern `0` selects media/grid and `1` selects dots.

The white grid uses a transparent texture with mipmaps. QPainter overlays coexist with OpenGL through `beginNativePainting()` / `endNativePainting()` and explicit viewport restoration. Preserve that ordering when changing rendering.

The v0.4.0 increment stores per-surface `brightness` and `opacity` as integer percentages (0–100). Earlier projects default to 100 for both. The shared shader scales RGB and alpha after image/video sampling, and applies the same factors to procedural dots. Separate RGB/alpha blend factors retain an opaque canvas for Qt's final composition. `Scene::setAppearance` supports one undo record per slider drag; selecting a surface or repainting an animation does not change these settings. Earlier apps ignore the new appearance settings; saving in an earlier app does not retain them.

The sidebar keeps the surface list above a scrolling inspector. Media, Appearance, Potato FX and Mapping are collapsible sections. Mapping and Potato FX begin collapsed; selecting dots reveals their settings. Potato FX offers media/grid plus dots, diagonal stripes, expanding rings and square waves. `src/patterns.h` holds the stable names/IDs (0–4); dots retain ID 1. Each generator uses speed, size/line width and pause, with its own fixed palette. All render inside the existing mesh and share the scene timer. Phase remains transient; ticking does not dirty a project or add Undo records.


The v0.5.0 increment stores `blend` (0 Normal, 1 Screen, 2 Add), defaulting to Normal for earlier mappings. Normal uses source-alpha composition, Add uses alpha-weighted addition and Screen uses `source * alpha + destination * (1 - source * alpha)` per RGB channel. The shared shader premultiplies only Screen RGB after applying brightness/opacity; separate alpha factors retain the opaque Qt canvas. Blend factors are selected for every surface so mixed layer modes work in order. `Scene::setBlend` and `resetAppearance` record Undo; reset restores 100% brightness/opacity and Normal in one operation. Screen/Add apply to images, native NV12 video and procedural patterns. They are layer blend modes, not multi-projector edge blending.

`--smoke-fx` checks the actual blend/pattern controls, numerical overlap pixels in editor/output at 0/50/100% opacity, image alpha, black overlays, return to Normal, reset/Undo, native NV12 Screen, old-format defaults, generator motion/pause/width, and saved settings. `--preview <temporary.png>` captures the four new cell effects as a contact sheet; `--snapshot <temporary.png>` captures the editor. Files and scenes are disposable.

## Build and focused checks

See the README for prerequisites. Builds deploy into fresh folders under `out`; they do not replace a running app or saved mappings. Always use the root launcher, `PotatoMapper.exe`, when trying updates. Its editor and libraries live under `versions/<version>`.

The dots integration check opens temporary editor/output windows, compares frames while moving and paused, and round-trips settings through a temporary project:

```powershell
$check = Start-Process .\app\PotatoMapper.exe -ArgumentList '--smoke-dots' -PassThru -Wait
$check.ExitCode # 0 means pass
Get-Content .\app\mapper.log -Tail 5
```

Existing general checks use `--smoke` (corner interaction, serialization, and rendering), optionally with `--media "C:\path\clip.mp4"`. These are diagnostic modes, not a comprehensive test suite. Do not run them against the user's actual project: they deliberately change their in-memory scene and use temporary project files.

`--smoke-video-colour` checks native NV12 rendering against Qt's converted colours, padded row strides, colour ranges, and switching back to an RGB image. `--profile-media "C:\path\one.mp4" "C:\path\two.mp4" "C:\path\three.mp4"` runs a separate temporary scene and output window, then logs per-source frame rate, frame preparation time, and output repaint rate after warmup. It does not save or change the user's mapping.

`--smoke-appearance` exercises the actual sliders and Potato FX picker, one Undo for a continuous slider drag, Redo, saved settings, earlier HomeMapper defaults, overlapping layers, image/dots/native-NV12 pixels and matching editor/output pixels. It also checks the compact sidebar and filename handling. `--snapshot <temporary.png>` captures the real editor for visual inspection. These checks use temporary synthetic projects/media.

On the original Radeon 840M development PC, three 1280x720 Grok clips in that profiling window improved from about 12 to 27 output repaints per second after the NV12 change. Frame preparation fell from about 8.4 ms to 1.6-2.0 ms per frame. These are local measurements, not a guaranteed frame rate for other hardware, codecs, or projector configurations.

To reopen a mapping normally:

```powershell
.\app\PotatoMapper.exe --project 'C:\path\mapping.pmap'
```

For visual work, run the app and inspect the changed behavior once. For output changes, check display selection and fullscreen output; distinguish an output-window test from testing a connected physical projector. Avoid repeated broad checks without a specific failure to investigate.

### Desktop/update checks

`--smoke-clear` checks the actual Clear media button, retained geometry, another surface, Undo/Redo, saved state, and original-file preservation. `--test-restart` exercises the real unsaved-work Cancel/Save prompts and active-output cancellation. `--test-splash --snapshot <temporary.png>` captures the actual splash widget; `QT_SCALE_FACTOR=1.5` allows a focused scaling check without changing Windows display settings.

The optional `PotatoUpdateChecks` build target runs the actual update dialog against deterministic release responses and the real release ZIP/extractor. Supply `<release.zip> <extracted-package> --install-root <new-disposable-folder>`. It checks the packaged manifest, missing payload/digest handling, an interrupted download, cancelled restart, unchanged personal files, and staging cleanup. With `--handoff`, it uses the real launcher and editor process lock, exits after preparation, and reopens a sample mapping in the installed editor. Close that disposable editor afterwards. The fixture network is injected only by this test executable; the application's Help command always uses the public repository endpoint.

The check reads the target version from the package and currently simulates an installed v0.5.0 editor. `--launcher <path>` lets the handoff use a retained older launcher. The v0.4.0 rehearsal used the actual v0.3.0 launcher and retained runtime; the v0.4.0 editor reopened the saved fixture with unchanged project/media bytes. The v0.5.0 rehearsal likewise used the actual v0.4.0 launcher, reopened the saved mapping and retained its files/runtime. These were local disposable installations; this update on Shane's other laptop remains untested.

Run `pwsh ./tests/launcher.ps1 -PackageDirectory <extracted-package>` for a packaged upgrade, rollback, repeat update after rollback, incomplete installation, and startup-failure recovery. These checks use a disposable installation under `out` and synthetic personal files; the retained old-version sentinel is not claimed as a run of an older editor. The current runtime is installed and launched for real, including reopening the sample mapping after recovery. The script requires PowerShell 7 for `ProcessStartInfo.ArgumentList`.

The desktop changes were checked locally on Windows 11, including the real GitHub update dialog and packaged mapping/dots/video-colour checks. Real projector hardware and a subsequent update on the user's other computer remain untested.

### Update layout and ownership

Layout protocol 1 is marked by `installation.txt` (`PotatoMapper/1` followed by LF). `current.txt` names the selected numeric version; `previous.txt` retains the prior selection. Both point into `versions`. `update-manifest.json` contains the format, protocol, version and SHA256 of every runtime file. GitHub's release-asset digest is checked before extraction. The helper waits for the editor PID and runtime lock, installs into a new version folder, switches pointers and reopens the saved project. Preparation failures leave the selection unchanged; caught selection/startup failures restore both original pointers. Owned staging is cleaned on success and caught failure. Retained version folders are reused only if they match the verified download byte for byte.

The launcher is statically linked to the C++ runtime and does not use Qt. Ordinary updates never replace the running launcher. A different layout protocol requires a manual full-package upgrade. The Windows updater uses system PowerShell for ZIP extraction; see `packaging/extract-update.ps1`. Projects/media stay outside `versions` and `.updates`; no updater operation scans or deletes personal files elsewhere. Crashing or losing power during download may leave a staging folder, and old version folders are retained rather than automatically pruned.

Release packaging is always fresh. Do not zip the developer's `app` folder or include private mappings, logs or media. The same Qt 6.10.3/FFmpeg 7.1.3 library sources remain available with v0.2.0 and are linked by the notices. See [desktop implementation plan](docs/desktop-update-plan.md).

## Known limits and next work

- One projector output. Manual mapping only; no camera calibration or 3D object reconstruction.
- The four flowing effects are original procedural interpretations of the inspected references, not exact replicas. Patterns remain clipped to their mapped surface. There is no shared world-coordinate pattern alignment between surfaces.
- Screen/Add blending can overlay black-background media; it does not remove arbitrary video backgrounds. Normal remains the default.
- The inspector may require scrolling on shorter displays; the surface list remains accessible above it.
- Project files reference media; there is no pack-and-collect feature.
- Existing video controls act on sources shared by path. Review resource cleanup and decoder performance before scaling to many videos.
- Releases contain a portable Windows x64 ZIP with a manual GitHub update command. There is no installer or automated CI yet. When updating runtime libraries, update their notices and publish matching sources alongside the ZIP. Retaining earlier runtimes consumes disk space; there is no version-cleanup UI yet.

Keep changes small and runnable. Preserve backward compatibility with the existing version-1 JSON format and earlier `HomeMapper` format tag. A new feature should be reachable in the actual UI and render through the same output path.

## Flowing Potato FX (v0.6.0, implemented pending release)

Stable IDs 0–4 retain their generators; ID 4 is labelled Stepped waves to distinguish it from the new cell-based SquareWave. IDs 5–8 are SquareWave, Diagonals, CubicCircles and SquareArray. New effects default to white, Flow 70%, no rotation/reverse/edge fade and densities 48/32/24/18. New cell sizes default to 38%, with Diagonals at 12% line width. Density and size are separate controls.

The shader combines continuous sine/cosine fields and coordinate warping to change neighbouring cells together. SquareWave stretches narrow cells; Diagonals changes stroke orientation; CubicCircles morphs square/circle/ring distance fields; SquareArray shifts rows while cell sizes change. Derivative-based antialiasing smooths small shapes. No added runtime library, video file or random reseeding is involved. Optional Soft edge fades the surface boundary and defaults to zero.

Each surface serializes `fxDensity`, `fxFlow`, `fxAngle`, `fxPalette`, `fxEdge` and `fxReverse`. Setters clamp values and record Undo. Older projects default to their original spacing/palettes, angle zero and no fade/reverse. Phase remains transient and shared by editor/output. Saving a new mapping in an older app can discard these settings and new pattern IDs.

`--smoke-fx` additionally checks control scope/Undo, saved look settings and old-pattern defaults, unchanged paused pixels when reversing, and timed backward motion. Direction applies to future Scene timer increments, preserving the current phase. `--test-fx-motion --frames <temporary-directory>` exports 48 actual output frames at 0.12-second phase steps, with all four cell effects mapped into quadrants. This is a deterministic visual preview, not a frame-rate benchmark. The shaders were inspected at multiple phases against the open MadMapper reference. Physical projection remains a user check.

Technique references: [procedural grids and transforms](https://thebookofshaders.com/09/) and [continuous wave modulation](https://thebookofshaders.com/13/). These informed original implementation code; no third-party effect shader was copied.
