# Lumen frost implementation plan

> Use superpowers:executing-plans for this approved increment.

Goal: implement the approved live frost study in the real Qt app, retaining 8.8 layout and all behavior.
Spec: design workspace `design/riff-four-2026-11-01/FROST-REVIEW.txt` and `lumen-frost-study.html`, approved in chat November 2.
Architecture: existing Qt Widgets/OpenGL and desktop-glass adapter; verify real native backdrop before integration. Reuse current build cache and installation pipeline.

Constraints: 1,900,000,000 bytes combined; preserve Git/source, user projects/media, working runtime and one rollback. No new framework, wallpaper imitation, screenshot capture, or changes to mapping/output. Keep pre-existing frost-menu.h edit untouched. Use current feature checkout to avoid duplicate build/runtime storage.

1. [ ] Resolve native frost in isolated proof. Existing failure: successful API calls yield opaque fill. Check system policy and window/composition styles. Test one cause at a time against a separate detailed window; foreground controls and opaque OpenGL must stay intact. Consider documented DesktopAcrylicController only if lighter documented integration cannot work and budget permits. Do not label clear alpha as frost.
2. [x] Port approved material into studio-themes.h/studio-widgets.h/desktop-glass.cpp: blue-grey translucent foundation, silver controls, inset fields, cobalt action, warm/iris icons. Preserve widget geometry and theme IDs. Verify backdrop enable/disable, accessibility fallback, theme switching and motion pause using existing studio diagnostics plus focused checks.
3. [x] Stage next local version, run packaged studio/all-theme/scaled checks, visually inspect actual native window/context menu, and exercise launcher update/reopen/rollback with synthetic saved mapping. Verify protected-file hashes and storage, then leave approved theme running and update README/ROADMAP/DEVELOPMENT. Public release/Store submission excluded.

Review focus: native composition lifecycle on theme switch/quit; inaccessible text over changing backdrops; high contrast/reduced transparency; unchanged opaque projector output; preserved saved work during launcher switch.

Execution record
- Baseline: 0.9.9 current, 0.9.8 rollback; branch codex/lumen-frost. Prior live preview verified. System transparency enabled, AC power, battery saver off. No dependency or production code change yet.
- Result: local 0.9.10 appearance installed with 0.9.9 rollback. Packaged 252 studio assertions at two scales, all-theme checks, actual rollback/reopen, native Close, 20 data hashes and both 95-file runtime manifests pass; about 1.30 GB total. Review's hover contrast issue corrected. Step 1 remains unresolved: native-only controller and raw host-backdrop probes still return solids. Failed integration/dependency excluded; clear alpha retained. See docs/0.9.10-lumen-finish.md.
