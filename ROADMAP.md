# Potato Mapper roadmap

A small, watermark-free projection mapper for home use with one projector. This checklist records what works, what we want next, and where development stopped. Priorities can change; proposed features are not release promises.

## Where we left off

- Latest published app: [v0.6.0 for Windows x64](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.6.0).
- On October 1, 2026, Shane reported that the downloaded app runs on another computer. That report does not establish every feature or projector combination on that computer.
- On October 4, Shane again confirmed that the GitHub download runs on a new laptop. Installing this newly published update there remains a user check.
- UI polish, Potato FX naming/picker and per-surface brightness/opacity are merged in [PR #6](https://github.com/TheGamingDonKey/potato-mapper/pull/6) and shipped in v0.4.0. Screen/Add blending and three more Potato FX patterns are merged in [PR #7](https://github.com/TheGamingDonKey/potato-mapper/pull/7) and shipped in v0.5.0. The flowing cell effects and shared controls are merged in [PR #8](https://github.com/TheGamingDonKey/potato-mapper/pull/8) and shipped in v0.6.0. Current work: `codex/organic-potato-fx`, the approved Waterlight/Contour Flow/Curve Maze/Depth Tunnel collection for v0.7.0 and focused performance measurements. Missing-media repair follows this increment. Raspberry Pi remains deferred.
- October 4 changes: branded splash; panel/context-menu Clear media; surface rename/content labels; File/Edit/Help menus; version display; remembered project folder; and GitHub download/install/restart with previous-version recovery. Media controls are placed before mesh settings in the existing sidebar.
- Local project storage is acceptable. Collecting projects/media for transfer is lower priority than preserving them during app updates. Keep the current release available while developing changes.

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

- [ ] **Collect project and media (lower priority).** Export a new folder containing the mapping and referenced files, handle duplicate filenames, and save portable relative paths. Reopen it after moving the folder with the original media unavailable; leave the original project untouched.
- [ ] **Find missing media.** Show which files are missing and allow them to be relinked without losing surface geometry. Check reopening a moved project and saving corrected paths.
- [ ] **Protect saved projects.** Add a recoverable backup when replacing an existing mapping and retain safe behaviour if saving fails. Keep a small set of earlier-format examples that must still load; preserve project media separately.
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
- Later visual research: flowing Truchet tiles, ripple fields and caustic-like light. Investigate one at a time after feedback on these four effects; these are not implemented. [Pattern techniques](https://thebookofshaders.com/09/) and [wave modulation](https://thebookofshaders.com/13/) provide a starting point.

## Organic and depth effects — implemented, unreleased v0.7.0

- [x] Four original procedural effects using the existing mapped renderer and Potato FX controls: Waterlight, Contour Flow, Curve Maze and Depth Tunnel. Previous pattern IDs are retained. Actual multi-phase output inspected; Waterlight and Curve Maze were tuned for finer detail.
- [x] Actual control/render/motion/width/pause/blend/legacy/save checks passed, as did appearance and video-colour checks. Corrected 1080p output plus 960×540 editor measurements were approximately 19–21 repaints/s for the new effects; forced plain-grid rendering was similar. Output-only measurements were around 60. These are short local measurements; simultaneous-rendering performance still needs investigation. Source review completed; diagnostic argument/graphics-failure handling was corrected.
- [x] Final portable FX and real MP4 playback passed without developer libraries on PATH. The actual v0.6.0 launcher installed the final ZIP and reopened the fixture mapping; project/media bytes and the retained runtime were unchanged. Extraction, interrupted download and cancelled restart checks also passed. Invalid profiler arguments exit with code 2; valid profiling passed after review corrections.
- [ ] Publish v0.7.0. Projector hardware and the new update on another computer remain user checks.
- [ ] Improve long-run animation phase continuity. The inherited 10,000-unit phase wrap can jump after approximately 2.8 hours at default speed, or 56 minutes at 300%; account for shader float precision when replacing it.
