# Development handoff

Read [ROADMAP.md](ROADMAP.md) for current progress and proposed work. Keep its checklist and release status current as described in [AGENTS.md](AGENTS.md).

## Reliability lessons across projects

Continuity handoff, October 7, 2026. Apply these practices incrementally; support and recovery evidence must be checked in each project's own repository.

- **Locate the real files.** Moving a chat into a project does not move its source, tools, executable or data. Check the actual checkout and preserve the original until a relocated copy builds and its extracted distribution runs.
- **Check the final package.** Compilation can succeed while a package omits UI assets or dependencies. Run the extracted final distribution and exercise actual restart/recovery paths before announcing readiness.
- **Build before profiling.** Wait for a successful build to exit before starting the executable or profiler. Record which binary was measured; overlapping build/run steps can lock files or measure stale code.
- **Keep regressions for real failures.** Geometry caches must distinguish accepted duplicate object IDs and invalidate when relevant state changes. Retain focused coverage for those failures, paused reuse and editor/output agreement.
- **Rehearse the updater handoff.** Before publishing an updater release, use the actual old launcher to install the final runtime and reopen the same managed project. Verify original project/media bytes, the previous runtime and launcher backup remain available. Record failed/interrupted recovery checks and their limits separately from successful installation.
- **Separate application files from user data.** Retain originals during migration and restore backups into a new copy. Never put live company databases into source, release packages, OneDrive code folders or messages. Document protection and tested recovery; promise only what the evidence supports.
- **Keep business-data requirements explicit.** Accounting tools require exact money, atomic and idempotent balanced posting, client isolation, immutable posted records, reasoned corrections and validated snapshots. The accounting handoff reports manual backups; automatic backups, a safe schema-migration updater, cloud recovery, tax compliance and production approval remain unimplemented. These are accounting requirements and limits, not mapper features.

Keep progress and recovery evidence in project docs so another developer can resume. Automatic accounting version checks/updates and its public website remain future work; this handoff authorizes documentation only.

## Stack and source map

C++17, Qt Widgets, Qt Multimedia, OpenGL 3.3 Core, and GLSL 330. CMake builds a native Windows desktop app. No server or account is required to run it.

| File | Responsibility |
| --- | --- |
| `src/main.cpp` | Application setup, toolbars, media controls, display selection, project actions, diagnostic CLI modes |
| `src/model.h`, `src/model.cpp` | Surfaces, JSON persistence, undo/redo snapshots, media decoding, animation timing |
| `src/canvas.h`, `src/canvas.cpp` | Shared editor/output renderer, mesh interaction, drag-and-drop |
| `assets/patterns.frag`, `src/patterns.h` | Shared GLSL image/video/pattern shader, stable pattern IDs and defaults |
| `assets/entrance.frag`, `src/entrance.h` | Potato Scan acquisition/tracing/grid/lock shader and transient reveal timing |
| `src/updates.h`, `src/updates.cpp` | GitHub release checks, streamed download, verification, extraction and restart handoff |
| `src/launcher.cpp` | Small Windows launcher, version selection, update installation, rollback and startup recovery |
| `src/launcher-files.h`, `src/launcher-refresh.cpp/.h` | Retained runtime inventory, native root launcher refresh and replacement backup |
| `src/projects.cpp/.h` | Managed portable project copies, media collection, recent paths and previous-save backups |
| `src/dynamic.cpp/.h`, `src/dynamic-ui.h` | Scene-wide procedural geometry, route/stages and Dynamic FX controls |
| `src/dynamic-metric.h` | Output-pixel mapping/inverse metrics for snake/cells and stroke thickness |
| `src/mesh-geometry.h` | Clips Dynamic FX triangles to mesh cells before mapping their vertices |
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

The sidebar keeps the surface list above a scrolling inspector. Media, Appearance, Potato FX and Mapping are collapsible sections. Mapping and Potato FX begin collapsed; selecting dots reveals their settings. Potato FX offers media/grid plus thirteen animations; the flowing effects and controls are described below. `src/patterns.h` holds the stable names/IDs (0–13); dots retain ID 1. Each generator uses speed, size/line width and pause. All render inside the existing mesh and share the scene timer. Phase remains transient; ticking does not dirty a project or add Undo records.


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

### Coordinated effects and managed projects (0.8.1)

Dynamic settings are appended to the version-1 project as `dynamic`; earlier apps ignore them and lose those settings if they save. Scene advances one transient group clock, builds one bounded primitive frame and shares it between canvases. The renderer batches glowing polygon geometry per surface, clips triangles to the existing mesh, and respects brightness, opacity and blend mode. Polygon generation is capped at 500 primitives. Routing uses sampled projective corner boundaries, so nearest entry on a heavily bent outer mesh remains approximate. Removing/hiding a participant restarts the intro; geometry changes and unrelated Undo preserve its clock. Loading a project restarts the intro.

Hot-Ass Potato stages are Explore, Overload, Reboot and Ambient. For three surfaces the intro is about 23.5 seconds at 100% speed. The glitch is an animation, followed by a glowing Potato Mapper title and tile assembly; it does not crash the program. Focus is an original scanning/polygon effect. Neither effect requires media or camera access. Examples are shipped inside the versioned runtime so protocol-1 updates deliver them.

`PotatoDynamicChecks` exercises 23 geometry/settings assertions; `PotatoProjectChecks` exercises 26 temporary-fixture assertions. `--smoke-dynamic` covers actual controls, shared pause, motion pixels, clean timer ticks, persistence, membership, Undo clock continuity, failed-load scene preservation and a scan following an interior mesh deformation. `--test-dynamic-motion --frames <folder>` exports a short real-renderer preview; `--snapshot <file>` captures controls. `--check-project <path>` reads a mapping without collection or saving. The existing local `My mapping.hmap` loaded with five surfaces; Shane's separate File → Open failure needs its file/message before a specific cause can be established.

Short simultaneous 1920×1080 output and 960×540 preview measurements for 0.8.1 on the original Radeon 840M: grid 21.6/25.0, Hot-Ass Potato 22.2/18.3 and Focus 18.7/20.1 output repaints/s for one/four overlapping surfaces. Editor rates matched. These are repaint counts over two-second samples, not promised performance. Four overlapping surfaces are a stress scene. The 0.8.2 optimization below addresses duplicate triangulation.

Projects collection uses a locked atomic index, unique destination folders and relative media paths. Source mappings/media are retained. Unchanged imports reuse their edited managed copy; newly referenced external media is repacked while retaining that copy. Default Save writes to Projects and keeps a `.bak`; Save itself does not collect newly added media. Missing media can still open for manual Load media repair. Structural validation runs before Scene mutation. Failed collection keeps the original valid mapping open and reports the path.

Launcher maintenance stays inside the detected installation. Protocol 1 includes `PotatoLauncherRefresh.exe`, the replacement launcher and its digest inside `versions/<version>`. The selected editor requests detached maintenance; older directly opened runtimes cannot replace a newer selected launcher. The helper uses the update lock, a flushed staged copy, atomic replacement, bounded retry and a previous executable backup. A damaged inventoried current runtime falls back to a verified previous version; older runtimes have weaker completeness checks because they lack inventories. A missing root launcher can be restored by a working inner editor started with an explicit `--install-root`; without a runnable entry point, obtain the full ZIP and retain personal data.

### Dynamic FX performance (0.8.2)

Scene now lazily caches one packed geometry frame, invalidated by animation/settings/mesh updates. Both canvases use its per-surface ranges; ranges are indexed by surface instance order so accepted duplicate IDs cannot overwrite another mesh. Each OpenGL context uploads the combined frame once per revision. Paused redraws retain CPU geometry and GPU buffers. A regular mesh maps directly; deformed meshes keep the previous clipping/interpolation with stack arrays (triangle intersection is bounded) and callback vertex emission. Hidden individual patterns continue their phase but do not repaint a paused covering Dynamic FX group.

`--profile-fx --dynamic-effect 1|2 --profile-cells 16 --profile-density 80` stresses one/four overlapping surfaces; add `--profile-warp` for deformed interiors. Reports include CPU meshing per window, asynchronous `GL_TIME_ELAPSED` native-pass intervals and an 8 ms event-loop heartbeat's lateness. Geometry preparation runs before the native query. Queries never block for unavailable results and exist only in profiling mode. The query excludes QPainter text/overlays and Qt/Windows final composition, so it is not a total GPU-usage measurement. Two-second repaint samples and heartbeat delays are local diagnostics, not guaranteed FPS or measured mouse latency.

Baseline dense regular meshes (16 cells, 80 shapes, four surfaces) used 48.0 ms combined meshing for Hot-Ass Potato and 37.7 ms for Focus, at 10.9/12.6 repaints/s. The final package build used 2.09/2.10 ms at 22.8/17.1 repaints/s. An earlier optimized sample gave 23.8/22.0, illustrating variation in short desktop measurements; the geometry-cost reduction is more consistent than total repaint rate. Further stack clipping reduced deformed-group preparation from 24.1/18.7 ms with the initial cache to 9.95/7.68 ms; final warped rates were 17.4/21.7 repaints/s. Final native GPU intervals were about 0.2 ms per output pass in that warped stress scene, while windows/driver composition still constrained total rate.

The actual-renderer check covers paused geometry and buffer reuse, hidden-FX idle behavior, shared motion, persistence, failed loads, deformed interior scans and duplicate-ID meshes. Cache and duplicate-ID regressions failed before fixes and passed after them. Eight baseline-vs-optimized output phases retained appearance within negligible edge rounding; the largest mean channel difference was 0.00073/255 and fewer than 0.002% of pixels changed by more than three levels. `--test-dynamic-motion --reference-frames --frames <folder>` exports those eight sample phases. Temporary baseline binaries/frames are local ignored artifacts and can be removed after comparison.

Focused source review found no remaining blocking issues. The final extracted 0.8.2 ZIP passed `--smoke-dynamic`, `--smoke-fx` and `--smoke` with only Windows system directories on PATH. A real 0.8.1 launcher installed it, refreshed the root launcher, reopened the managed mapping and retained the original project/media bytes, prior runtime and launcher backup. SHA256: `36c529a89e406f1c75276c073d1ed3a430738b6a48fa62ab629ff9d943a804f3`. Physical projector and other-laptop performance remain user checks.

Merged in [PR #13](https://github.com/TheGamingDonKey/potato-mapper/pull/13), release commit `ec390a9f2cc58077b222948a13824890f7e16fa2`, and published as [v0.8.2](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.2). Public latest metadata and the canonical HTTP 200 download match the final package's 27,049,169 bytes and digest. Existing personal mappings/media and the previous local package remain in place.

### Snake connections (shipped in 0.8.3)

The nearest-neighbour tour previously chose the first equally close sampled boundary pair. Aligned surfaces therefore connected at corners despite a whole facing edge being available. For each chosen neighbour, facing edges now project onto a common tangent; the widest shared interval supplies centred portals. If the centre between those portals maps inside both overlapping surfaces, both use that same point. With no usable facing span, retain the closest-boundary fallback. Geometry edits invalidate the existing cached tour; phase changes do not rerun the search.

Portal bends use a common output direction converted into each surface's UV coordinates. The lateral wave's endpoint derivative is zero. Trailing segments can occupy the previous stop while the head moves onto the next; internal handoffs keep full opacity, while tour beginnings/endings retain their fade. This applies to exploration and ambient passes. Geometry remains bounded, separate gaps stay dark, and project settings/format are unchanged. Routing still uses the corner/projective outline; heavily deformed outer mesh edges remain approximate.

Review caught independent UV-axis clipping changing the direction near an oblique corner. `dynamic-geometry.h` now shortens the entire ray to its first square boundary. The independent mapped-heading regression failed before and passed after correction; focused review found no remaining blockers. Nested/concentric panels have no facing opening and retain the documented fallback.

`--test-dynamic-motion --portal-frames --frames <folder>` captures 24 actual output frames around a touching, partially aligned two-surface handoff. The runtime check also samples brightness at that shared opening immediately before and after the crossing.

Release build, geometry/settings checks, actual rendered handoff and extracted `--smoke-dynamic`/`--smoke` passed. The real 0.8.2 launcher installed the final 0.8.3 ZIP, refreshed itself and reopened a managed project, retaining original project/media bytes, the old runtime and launcher backup. SHA256: `83e2bc3a1abac3db9c8b99792593a859154020e2760c4cae358523b10ecd6598`.

A same-session dense four-surface profile (16 cells, density 80) measured 2.281 ms CPU preparation and 0.185 ms native GPU time for this build, versus 2.289/0.177 ms for the previous 0.8.2 binary. Repaints were 12.0 versus 12.3/s with another demo editor still open; these short samples do not establish guaranteed FPS or explain window-composition limits. The routing search is cached until geometry changes, rather than repeated each animation frame.

Merged in [PR #14](https://github.com/TheGamingDonKey/potato-mapper/pull/14), release commit `247560fc0b6f40e4c998d59ad3024513720e2cd0`, and published as [v0.8.3](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.3). Public latest metadata and canonical HTTP 200 download match the 27,053,718-byte checked package and digest. Personal files and previous local packages remain in place.

### Desktop/update checks

`--smoke-clear` checks the actual Clear media button, retained geometry, another surface, Undo/Redo, saved state, and original-file preservation. `--test-restart` exercises the real unsaved-work Cancel/Save prompts and active-output cancellation. `--test-splash --snapshot <temporary.png>` captures the actual splash widget; `QT_SCALE_FACTOR=1.5` allows a focused scaling check without changing Windows display settings.

The optional `PotatoUpdateChecks` build target runs the actual update dialog against deterministic release responses and the real release ZIP/extractor. Supply `<release.zip> <extracted-package> --install-root <new-disposable-folder>`. It checks the packaged manifest, missing payload/digest handling, an interrupted download, cancelled restart, unchanged personal files, and staging cleanup. With `--handoff`, it uses the real launcher and editor process lock, exits after preparation, and reopens a sample mapping in the installed editor. Close that disposable editor afterwards. The fixture network is injected only by this test executable; the application's Help command always uses the public repository endpoint.

The check reads the target version from the package and currently simulates an installed v0.6.0 editor. `--launcher <path>` lets the handoff use a retained older launcher. The v0.4.0 rehearsal used the actual v0.3.0 launcher and retained runtime; the v0.4.0 editor reopened the saved fixture with unchanged project/media bytes. The v0.5.0 rehearsal likewise used the actual v0.4.0 launcher, reopened the saved mapping and retained its files/runtime. These were local disposable installations; this update on Shane's other laptop remains untested.

Run `pwsh ./tests/launcher.ps1 -PackageDirectory <extracted-package>` for a packaged upgrade, rollback, repeat update after rollback, incomplete installation, and startup-failure recovery. These checks use a disposable installation under `out` and synthetic personal files; the retained old-version sentinel is not claimed as a run of an older editor. The current runtime is installed and launched for real, including reopening the sample mapping after recovery. The script requires PowerShell 7 for `ProcessStartInfo.ArgumentList`.

The desktop changes were checked locally on Windows 11, including the real GitHub update dialog and packaged mapping/dots/video-colour checks. Real projector hardware and a subsequent update on the user's other computer remain untested.

### Update layout and ownership

Layout protocol 1 is marked by `installation.txt` (`PotatoMapper/1` followed by LF). `current.txt` names the selected numeric version; `previous.txt` retains the prior selection. Both point into `versions`. `update-manifest.json` contains the format, protocol, version and SHA256 of every runtime file. GitHub's release-asset digest is checked before extraction. The helper waits for the editor PID and runtime lock, installs into a new version folder, switches pointers and reopens the saved project. Preparation failures leave the selection unchanged; caught selection/startup failures restore both original pointers. Owned staging is cleaned on success and caught failure. Retained version folders are reused only if they match the verified download byte for byte.

In v0.7.1, a direct editor launch under `versions/<numeric-version>` also finds the enclosing installation by its exact marker and launcher. Previously, startup without `--install-root` used the editor's own directory, so updates incorrectly reported an outside-layout error despite an intact package. Explicit root arguments remain authoritative; unrelated ancestors and incomplete packages are not inferred. `PotatoUpdateChecks unused unused --root-resolution-only` checks this regression without downloading/extracting an update.

The regression failed before the fix and passed after it. Direct packaged startup acquired the outer runtime lock; direct packaged rendering passed without developer libraries on PATH. The actual nested updater without an explicit root offered installation and passed extraction/interruption/cancellation checks. The v0.6.0 launcher installed the final v0.7.1 ZIP and reopened the fixture, retaining project/media bytes and the old runtime. Source review found no blockers. [PR #10](https://github.com/TheGamingDonKey/potato-mapper/pull/10) is merged and [v0.7.1](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.7.1) is published; public metadata/download size/digest match SHA256 `0dd92da42f005cba5abe6822deeb02abdd340eef8a86327144557f8983fa3925`. On October 5, Chase inspected Predator's intact installation and directly launched older editors, then verified a fresh root-launcher start offered v0.7.1. Shane installed it, and Chase verified the updated copy reported up to date. Direct editor discovery was not separately exercised on Predator.

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

## Flowing Potato FX (shipped in v0.6.0)

Stable IDs 0–4 retain their generators; ID 4 is labelled Stepped waves to distinguish it from the new cell-based SquareWave. IDs 5–8 are SquareWave, Diagonals, CubicCircles and SquareArray. New effects default to white, Flow 70%, no rotation/reverse/edge fade and densities 48/32/24/18. New cell sizes default to 38%, with Diagonals at 12% line width. Density and size are separate controls.

The shader combines continuous sine/cosine fields and coordinate warping to change neighbouring cells together. SquareWave stretches narrow cells; Diagonals changes stroke orientation; CubicCircles morphs square/circle/ring distance fields; SquareArray shifts rows while cell sizes change. Derivative-based antialiasing smooths small shapes. No added runtime library, video file or random reseeding is involved. Optional Soft edge fades the surface boundary and defaults to zero.

Each surface serializes `fxDensity`, `fxFlow`, `fxAngle`, `fxPalette`, `fxEdge` and `fxReverse`. Setters clamp values and record Undo. Older projects default to their original spacing/palettes, angle zero and no fade/reverse. Phase remains transient and shared by editor/output. Saving a new mapping in an older app can discard these settings and new pattern IDs.

`--smoke-fx` additionally checks control scope/Undo, saved look settings and old-pattern defaults, unchanged paused pixels when reversing, and timed backward motion. Direction applies to future Scene timer increments, preserving the current phase. `--test-fx-motion --frames <temporary-directory>` exports 48 actual output frames at 0.12-second phase steps, with all four cell effects mapped into quadrants. This is a deterministic visual preview, not a frame-rate benchmark. The shaders were inspected at multiple phases against the open MadMapper reference. Physical projection remains a user check.

Technique references: [procedural grids and transforms](https://thebookofshaders.com/09/) and [continuous wave modulation](https://thebookofshaders.com/13/). These informed original implementation code; no third-party effect shader was copied.

The v0.6.0 release is published from PR #8. The final portable FX and real MP4 checks passed without developer libraries on PATH. The v0.5.0 launcher installed the final ZIP and reopened the fixture mapping, preserving project/media bytes and the old runtime. Source review completed; Reverse continuity was fixed and verified. Published SHA256: `e240ba0e9b04401e40f4b9f69be7191de50f28c23586429ed8fe7d338eefdfe0`. The developer app and personal mappings/media were left in place.

## Organic and depth collection (shipped in v0.7.0)

Stable IDs 9–12 are Waterlight, Contour Flow, Curve Maze and Depth Tunnel. The earlier generators/settings remain unchanged. All four reuse speed, line width, density, flow, angle, palette, reverse, optional edge fade, brightness, opacity and blending; JSON version 1 is retained. No new runtime dependency. Defaults are White, density 30/18/24/18 and line width 14/12/14/12 percent.

Waterlight uses warped interference ridges and a local light halo to suggest water caustics; it does not simulate fluid or ray-trace light. Contour Flow samples two fixed scales of smoothly interpolated noise. Curve Maze pairs quarter-circles at tile-side midpoints and warps the complete coordinate field, preserving joined paths. Depth Tunnel combines logarithmic rings and angular spokes with distance fading near the vanishing point; it is a 2D perspective illusion, not reconstructed 3D geometry.

`--test-fx-motion --first-pattern 9 --frames <temporary-directory>` renders the new collection. `--smoke-fx --preview <temporary.png>` captures the newest four patterns while testing every registered generator. All frame export is diagnostic-only.

`--profile-fx` measures actual output/editor repaint rates for patterns 5 onward, with one full-screen surface and four overlapping surfaces. Output is 1920×1080 physical pixels; a separate editor Canvas is 960×540, visible over part of the output on this single-monitor PC. There is 0.4-second warmup and a 2-second measurement per case. It fails if either renderer cannot initialize graphics or stops repainting; invalid pattern ranges exit with code 2. The first output-only measurements had an occluded editor and cannot establish combined performance. These are short desktop measurements, not GPU timestamps, physical-projector measurements or a long-duration benchmark.

The corrected October 5 measurement on the Radeon 840M development PC gave the new effects approximately 19–21 output/editor repaints per second, including four overlapping layers. Forced repaint of the plain grid gave 21.4 (one layer) and 20.1 (four); output-only runs were around 60 with the editor inactive. The similar grid/effect rates suggest that presentation or shared rendering overhead is the primary limit in this setup, but that cause is not proven by GPU timing. Earlier effects varied approximately 13–21 in the short same-session sweep. Further work should investigate simultaneous preview/output rendering rather than claim these effects guarantee 60 FPS. Use `--first-pattern <id> --last-pattern <id>` to profile a subset; pattern 0 explicitly requests grid repaints because static scenes normally do not animate.

The updater's public HTTPS GET goes to GitHub's latest-release endpoint and then the release-asset URL. There is no owner login or credential in the app request, and no project upload. Publishing uses developer credentials separately. The existing native launcher switches version folders and reopens the saved mapping.

The inherited renderer wraps its phase at 10,000 units. Effects are not all periodic at that boundary and can visibly jump after about 2.8 hours at speed 100%, or 56 minutes at 300%. Removing that wrap requires considering shader float precision during long runs. This release's short motion checks do not establish multi-hour continuity; a bounded animation-clock improvement is follow-up work.

Merged in [PR #9](https://github.com/TheGamingDonKey/potato-mapper/pull/9) and published as [v0.7.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.7.0). Focused source review found no blocking issues; diagnostic failure handling was corrected and invalid-range/valid-grid profiling checked. Final portable FX and real MP4 playback passed with only Windows system directories on PATH. The actual v0.6.0 launcher installed the final ZIP and reopened the disposable saved mapping, retaining the project/media bytes and old runtime. Extraction, interrupted download and cancelled restart also passed. Public latest-release metadata, canonical HTTP 200 download, size and SHA256 match the final package: `050e613d9cc11f797c8388a7dcd95e06a0d56e891026aec2437b4956d61455e5`. The existing developer app and personal files remain in place.

## Hex Tide (shipped in v0.8.0)

Hex Tide appends stable ID 13 without changing earlier IDs or project format 1. Two staggered centre lattices produce regular hexagons, kept inside their cells so selecting the nearest centre happens in the empty gaps. Two broad travelling harmonics coordinate size and brightness; a small continuous warp bends the lattice gently. It uses the existing GPU shader and Scene clock, with no additional library or media decoder. Defaults: White, density 36, cell size 38%, flow 70%, speed 100% and no edge fade. Existing angle, reverse, pause, palette, appearance and blend controls apply.

`--smoke-fx` failed first when Hex Tide was absent, then passed after implementation. The additional checks exercise selection, distinct output from Depth Tunnel, nearby-time continuity and saving/reopening; the existing generator loop covers size, motion in editor/output, pause, blending and clean animation ticks. `--test-fx-motion --first-pattern 13 --frames <temporary-directory>` now permits a single final generator and captures it across the full preview. This remains a short visual check; the inherited long-run phase-wrap limitation above is unchanged.

After simplifying the pulse calculation, the October 6 short profile on the Radeon 840M PC measured Hex Tide at 26.3/26.0 output/editor repaints per second for one/four overlapping layers, versus 26.4/26.6 for the plain grid in the same session. Output was 1920×1080 and preview 960×540. These are desktop repaint counts, not a guaranteed frame rate, a projector test or a long-duration measurement.

The final extracted v0.8.0 package passed `--smoke-fx` with only Windows system directories on PATH. Its actual ZIP was installed by a retained v0.6.0 native launcher and reopened the saved disposable mapping, with project/media bytes and the previous runtime unchanged. Interrupted-download and cancelled-restart checks passed on the first package before the shader-only optimization. The final ZIP SHA256 is `bda5985f3f473c77fcb960df9d76387250b8935853492a3daa97aa157583531b`. Focused source review found no blocking issues, including the optimized wave calculation.

Merged in [PR #11](https://github.com/TheGamingDonKey/potato-mapper/pull/11) and published as [v0.8.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.0). Public latest-release metadata, canonical HTTP 200 download, size and digest match the final ZIP. The existing developer app and personal mappings/media were left in place.

### Potato Scan entrance (0.8.4)

Potato Scan is an eight-second theatrical entrance using saved surface geometry, with no camera, recognition service or extra library. Scene owns one transient clock shared by both canvases. Stages are acquisition, edge trace, grid assembly, panel lock and a quintic-eased reveal beginning at 5.6 seconds. Replay resets only this clock. Skip, output hide, successful project load and New end it; failed loads retain the active scene. Blackout pauses the entrance. Media and other animation clocks continue behind it.

The scan mesh packs position/UV vertices once per mapping revision. Each GL context retains its uploaded buffer until mapping/settings change; animation frames update uniforms. Grid spacing and fine line widths use approximate projector pixels from the mapped corner spans. Perspective and strong mesh deformation can change their apparent size locally. Hidden and zero-opacity surfaces are excluded; brightness and opacity apply to scan ink. There is no per-frame polygon network generation. Prior to the reveal the renderer skips hidden content preparation/drawing; during the reveal its brightness is faded in while scan ink fades out. This also avoids popping translucent content into view.

The local automatic-start preference lives in QSettings (`output/entrance`), defaults on and is separate from project JSON and Undo. Diagnostics suppress preference writes. `--smoke-entrance` exercises actual replay/skip/output controls, real shader frames, cache reuse/invalidation, interior deformation, transparent-content reveal continuity, persistence/failed-load preservation, blackout pause and timer completion. `--frames <folder>` exports nine rendered stages; `--snapshot <file>` captures the actual editor. All fixtures use disposable synthetic media/projects. `--profile-fx --profile-entrance` measures one/four overlapping scan panels at 1080p with a second visible preview; existing GPU-query and heartbeat limitations still apply.

No physical projector test is established by a local output window. Camera recognition and richer snake/cell choreography remain separate future increments.

Short simultaneous output/preview samples on the Radeon 840M development PC at 1920x1080 output, 960x540 preview, 16x16 deformed meshes: one/four fully overlapping Scan panels measured 13.4/12.6 output repaints per second and 0.795/2.053 ms GPU native-pass time. Matching white-grid samples measured 13.5/13.2 repaints per second and 0.367/0.953 ms. Scan heartbeat p95 lateness was 70/96 ms; grid was 73/78 ms. These short desktop measurements show bounded extra shader work and a slower four-layer stress case; they do not establish guaranteed FPS, total GPU load or physical projector performance. Static scan geometry and buffer reuse are covered by the entrance check.

Release handoff: 0.8.4 merged in PR #15 and published at https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.4. Final extracted launcher entrance and runtime FX checks passed. The actual 0.8.3 launcher installed 0.8.4, reopened the managed fixture mapping, refreshed its root launcher and retained original project/media bytes, the prior runtime and launcher backup. Interrupted download, cancelled restart and incomplete payload checks passed. Public latest metadata and canonical HTTP 200 download confirm the 27,070,173-byte ZIP with SHA256 `6e9ed67bf42522e6a9378554d7203c392c1853365f3599d240efec45db36f915`. The checked local archive is `out/releases/PotatoMapper-0.8.4-windows-x64.zip`; final extracted copy is `out/entrance-check/final/PotatoMapper-0.8.4-windows-x64`. Physical projector appearance remains unverified; next step is Shane's visual feedback.

### Entrance from blackout (0.8.5)

Starting an entrance clears blackout in Scene before resetting its clock and issuing one repaint notification. The Play entrance callback synchronizes the Blackout button and output status immediately; automatic entrances reuse the same transition. Output start without an entrance retains blackout, and manual blackout still pauses a running scan. No-visible-surface starts keep blackout unchanged. The actual UI regression first reproduced the two blocked entrance failures and checks the synchronized controls, clean saved state and continued blackout pause. Release build and final extracted entrance checks passed. Actual 0.8.4 launcher upgrade/reopen retained original project/media and prior runtime/launcher; focused review found no blockers. Published as 0.8.5 in PR #16: https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.5. Public latest metadata and canonical HTTP 200 download match the checked 27,070,792-byte ZIP and SHA256 `db5ab19895371347656384565a2b3c37be8a5df4e1bb981f9c0994a1b4804afa`. Final extracted copy: `out/entrance-unblackout-check/final/PotatoMapper-0.8.5-windows-x64`.

### Fluid snake candidate (implemented, unreleased)

October 7 assignment from the established mapper chat, verified against Shane's direct collaboration request. Source remains C:\Users\mrbos\Downloads\Potato Mapper; branch `codex/fluid-snake-cells`, implementation commit `2de9ad3`. Build with the documented cached CMake and `--build out/build-desktop --config Release --parallel 4`. This candidate is locally committed and packaged; nothing is pushed, merged or released. Parent chat owns review/integration and numbering the future 0.8.6 release.

Hot-Ass Potato keeps Explore/Overload/Reboot/Ambient, replay/skip/palettes and the shared clock. A cached 96-interval mapped arc-length table per leg replaces equal panel-duration movement; quintic acceleration applies at the whole tour's start/end. A connected 36-part tapering ribbon follows earlier route distances and splits at openings; three terminal slots preserve the head/portal copies under the 500-primitive cap. Existing widest-opening greedy routing remains. Strongly bent outer edges still use the corner-outline approximation, although actual meshes influence travel distance/direction and render all geometry.

Cells use a cached output-coordinate grid, nominal spacing 1.8 times the square side, with fewer cells on small panels. Shapes and fixed geometry budgets can thin a large field. Nearest route distance and lateral delay drive a deterministic eased rotation/stretch ripple, smooth settling and gentle micro-motion. Small output-pixel fragments drift to the inside perimeter. There is no accumulated per-frame particle state or new dependency; Potato Focus retains its previous appearance.

Optional integer JSON fields `dynamic.snakeWidth` and `dynamic.cellSize` have ranges 8-96 / 24-160 and defaults 40/56 for earlier projects. Width describes the body; Cell size describes the resting square side. Scene setting changes use Undo/Redo; ticks/seeking preserve clean saved state. Older apps ignore these fields and discard them if they save. Project format stays version 1, with no schema migration.

Pixel offsets/strokes use the inverse local output Jacobian: exact within an affine region, approximate across strong perspective or several mesh triangles. Scene still clips/maps final triangles through the actual mesh. Boundary metrics extrapolate the nearest mesh triangle before clipping; review found clamped probes could erase a visible crossing outline. A renderer line from UV (-.3,.3) to (.1,.7) failed at visible point (.05,.65), then passed after correction. Existing output-display handling sets Scene outputSize to physical display pixels; diagnostic/preview windows uniformly scale that resolution.

Candidate artifacts under `out/fluid-snake-check`:

- `extracted/PotatoMapper-fluid-snake-CANDIDATE-0.8.5/Run-fluid-snake.cmd` opens the synthetic three-panel demo; the root `PotatoMapper.exe` also works. Keep this explicitly labelled 0.8.5-metadata candidate separate from public installations. It is not offered by the updater.
- `PotatoMapper-fluid-snake-CANDIDATE-0.8.5.zip`, 31,032,918 bytes, SHA256 `dc3580e35993515fa515510206c70395e170415e9c414fbb1f9dbfb9f29c9fcd`.
- `fluid-snake-preview.gif`: 240 actual OpenGL framebuffer exports at 70 ms of scene time; `preview.html` supports seeking, `final-frames` holds originals and `fluid-controls.png` shows the real UI. Unequal touching/rotated panels have deformed meshes; the separate gap stays dark.
- `geometry-results.txt`, `storage-results.txt`, `recollection-investigation.txt`, profile files and the extracted candidate's `mapper.log` retain evidence.

Release build and 34 geometry assertions passed: 200/800-pixel head sizing, fewer cells on smaller panels, touching tangent/speed, overlap/rotation, stage bounds and determinism. Extracted `--smoke-dynamic`, `--smoke-entrance`, `--smoke-fx` and `--smoke` passed with system directories only on PATH, covering controls/Undo/Redo/save-reopen, clean ticks, pause CPU/GPU reuse, deformation, duplicate IDs, crossing brightness and the shipped blackout fix. Root-launcher verification checked actual child completion, not launcher exit alone. The real ZIP passed digest/manifest, incomplete-payload, extraction, interrupted-download, cancelled-restart and synthetic project/media-preservation checks. Actual old-launcher install/reopen is deferred until the parent assigns the release version.

Parent review reproduced the collection failure at the final staged-directory rename, with Windows error 5 (access denied). Original edited project bytes and the index remained intact. The external process causing that denial has not been identified. `projects-publish.h` now publishes without replacing an existing destination and retries only access/sharing/lock errors for at most 400 ms. A deterministic real Windows directory handle without delete sharing reproduced failure before retry and passed afterwards; persistent locks fail visibly and retain source bytes, and occupied destinations are never overwritten. This is bounded recovery from temporary denial, not proof that every possible collection failure is fixed. Diagnostics retain synthetic fixtures on failure. A separate existing limitation remains: import identity uses media path/size/timestamp, so a replacement with identical size and timestamp can reuse a stale collected copy.

Short same-machine AMD Radeon 840M samples, 1920x1080 output plus 960x540 preview, density 80, 16x16 deformed meshes, two seconds per one/four-layer case:

| Measurement | Shipped baseline | Final extracted candidate |
| --- | ---: | ---: |
| Output repaints/s, one / four | 16.2 / 14.4 | 22.9 / 18.9 |
| Output mesh ms/frame, one / four | 2.740 / 10.841 | 2.264 / 9.736 |
| Output GPU ms/native pass, one / four | 0.075 / 0.288 | 0.058 / 0.204 |
| Heartbeat p95 lateness ms, one / four | 68 / 80 | 43 / 53 |

An earlier candidate sample measured 17.4/14.7 repaints/s and 2.342/10.102 mesh ms/frame. The variation limits conclusions: these are short desktop measurements, not guaranteed FPS, total GPU utilization or physical-projector proof. The published 0.8.5 ZIP still matches `db5ab19895371347656384565a2b3c37be8a5df4e1bb981f9c0994a1b4804afa`; installed app/projects/media were not replaced. Next step is parent visual/code review, storage-result classification, coordinated integration/update release gates, then Shane's projector check.
