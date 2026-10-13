# Halloween and Fall Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking. Root owns integration, builds and release; narrowly assigned artists/test helpers may edit disjoint files without running builds.

**Goal:** Publish Potato Mapper 0.8.8 with Pumpkin Mischief, Ghost Parade, Rainlit Window, four examples and a native moving preview.

**Architecture:** Append three effects to the existing per-surface GLSL renderer. Reuse seasonal geometry helpers and the existing model/UI/persistence/update path. One artist owns the shared shader; root integrates and validates one final package.

**Tech Stack:** C++17, Qt 6.10.3/MSVC2022 x64, OpenGL 3.3, cached CMake; Python/Pillow only for encoding exported preview frames.

**Spec:** `docs/superpowers/specs/2026-10-13-halloween-fall-design.md`

## Global Constraints

- Preserve IDs 0–15 and project format version 1; append 16–18 only.
- Size 24 for all new IDs; Density 24/32/24; Flow 60/60/50; Original colour 0.
- IDs 14–18 use a twelve-second period; IDs 0–13 keep 10000 seconds.
- Fixed per-fragment work; no external media, dependency changes or spatial exits around derivatives.
- Keep user project/media/install locations and updater protocol; root alone builds, packages and publishes.

## Review Focus

- Negative/long-running phases: wrapped motion must match its twelve-second reference and stay continuous at the seam (Task 1 renderer regression).
- Dense, tiny or strongly shaped panels: artwork must render finite visible content and respond to controls without destabilizing the editor (Task 1 representative extremes and visual capture).
- Paused Reverse/Speed zero: preserve the current frame, then resume backward cleanly (Task 1 timed state checks).
- Missing new JSON controls/older saved IDs: default new Flow correctly while retaining legacy values and Undo/save behaviour (Task 1 load/roundtrip assertions).
- Advertised density and final installation: measure actual requested density and preserve existing saved work/rollback files (Tasks 2 and 3 diagnostics/handoff).

---

### Task 1: Native scenes, controls and examples

**Files:** Modify `assets/patterns.frag`, `src/patterns.h`, `src/model.cpp`, `src/canvas.cpp`, `src/main.cpp`, `tests/fx-check.h`; create `tests/fall-check.h` and four named `.pmap` examples.

**Interfaces:** `potatoPatternPeriod(int)` returns `double`; `potatoPatternDefaultFlow(int)` returns `int`. `runFallChecks(Window&, QApplication&, QStringList)` implements `--smoke-fall` / `--test-fall-motion`, optional `--frames`, `--snapshot`, `--demo`. Preserve `runHarvestChecks` and its sampling utilities.

- [x] Add picker assertions for IDs 16/17/18 to the real FX sweep. Add the dedicated renderer fixture before implementing effects; dispatch its diagnostic from main.
- [x] Build with cached CMake and run `--smoke-fall` / `--smoke-fx`; retain RED logs naming missing presets rather than unrelated errors.
- [x] Implement exactly the selected scene compositions and bounded choreography. Extend shaded palette handling to new IDs. Append list names, defaults and CPU period/Flow helpers, then connect model defaults and example menu routing.
- [x] Include three separate scene examples and an October Porch mixed mapping (one panel of each new ID), original colours, playing defaults and Dynamic FX off. Keep all coordinates valid and paths self-contained.
- [x] Run `--smoke-fall`; assert all default IDs, finite/visible output, isolated motion, negative/long phase equivalence, seam continuity, representative control extremes, editor/output parity, settings Undo/Redo, clean ticks, Pause/Reverse/Speed-zero and save/reopen. Existing full FX sweep retains brightness/alpha blend assertions. For new illustrated scenes, Size must change rendered content; increasing total RGB energy is not a universal size contract.
- [x] Export 96 frames at 0.125-second phase steps and a still from actual mapped output. Inspect the whole moving story and landscape/portrait frames; revise visible defects before accepting art. Encode the final identical frames to a twelve-second GIF without altering artwork.
- [x] Commit reviewed scene/test/example changes after the native checks pass.

### Task 2: Performance, compatibility and numbered candidate

**Files:** Modify `tests/fx-profile.h`, `CMakeLists.txt`, `assets/potato-mapper.rc`, README, ROADMAP, DEVELOPMENT and `packaging/START-HERE.txt`.

**Interfaces:** Existing `--profile-fx --first-pattern 16 --last-pattern 18 --profile-density 80 --profile-warp` must apply Density 80 to every ordinary FX surface and print actual density. Runtime/app/launcher all report 0.8.8.

- [x] Add an opt-in model assertion (`--check-profile-density`) to the existing native profile fixture; verify RED because ordinary FX currently ignores requested density.
- [x] Apply the fixture density with `Scene::setFxLook` and report/check actual values; verify GREEN. This fixes measurement, without changing ordinary app controls.
- [x] Measure defaults and maximum density for one/four overlapping layers at 1080p output plus preview. Compare costs with shipped seasonal scenes on the same hardware; retain measurements rather than promise a frame rate. If an optimization changes frames, investigate/revert it instead of accepting a subjective score.
- [x] Bump CMake/Windows resource metadata to 0.8.8 and write user instructions. Mark implemented work unreleased and actual remaining gates in ROADMAP/DEVELOPMENT.
- [x] Build Release and run full FX, fall, harvest, Dynamic FX, entrance and mapping checks, plus `PotatoDynamicChecks` and `PotatoProjectChecks`; read every result. Repeat only after changes or failures justify it.
- [x] Commit the checked candidate and documentation.

### Task 3: Verified distribution, upgrade and public release

**Files:** Existing packager/updater unchanged; ignored evidence/helpers under `out/halloween-check`; final release handoff in ROADMAP/DEVELOPMENT.

**Interfaces:** `out/releases/PotatoMapper-0.8.8-windows-x64.zip`; fresh extraction under `out/halloween-check/final`; prior runtime fixture from `out/harvest-check/final/PotatoMapper-0.8.7-windows-x64`.

- [ ] Package final Release binaries using `package.ps1` with cached Qt. Extract into a fresh directory; record exact ZIP size/SHA256.
- [ ] With only Windows system directories on PATH, run extracted mapping/fall/FX/harvest/Dynamic FX/entrance checks, all four new examples, and an actual root-launcher child diagnostic. Check child completion, not just launcher exit.
- [ ] Adapt the proven disposable update rehearsal to actual 0.8.7→0.8.8. Check managed mapping reopen, original project/media bytes, root launcher refresh, prior runtime and launcher backup. Run real ZIP/manifest, incomplete payload, interrupted download and cancelled restart checks.
- [ ] Obtain a fresh read-only whole-diff review; fix demonstrated blockers and rerun affected gates. Push feature branch, create and attach PR, merge the reviewed SHA and publish only the checked ZIP and GIF.
- [ ] Verify public latest stable v0.8.8, canonical HTTP-200 ZIP/GIF and newly downloaded app bytes matching local SHA256. Keep old releases available.
- [ ] Return checkout to main, record PR/release/hash/evidence and remaining physical-projector check, push handoff and confirm clean status. Give Shane update/menu directions and preview/release links.

## Evidence ledger

Root records task status, deviations and test outcomes under ignored `out/halloween-check/progress.md`. This preserves the existing project's disposable-artifact convention. Planning was completed before production implementation; Shane already authorized subsequent execution and publication.
