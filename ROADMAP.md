# Potato Mapper roadmap

A small, watermark-free projection mapper for home use with one projector. This checklist records what works, what we want next, and where development stopped. Priorities can change; proposed features are not release promises.

## Where we left off

- Latest published app: [v0.3.0 for Windows x64](https://github.com/TheGamingDonKey/potato-mapper/releases/tag/v0.3.0).
- On October 1, 2026, Shane reported that the downloaded app runs on another computer. That report does not establish every feature or projector combination on that computer.
- Desktop usability/update work is merged in [PR #5](https://github.com/TheGamingDonKey/potato-mapper/pull/5) and published in v0.3.0. No feature implementation is currently in progress. Raspberry Pi remains deferred.
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

## Supporting improvements

Take one item at a time. Each line includes the behaviour needed before checking it off.

- [ ] **Collect project and media (lower priority).** Export a new folder containing the mapping and referenced files, handle duplicate filenames, and save portable relative paths. Reopen it after moving the folder with the original media unavailable; leave the original project untouched.
- [ ] **Find missing media.** Show which files are missing and allow them to be relinked without losing surface geometry. Check reopening a moved project and saving corrected paths.
- [ ] **Protect saved projects.** Add a recoverable backup when replacing an existing mapping and retain safe behaviour if saving fails. Keep a small set of earlier-format examples that must still load; preserve project media separately.
- [ ] **Centralise editing actions (started with clear and rename).** Move surface changes out of individual button callbacks into shared operations that handle validation, undo, dirty state, and view updates. Start with existing actions; check their behaviour stays the same.
- [ ] **Clarify code responsibilities as we extend them.** Separate project storage, media playback, rendering, and interface code where they currently overlap. Make small changes around actual features; keep an executable working between changes.
- [x] **Identify the running version — shipped in v0.3.0.** Help/About and the update dialog show the actual application version. File/Open app folder locates the runtime log in the stable app root. Live update/version UI inspected locally.

## Next visual features

- [ ] **Screen and Add blending.** Let a surface choose normal, Screen, or Add blending so suitable black-background videos can overlay other surfaces. Check overlapping media in editor/output, undo, and save/reopen. This is not general background removal.
- [ ] **Brightness and opacity per surface.** Apply controls consistently to images, videos, and generated patterns; include defaults for old projects, undo, save/reopen, and matching projector output.
- [ ] **One more procedural pattern.** Start with diagonal moving lines, then consider rings or square waves. Share timing and settings handling with dots; verify animation, pause, and saved controls in editor/output. A real audio spectrum or waveform is separate work.

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
