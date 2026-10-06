# Potato Mapper roadmap

A small, watermark-free projection mapper for home use with one projector. This checklist records what works, what we want next, and where development stopped. Priorities can change; proposed features are not release promises.

## Where we left off

- Latest published app: [v0.8.2 for Windows x64](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.2).
- **0.8.3 implemented — unreleased**, branch `codex/snake-wide-portals`. Centred wide openings, continued tail and common mapped headings. Focused geometry/actual-renderer checks, final extracted package and real 0.8.2 update/reopen passed; review found no remaining blockers. Publishing the checked portable ZIP is next.
- **0.8.2 shipped**, merged in [PR #13](https://github.com/TheGamingDonKey/potato-mapper/pull/13): shared Dynamic FX geometry, paused buffer reuse, regular-mesh fast path and reduced deformed-mesh allocation. Final portable mapping/FX checks and actual 0.8.1 update/reopen passed, preserving fixture project/media and prior runtime. Public latest-release metadata and canonical HTTP 200 download match the checked ZIP's size and SHA256. The 0.8.1 managed Projects, launcher refresh/recovery and coordinated animations remain included. Camera work is deferred; continue with 0.8.x increments before 0.9/1.0. The specific reported File → Open failure still needs the failing file path/message.
- On October 1, 2026, Shane reported that the downloaded app runs on another computer. That report does not establish every feature or projector combination on that computer.
- On October 4, Shane again confirmed that the GitHub download runs on a new laptop. Installing this newly published update there remains a user check.
- UI polish and appearance shipped in v0.4.0 ([PR #6](https://github.com/TheGamingDonKey/potato-mapper/pull/6)); Screen/Add and more patterns in v0.5.0 ([PR #7](https://github.com/TheGamingDonKey/potato-mapper/pull/7)); flowing cell effects and controls in v0.6.0 ([PR #8](https://github.com/TheGamingDonKey/potato-mapper/pull/8)). Waterlight, Contour Flow, Curve Maze and Depth Tunnel shipped in v0.7.0 ([PR #9](https://github.com/TheGamingDonKey/potato-mapper/pull/9)). Direct editor update-folder detection is fixed in v0.7.1 ([PR #10](https://github.com/TheGamingDonKey/potato-mapper/pull/10)). On October 5, Chase inspected Predator: the old directly opened editors lacked installation-root information; a fresh main-folder launcher correctly offered v0.7.1, Shane installed it, and the updated copy's check reported up to date. Projects and the existing older editor windows were left untouched. Hex Tide shipped in v0.8.0; project protection/repair, simultaneous preview/output performance and long-run phase continuity remain follow-ups. Raspberry Pi remains deferred.
- October 4 changes: branded splash; panel/context-menu Clear media; surface rename/content labels; File/Edit/Help menus; version display; remembered project folder; and GitHub download/install/restart with previous-version recovery. Media controls are placed before mesh settings in the existing sidebar.
- Local portable project storage is the approved default: a Projects folder beside the root launcher, with collected media and originals retained. Deleting the whole portable folder still deletes its local projects; keep independent backups.
- October 5 priority correction: Shane wants recovery from a broken launcher/updater without asking users to delete their installation or manually rescue projects. The October 6 design adds native launcher refresh, retained-runtime repair and project collection; a deleted launcher still requires a runnable editor or full-package replacement.

## How to keep this useful

- Unchecked means planned or unfinished. Add **In progress** and a branch/PR link when work starts.
- Check an item only after its completion condition is met and relevant checks pass. Include a short evidence note and the implementation PR or commit.
- If merged code is not in a downloadable app yet, label it **Implemented — unreleased**. Add the release link when it ships. Merging and publishing are separate steps.
- Record unperformed checks honestly. A local output window is not evidence of a physical projector test.
- Update this file in the same change as the feature or fix. After publishing, update release links and the handoff above. Keep completed entries as history.
- A checked item is not a guarantee against every future bug. Reopen it or link a follow-up issue when a concrete problem remains.
- Use GitHub Issues for detailed bug reports or larger discussions when useful; link them here. Keep this file as the short overview.

## Shipped in v0.2.0

These features are in the [published release](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.2.0). See [DEVELOPMENT.md](DEVELOPMENT.md) for the scope of existing diagnostics and performance measurements.

- [x] Manual corner and subdivided-mesh mapping with multiple surfaces, images, and looping video.
- [x] Transparent white alignment grids and animated dots with speed, size, and pause controls.
- [x] Surface ordering, duplication, visibility, locking, undo, and redo.
- [x] Save/open `.pmap` projects and load earlier `.hmap` files. Media is currently linked, not embedded.
- [x] Separate fullscreen projector output, display selection, and blackout.
- [x] Reduce common NV12 video-frame conversion costs. Colour/fallback diagnostics and local multi-video profiling completed; see [PR #1](https://github.com/TheGamingDonKey/potato-mapper/pull/1). Performance still depends on hardware and media.
- [x] Portable Windows x64 download with Qt/FFmpeg and Visual C++ runtime dependencies. Extracted-package mapping, animation, colour, and MP4 checks passed; see [PR #2](https://github.com/TheGamingDonKey/potato-mapper/pull/2). Shane subsequently reported a successful launch on another computer.

## Shipped in v0.3.0

These desktop improvements are in [v0.3.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.3.0), implemented in [PR #5](https://github.com/TheGamingDonKey/potato-mapper/pull/5).

- [x] **Branded startup splash — shipped in v0.3.0.** Existing potato logo, actual version, centred/dismissible presentation; excluded from diagnostics and projector output. Normal editor launch and the splash widget's rendering/centering at increased Qt scale (1.875x) checked locally. See PR #5 and `--test-splash`.
- [x] **Clear selected surface content — shipped in v0.3.0.** Visible Media-panel action plus right-click on a surface or its list entry. Removes image/video/dots content, retains geometry and other surfaces, supports Undo/Redo and save/reopen, and leaves source files in place. `--smoke-clear` passed; actual surface context menu inspected.
- [x] **Check and install GitHub updates (Windows) — shipped in v0.3.0.** Help menu checks stable public releases, streams/cancels downloads, verifies SHA256 and runtime manifest, prompts for unsaved work/active projection, then hands off to the native launcher and reopens the saved mapping. Versioned folders retain previous runtimes. Actual ZIP extraction and editor-process handoff passed in a disposable installation, as did interrupted download, cancelled restart, incomplete installation, rollback/re-update, startup-failure recovery with both pointers restored, staging cleanup, and unchanged project/media checks. The real GitHub check dialog was inspected. v0.2.0 needs one manual package-copy upgrade; later copies use Help. A future GitHub-hosted update on Shane's other computer and a physical-projector update remain untested.

## Approved development order — October 4, 2026

Shane approved these increments in chat. Work proceeds in this order, with a runnable release between useful milestones. The current increment extends the existing editor and renderer; it does not introduce another framework.

1. **UI polish and appearance controls — shipped in v0.4.0.** See the completed checklist below. Dots remain the only generator in this first increment.
2. **Screen/Add blending — shipped in v0.5.0.** Overlay suitable black-background media. This does not remove arbitrary video backgrounds.
3. **Potato FX pattern collection — shipped in v0.6.0.** Diagonal stripes, rings and stepped waves shipped in v0.5.0. The new collection includes the inspected SquareWave, Diagonals, CubicCircles and SquareArray: smaller elements, coordinated motion and shared colour/direction/density controls. This brings the collection to eight animations including the retained old effects. Check motion, pause, saved settings and output performance as patterns are added.
4. **Missing-media repair and project collection.** Relink files while retaining geometry; export a project with its media for another computer.
5. **Recovery and performance display.** Recover recent work after interruption, retain recoverable project backups and show useful output frame-rate information.

Raspberry Pi, audio-reactive effects and multiple projectors remain later investigations.

## Shipped in v0.4.0

Implemented in [PR #6](https://github.com/TheGamingDonKey/potato-mapper/pull/6), merged and published in [v0.4.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.4.0).

- [x] **UI polish.** Rounded themed scrollbars, resizable sidebar, persistent surface list, collapsible inspector sections, shortened filenames/full-path tooltips and relevant video controls. Initial window fits the available desktop. Actual editor inspected at 1024 × 640 logical pixels; checks cover resizing, filename handling and keeping newly selected/added surfaces visible while preserving scroll during appearance edits.
- [x] **Potato FX picker.** Existing dots are selected through Animation; their speed/size/pause controls appear when relevant. Loading media or clearing content returns the surface to media/grid. Additional generators remain planned.
- [x] **Brightness and opacity per surface.** Shared image/video/dots rendering, defaults of 100% for older projects, live preview, one Undo per slider drag, Redo, reset and saved settings. Pixel checks cover images, dots, native NV12 video, overlapping layers and agreement between editor/output. Canvas alpha composition was corrected so Qt does not apply surface transparency twice.
- [x] **Portable release and update handoff.** Release build and packaged appearance/mapping/dots/colour/clear/MP4 checks passed without developer libraries on PATH. Download/extraction/interruption/cancellation checks passed. A real v0.3.0 launcher installed v0.4.0 and reopened a saved fixture mapping; project/media bytes and the retained v0.3.0 runtime were unchanged. Public latest-release metadata matches the checked ZIP and SHA256. Physical-projector hardware and this new update on Shane's other laptop remain untested.

## Project and editing improvements

### Requested priority: project protection and application repair

Protection in 0.8.1: updates retain versions, managed copies collect available media, and saves use QSaveFile plus a previous-save backup. New media stays linked until collection. Deleting the whole portable folder can remove user work; keep an independent backup. Recovery still needs a runnable entry point or downloaded replacement.

- [x] **Agree the storage and repair design before implementation.** Shane approved portable Projects beside the launcher and maintenance within that installation on October 6. See the committed Dynamic FX/projects design and implementation plan under `docs/superpowers`.
- [x] **Preserve existing work during adoption — shipped in 0.8.1.** Copy mappings/media into unique managed folders, retain originals, resolve collisions and rewrite only copied references. Temporary-fixture checks prove original retention, independent reopening, repeat collection, edited-copy preservation and relocation.
- [x] **Recoverable project saves — shipped in 0.8.1.** Previous exact bytes retained as `.bak` before atomic replacement; focused backup and round-trip checks pass.
- [ ] **Broader guided installation repair.** 0.8.1 implements native root-launcher refresh, pointer recovery and fallback for damaged inventoried runtimes. A deleted launcher or missing installation marker still needs a working editor with its root path or full-package replacement. A dedicated guided repair/download entry point remains future work.
- [ ] **Demonstrate every full-reinstall scenario.** 0.8.1 disposable fixtures passed actual old-launcher refresh/reopen, corrupted current editor, missing inventoried DLL, invalid pointer, valid rollback and interrupted atomic replacement while preserving project/media. Missing installation markers and complete replacement with no runnable entry point remain separate scenarios.

- [x] **Collect project and media — shipped in 0.8.1.** Opening and bounded startup adoption create portable managed copies, with relative paths and unique names. Moved-copy and unavailable-original checks pass. Separate explicit Export UI remains optional future work.
- [ ] **Find missing media.** Show which files are missing and allow them to be relinked without losing surface geometry. Check reopening a moved project and saving corrected paths.
- [x] **Protect saved projects — shipped in 0.8.1.** Atomic saves plus `.bak`; earlier local HomeMapper project loads. Invalid structural data leaves the current scene unchanged. The separately reported failing file remains unprovided.
- [ ] **Centralise editing actions (started with clear and rename).** Move surface changes out of individual button callbacks into shared operations that handle validation, undo, dirty state, and view updates. Start with existing actions; check their behaviour stays the same.
- [ ] **Clarify code responsibilities as we extend them.** Separate project storage, media playback, rendering, and interface code where they currently overlap. Make small changes around actual features; keep an executable working between changes.
- [x] **Identify the running version — shipped in v0.3.0.** Help/About and the update dialog show the actual application version. File/Open app folder locates the runtime log in the stable app root. Live update/version UI inspected locally.

## Next visual features

- [x] **Screen and Add blending — shipped in v0.5.0.** Selected-surface Normal/Screen/Add, Undo/Redo/reset and saved settings. Numerical editor/output checks cover opacity, image alpha, black overlays, native NV12 and patterns over a midtone layer. Normal is the default for older mappings. This is not general background removal.
- [x] **Brightness and opacity per surface — shipped in v0.4.0.** Images, videos and dots share the controls; old-project defaults, Undo/Redo, save/reopen, overlap pixels and matching output checked. See PR #6 and the v0.4.0 checklist above.
- [x] **Three more procedural patterns — shipped in v0.5.0.** Diagonal stripes, expanding rings and square waves, plus existing dots. Actual UI speed/width/pause, motion in both canvases, unchanged project state during ticks, saved settings and Clear/Undo checked. Actual output contact sheet and editor inspected. Four further patterns and colour/direction/spacing controls remain planned; audio-reactive effects are separate.

## Builds and releases

- [ ] **Automate Windows packaging on GitHub Actions.** Build with documented dependency versions, package the runtime files/notices, and retain the downloadable artifact. Separate checks that can run on the build machine from actual graphics/playback checks; publish a release only after its required checks pass.
- [ ] **Try a second operating system.** Choose macOS or Linux when a suitable test machine is available. Build and package it, then check opening projects, images, video, mesh editing, and display output. Record limitations before advertising support. Qt portability alone does not prove the app works there.

<details>
<summary>Later ideas — investigate before scheduling</summary>

- More pattern controls and a clearer media panel, based on actual use.
- Audio-reactive patterns: requires audio input/analysis, timing, and device handling.
- Multiple projector outputs and edge blending: requires multiple-display hardware and a larger rendering/output design.
- Automatic calibration and 3D model workflows: larger research projects, outside the current home-mapper scope.
- Installer and signing if distribution needs justify them. User-requested GitHub updates are tracked in Current requested features above.

These are possibilities, not an approved implementation queue. Preserve the useful small application while deciding what to add.

</details>

## Shipped in v0.5.0

Merged in [PR #7](https://github.com/TheGamingDonKey/potato-mapper/pull/7) and published in [v0.5.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.5.0). Public latest-release metadata, canonical download (HTTP 200), archive size and SHA256 match the checked package.

- Release build and source review passed. `--smoke-fx`, appearance, mapping, video-colour and clear checks passed locally. The strengthened pattern Screen check uses a midtone underlay.
- Fresh portable package FX/appearance and real MP4 checks passed with only Windows system directories on PATH. An earlier FX check hung after its assertions; it did not recur in the fresh checked package. No persistent cause was established.
- Real ZIP extraction, interrupted download and cancelled restart checks passed. The actual v0.4.0 launcher installed v0.5.0 and reopened `My mapping.pmap` in a disposable installation. Project/media bytes and the retained old runtime were unchanged.
- Physical-projector testing and this update on Shane's other laptop remain untested. The existing user app/mappings/media were preserved.

## Flowing effects — shipped in v0.6.0

Merged in [PR #8](https://github.com/TheGamingDonKey/potato-mapper/pull/8) and published in [v0.6.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.6.0). Public latest-release metadata, canonical download (HTTP 200), archive size and SHA256 match the checked final package.

- [x] Four new cell effects with coordinated movement: SquareWave, Diagonals, CubicCircles and SquareArray. Existing IDs/renderers retained; the old square-wave effect is labelled Stepped waves. Defaults use smaller white elements without a fixed boundary fade. Actual output inspected at multiple phases against MadMapper.
- [x] Per-surface density, flow, angle, colour presets, reverse and optional soft edge; size/speed/pause retained. Actual controls, Undo/Redo, save/reopen, legacy defaults, motion and blending passed locally. Appearance and video-colour regression checks passed.
- [x] Published v0.6.0. Focused source review completed; its Reverse continuity issue was fixed and checked for unchanged paused pixels in both canvases and backward timed movement. Fresh portable FX and real MP4 checks passed with only Windows system directories on PATH. The actual v0.5.0 launcher installed the final v0.6.0 ZIP and reopened the saved fixture; project/media bytes and the retained runtime were unchanged. Real extraction, interrupted download and cancelled restart also passed. Physical-projector and other-laptop checks remain separate.
- Later visual research: Curve Maze's connected tiles and Waterlight's caustic-like ridges now ship in v0.7.0. Ripple fields remain an idea. [Pattern techniques](https://thebookofshaders.com/09/) and [wave modulation](https://thebookofshaders.com/13/) provide a starting point.

## Organic and depth effects — shipped in v0.7.0

Merged in [PR #9](https://github.com/TheGamingDonKey/potato-mapper/pull/9) and published in [v0.7.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.7.0). Public latest-release metadata, canonical HTTP 200 download, size and SHA256 match the checked final package.

- [x] Four original procedural effects using the existing mapped renderer and Potato FX controls: Waterlight, Contour Flow, Curve Maze and Depth Tunnel. Previous pattern IDs are retained. Actual multi-phase output inspected; Waterlight and Curve Maze were tuned for finer detail.
- [x] Actual control/render/motion/width/pause/blend/legacy/save checks passed, as did appearance and video-colour checks. Corrected 1080p output plus 960×540 editor measurements were approximately 19–21 repaints/s for the new effects; forced plain-grid rendering was similar. Output-only measurements were around 60. These are short local measurements; simultaneous-rendering performance still needs investigation. Source review completed; diagnostic argument/graphics-failure handling was corrected.
- [x] Final portable FX and real MP4 playback passed without developer libraries on PATH. The actual v0.6.0 launcher installed the final ZIP and reopened the fixture mapping; project/media bytes and the retained runtime were unchanged. Extraction, interrupted download and cancelled restart checks also passed. Invalid profiler arguments exit with code 2; valid profiling passed after review corrections.
- [x] Published v0.7.0. Projector hardware and the new update on another computer remain user checks. Shane reported the existing updater works; this specific version still needs his real-use feedback.
- [ ] Improve long-run animation phase continuity. The inherited 10,000-unit phase wrap can jump after approximately 2.8 hours at default speed, or 56 minutes at 300%; account for shader float precision when replacing it.

## Update folder discovery — shipped in v0.7.1

Merged in [PR #10](https://github.com/TheGamingDonKey/potato-mapper/pull/10) and published in [v0.7.1](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.7.1).

- [x] Directly opening an editor under `versions/<version>` now discovers its enclosing portable package when the launcher and installation marker are present. Explicit launcher arguments and flat copies retain their existing behavior. Missing files produce instructions to open the main-folder launcher before recommending another ZIP.
- [x] Folder-resolution regression failed before the fix and passed afterwards. A direct packaged editor startup acquired the outer installation lock, and a direct packaged smoke run passed with only Windows system directories on PATH. A nested updater test with no explicit root offered the download, extracted the real ZIP and passed interruption/cancellation checks. The actual v0.6.0 launcher installed v0.7.1 and reopened the saved fixture; project/media and the retained runtime were unchanged. Focused source review found no blockers.
- [x] Published v0.7.1. Public latest-release metadata, canonical HTTP 200 download, size and SHA256 match the final package. On October 5, Chase confirmed on Predator that opening the main-folder launcher offered the update; Shane installed it, and the updated editor reported up to date. Direct editor discovery has been checked locally, but was not separately exercised on Predator.

## Hex Tide — shipped in v0.8.0

Merged in [PR #11](https://github.com/TheGamingDonKey/potato-mapper/pull/11) and published in [v0.8.0](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.0).

- [x] Coordinated breathing hexagons with gentle lattice flow, using stable ID 13 and the existing controls. Actual portable editor/picker and multi-phase output inspected. No added runtime dependency.
- [x] Selection, distinct geometry, short-time continuity, size, motion in both canvases, pause, blending, clean timer ticks and saved settings passed. Focused source review found no blockers. After simplifying the wave calculation, a short 1080p output plus 960×540 preview profile measured 26.3/26.0 repaints/s for one/four layers, versus grid 26.4/26.6. Physical projection and long-run phase continuity remain separate checks.
- [x] Final extracted FX and actual older-launcher install/reopen passed; fixture project/media and the retained runtime stayed unchanged. Public latest-release metadata, canonical HTTP 200 download, size and SHA256 match the final ZIP. Further-user hardware testing remains open.

## Dynamic FX and managed projects — shipped in 0.8.1

Merged in [PR #12](https://github.com/TheGamingDonKey/potato-mapper/pull/12) and published in [v0.8.1](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.1).

- [x] **Hot-Ass Potato and Potato Focus.** Shared group clock, participating surfaces, six palettes, speed/density/pause, replay/skip, bounded procedural geometry and existing mesh/blend handling. Original content returns with Off. Actual motion stages and controls inspected; three-surface examples packaged. Closest openings use corner/projective boundary sampling; bent-edge routing remains approximate and gaps stay dark.
- [x] **Projects and previous saves.** Portable collection with media and relative paths, collision handling, retained edited copies, bounded legacy adoption, recent projects and `.bak`. Twenty-six temporary-fixture assertions passed; the existing local HomeMapper file loads five surfaces without being saved. New media remains linked until collection. The separately reported File → Open failure remains unprovided.
- [x] **Promoted launcher.** Protocol-1 payload refreshes the root executable through a native helper; backups/retries/error reports retained. Real 0.6 launcher installed the final ZIP, refreshed itself, reopened the managed fixture and retained original mapping/media/runtime bytes. Corrupted editor and missing inventoried DLL recover to a complete previous version; invalid pointer repairs and valid rollback stays selected. Interrupted replacement and failed verification preserve the old executable, and successful retry clears stale errors.
- [x] **Focused validation.** Release build, 23 geometry assertions, 26 project assertions, Dynamic FX runtime checks and extracted mapping/existing FX checks passed. Review regressions covered invalid-load scene preservation, Undo clock continuity, six palettes and interior mesh deformation. Final ZIP passed real extraction, interrupted-download/cancel checks and older-launcher update/reopen. Short simultaneous 1080p output/preview measurements were 18–22 repaints/s, versus grid 22–25 on the older development PC; see DEVELOPMENT.md.
- [x] **Publish 0.8.1 and verify its public download.** Public stable latest is v0.8.1; canonical HTTP 200 ZIP, 27,042,178 bytes and SHA256 match the tested final artifact. Camera, physical-projector testing, the specific unprovided failing project and broader guided repair remain follow-ups.

## Dynamic FX optimization — shipped in 0.8.2

Merged in [PR #13](https://github.com/TheGamingDonKey/potato-mapper/pull/13) and published in [v0.8.2](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.2).

- [x] **Share prepared geometry.** One lazy packed vertex frame serves editor and projector. Each canvas uploads it once per revision and reuses it while paused. Regular meshes skip unnecessary subdivision; deformed meshes retain exact cell/diagonal clipping with bounded stack storage instead of temporary polygon allocation.
- [x] **Stop hidden-animation redraws.** Individual FX under a paused Dynamic FX group retain their phase progression without triggering invisible repaint work. Timed clean-state, pause/pixel reuse and uploaded-buffer reuse checks pass.
- [x] **Retain accepted project behavior and appearance.** Review caught duplicate surface IDs sharing the wrong cached mesh. Surface-index ranges fix it; an actual two-mesh pixel regression failed before and passed after the correction. The interior mesh scan still passes. Eight rendered phases match the previous renderer within tiny edge rounding (maximum mean channel difference 0.00073/255; fewer than 0.002% of pixels change by more than three levels).
- [x] **Measure real costs.** The original dense regular-mesh test used roughly 38–48 ms total CPU meshing across both windows, while native GPU query intervals remained below 0.4 ms. Shared preparation and the regular fast path reduced this to roughly 2 ms. Deformed four-surface preparation fell further from 19–24 ms with the initial cache to 8–10 ms with allocation-free inner clipping. Native-pass profiling excludes CPU geometry preparation and Qt's final composition; it is not total GPU utilization. See DEVELOPMENT.md for final rates and limits.
- [x] **Check the final portable ZIP and update.** Extracted Dynamic FX, existing FX and mapping checks passed with only Windows system directories on PATH. An actual 0.8.1 launcher installed 0.8.2, refreshed itself and reopened the managed mapping; original project/media bytes, old runtime and launcher backup were retained. Source review found no remaining blockers.
- [x] **Publish the checked 0.8.2 portable ZIP.** Public latest metadata and canonical HTTP 200 download match the tested 27,049,169-byte ZIP and SHA256 `36c529a89e406f1c75276c073d1ed3a430738b6a48fa62ab629ff9d943a804f3`. Hardware/driver/window composition still affect repaint rates; optimization does not guarantee 60 FPS on all computers.

## Wider snake connections — implemented, unreleased (0.8.3)

- [x] Choose the centre of the widest facing overlap between the nearest tour neighbours, retaining nearest-boundary fallback for corner-only/separated diagonal arrangements. Overlapping panels share one mapped point when available; nested/concentric panels retain fallback.
- [x] Keep the head visible and distribute the trailing body across surfaces during both exploration and ambient passes; join portal directions in mapped space. Oblique near-corner bends preserve direction by shortening the whole UV ray. Regression failed before correction and passed afterwards.
- [x] Check partial overlaps, touching and overlapping panels, rotation/winding, rendered handoff brightness and animation performance. Actual output inspected around a crossing. Dense four-surface geometry cost remained roughly 2.3 ms in a same-session 0.8.2 comparison; short desktop FPS remains hardware/window dependent.
- [x] Final extracted mapping/Dynamic FX checks and real 0.8.2 launcher update/reopen passed, retaining original fixture project/media, previous runtime and root-launcher backup. Focused review found no remaining blockers.
- [ ] Publish and verify the checked portable 0.8.3 update. Physical projection remains a user check; corner-based routing still approximates deformed outer mesh edges.
