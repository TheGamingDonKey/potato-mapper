# Development handoff

## Stack and source map

C++17, Qt Widgets, Qt Multimedia, OpenGL 3.3 Core, and GLSL 330. CMake builds a native Windows desktop app. No server or account is required to run it.

| File | Responsibility |
| --- | --- |
| `src/main.cpp` | Application setup, toolbars, media controls, display selection, project actions, diagnostic CLI modes |
| `src/model.h`, `src/model.cpp` | Surfaces, JSON persistence, undo/redo snapshots, media decoding, animation timing |
| `src/canvas.h`, `src/canvas.cpp` | Shared editor/output renderer, shaders, mesh interaction, drag-and-drop |
| `assets/` | Potato logo, Windows icon and resource metadata |
| `build.ps1` | Configure, build Release, and deploy runtime DLLs locally |

The editor and projector use separate Canvas instances with the same Scene. Surface corners are normalized output coordinates. A projective transform maps the unit square to the four corners, and a triangulated mesh provides local deformation. Keep editor overlays out of the output Canvas.

Qt Multimedia decodes video through QVideoSink into QImage frames, which are uploaded as textures. This is a straightforward initial pipeline; it is not a fully GPU-native video decoder pipeline. Media sources are shared by path.

Dots are procedural fragment-shader output, with per-surface phase advanced by Scene's timer. Both canvases read the same phase. Animation frames emit repaint notifications without marking the project dirty. Speed, size, and pause are serialized; animation phase restarts on project load. Pattern `0` selects media/grid and `1` selects dots.

The white grid uses a transparent texture with mipmaps. QPainter overlays coexist with OpenGL through `beginNativePainting()` / `endNativePainting()` and explicit viewport restoration. Preserve that ordering when changing rendering.

## Build and focused checks

See the README for prerequisites. Close the running executable before building into `app` on Windows. Save the user's mapping first.

The dots integration check opens temporary editor/output windows, compares frames while moving and paused, and round-trips settings through a temporary project:

```powershell
$check = Start-Process .\app\PotatoMapper.exe -ArgumentList '--smoke-dots' -PassThru -Wait
$check.ExitCode # 0 means pass
Get-Content .\app\mapper.log -Tail 5
```

Existing general checks use `--smoke` (corner interaction, serialization, and rendering), optionally with `--media "C:\path\clip.mp4"`. These are diagnostic modes, not a comprehensive test suite. Do not run them against the user's actual project: they deliberately change their in-memory scene and use temporary project files.

To reopen a mapping normally:

```powershell
.\app\PotatoMapper.exe --project 'C:\path\mapping.pmap'
```

For visual work, run the app and inspect the changed behavior once. For output changes, check display selection and fullscreen output; distinguish an output-window test from testing a connected physical projector. Avoid repeated broad checks without a specific failure to investigate.

## Known limits and next work

- One projector output. Manual mapping only; no camera calibration or 3D object reconstruction.
- Dots currently use a fixed cyan/white palette and fixed lattice density. Other generator names discussed in chat have not been implemented.
- Ordinary media uses alpha blending. Screen/Add blending and background removal are not implemented. Black in an opaque video can obscure lower layers.
- Sidebar controls may require scrolling on shorter displays.
- Project files reference media; there is no pack-and-collect feature.
- Existing video controls act on sources shared by path. Review resource cleanup and decoder performance before scaling to many videos.
- No automated CI or packaged release exists yet. Runtime redistribution needs the corresponding dependency notices and license terms.

Keep changes small and runnable. Preserve backward compatibility with the existing version-1 JSON format and earlier `HomeMapper` format tag. A new feature should be reachable in the actual UI and render through the same output path.
