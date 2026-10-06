# Potato Mapper roadmap

A small, watermark-free projection mapper for home use with one projector. This checklist records what works, what we want next, and where development stopped. Priorities can change; proposed features are not release promises.

## Where we left off

- Latest published app: [v0.8.0 for Windows x64](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.8.0).
- **Hex Tide shipped in v0.8.0**, implemented in [PR #11](https://github.com/TheGamingDonKey/potato-mapper/pull/11). Shane approved small breathing hexagons with coordinated travelling waves and gentle flow on October 6. Stable ID 13 and existing controls; final extracted FX checks and an older-launcher update/reopen passed, preserving fixture project/media and the old runtime. Short simultaneous output/preview performance was close to the plain grid; physical projection remains a user check. No active implementation remains.
- On October 1, 2026, Shane reported that the downloaded app runs on another computer. That report does not establish every feature or projector combination on that computer.
- On October 4, Shane again confirmed that the GitHub download runs on a new laptop. Installing this newly published update there remains a user check.
- UI polish and appearance shipped in v0.4.0 ([PR #6](https://github.com/TheGamingDonKey/potato-mapper/pull/6)); Screen/Add and more patterns in v0.5.0 ([PR #7](https://github.com/TheGamingDonKey/potato-mapper/pull/7)); flowing cell effects and controls in v0.6.0 ([PR #8](https://github.com/TheGamingDonKey/potato-mapper/pull/8)). Waterlight, Contour Flow, Curve Maze and Depth Tunnel shipped in v0.7.0 ([PR #9](https://github.com/TheGamingDonKey/potato-mapper/pull/9)). Direct editor update-folder detection is fixed in v0.7.1 ([PR #10](https://github.com/TheGamingDonKey/potato-mapper/pull/10)). On October 5, Chase inspected Predator: the old directly opened editors lacked installation-root information; a fresh main-folder launcher correctly offered v0.7.1, Shane installed it, and the updated copy's check reported up to date. Projects and the existing older editor windows were left untouched. Hex Tide shipped in v0.8.0; project protection/repair, simultaneous preview/output performance and long-run phase continuity remain follow-ups. Raspberry Pi remains deferred.
- October 4 changes: branded splash; panel/context-menu Clear media; surface rename/content labels; File/Edit/Help menus; version display; remembered project folder; and GitHub download/install/restart with previous-version recovery. Media controls are placed before mesh settings in the existing sidebar.
- Local project storage is acceptable. Collecting projects/media for transfer is lower priority than preserving them during app updates. Keep the current release available while developing changes.
- October 5 priority correction: Shane wants recovery from a broken launcher/updater without asking users to delete their installation or manually rescue projects. Project protection and repair take priority over further animation work. The proposed storage/repair design below is pending agreement; v0.7.1 does not implement it.

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

Current protection: updates stage new runtimes and retain the previous version; saves use QSaveFile. Current gap: a new project's default folder is the app root, and media is linked rather than collected. Deleting that folder can therefore remove user work. Recovery currently depends on a working launcher or manual extraction.

- [ ] **Agree the storage and repair design before implementation.** Proposed default: an accessible user project folder outside the program folder; retain an explicit portable workflow. Existing project locations remain valid. Storage changes and the independent repair flow need a reviewed design.
- [ ] **Preserve existing work during adoption.** Offer to copy mappings and referenced media into a chosen project folder, keep originals, resolve filename collisions and rewrite references only in the new mapping. Verify the copied mapping opens with original media unavailable before treating it as protected.
- [ ] **Recoverable project saves.** Add a previous-save backup alongside existing atomic writes. A failed save or backup must leave the original mapping usable and report what happened.
- [ ] **Repair a broken installation.** A recovery entry point must work when the editor cannot start, and have a downloadable replacement when the launcher itself is broken. Repair program files in the existing location, retain user data, restore a valid selected runtime and reopen the saved mapping. Show the affected locations before making changes; failures leave existing data and a recoverable runtime in place.
- [ ] **Demonstrate the complete recovery.** In a disposable installation with real mapping/media files, exercise a damaged editor, missing launcher/layout file, interrupted repair and reinstall of program files. Check project/media bytes and successful reopening. A normal update handoff alone does not prove this recovery flow.

- [ ] **Collect project and media.** Export a new folder containing the mapping and referenced files, handle duplicate filenames, and save portable relative paths. Reopen it after moving the folder with the original media unavailable; leave the original project untouched. This is now part of protecting projects during repair/reinstall, rather than only a transfer convenience.
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
