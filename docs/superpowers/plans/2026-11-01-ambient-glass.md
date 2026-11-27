# Ambient Glass 1 native implementation plan

**Goal:** Install the approved live Ambient Glass 1 design in Potato Mapper with Clear/Smoked glass, Pause, Speed and Light controls.
**Architecture:** Extend existing C++17/Qt Widgets chrome painting with one bounded procedural environment. Existing widgets, mapping renderer, scene model, project formats and launcher remain in place.
**Spec:** Approved browser prototype and handoff at `C:/Users/mrbos/OneDrive/Music/Documents/ChatGPT/Potato mapper/design/ambient-glass-live/`. User explicitly authorized actual app implementation on November 1, 2026.

## Global constraints

- Preserve the 8.8 layout and every existing feature, saved project and media file.
- Combined storage cap: 1,900,000,000 bytes. Reuse existing build tree; preserve current working runtime as rollback. No dependency installation or duplicate checkout.
- No desktop/canvas capture, OS wallpaper or wallpaper-like folded artwork for Ambient. Port original preview light fields.
- Theme preferences never edit a scene, create undo records or change projected pixels.
- Existing Classic, Material, Liquid Glass, AuroraClear and Transparent options remain available.
- Work on `codex/ambient-glass-theme`, preserving the pre-existing uncommitted proof header edit.

## Task 1: Native renderer

Owner: ambient_native_render. Files: new `src/ambient-glass.h`, `src/material.h`, `src/material-menu.h` only.

- [x] Add namespace PotatoAmbient with active(), accent(), radius(Role), clearCaches(), advance(double seconds), and paintSurface(QPainter&, const QRectF&, Role, const QPointF&, const QSizeF&, int state, const QString &family={}). Role aliases PotatoGlass::Role.
- [x] Select with potatoInterfaceTheme=ambientglass1; properties potatoAmbientGlass=smoked|clear, potatoAmbientPaused=false, potatoAmbientSpeed=.5, potatoAmbientIntensity=.65. Speed bounds .2..1.2, intensity .2..1.
- [x] Port backdrop.js paths/colours to cached light textures and one <=800x500 environment. Paint sampled chrome with inexpensive gradients/tints/rims; do not rebuild Aurora per-widget pixel caches each frame.
- [x] One ChromeMaterial-owned <=30fps timer; freeze simulation during pause, hidden/minimized editor, reduced motion, inactive theme, or opaque accessibility overrides. Resume without time jumps. Repaint only owned visible chrome and menus, excluding editor/projector OpenGL descendants.
- [x] Publish diagnostic potatoAmbientSeconds (double), potatoAmbientFrames (qulonglong), potatoAmbientRunning(bool), potatoAmbientEnvironmentPixels(qlonglong) for focused checks.
- [x] All ambient controls/dialogs/menus sample the same environment and preserve readable text/contrast. Clear and Smoked use preview tint strengths. Existing themes unaffected.

## Task 2: Preferences and controls

Owner: root. Files: `src/appearance.h`, `src/main.cpp`, `src/style.h`, `src/studio-widgets.h`.

- [x] Add ambientglass1 profile and clear-glass delegate/style compatibility, retaining every old profile.
- [x] Persist `ui/ambientGlass` (smoked default), `ui/ambientPaused` (false), `ui/ambientSpeed` (integer percent50,20..120), `ui/ambientIntensity` (integer percent65,20..100), safely normalized.
- [x] Settings > Appearance > Ambient Glass has Clear/Smoked actions named ambientGlass_clear/ambientGlass_smoked, Pause background motion named ambientPause, Adjust background action named ambientSettings.
- [x] Adjustment dialog has sliders ambientSpeed/ambientIntensity and visible values, note about reduced motion, plus Pause and glass choice. Keep it outside established workspace geometry.
- [x] Initialize app properties before ChromeMaterial; smoke mode never writes actual preferences. Update current open controls without creating scene mutations.

## Task 3: Checks and installed result

Owners: ambient_native_checks (new focused test header), root integration and installation.

- [x] Build a focused native `--smoke-ambient` check: actions, Clear/Smoked difference, real moving-vs-paused chrome, speed/intensity, bounded pixels, scene/output invariance, theme switching, accessibility and hidden/minimized pause, preferences reload.
- [x] Run meaningful existing theme/material/scene/project/connection checks and a scaled-display run. Independently review the changed code and actual native appearance.
- [x] Version 0.9.6. Update storage guard to authorized1.9GB; hash protected projects/media before installation. Stage into a fresh managed update session with package.ps1 -SkipZip, run that deployed binary without developer dependency PATH.
- [x] Validate payload, use real installed launcher update, retain0.9.5, reopen saved work and verify hashes/runtime inventory. Verify rollback and return with synthetic work. Avoid full duplicate installation wrapper under storage cap.
- [x] Leave actual native Ambient theme selected and open; capture screenshot. Update README/DEVELOPMENT/ROADMAP and concise evidence handoff. Commit only intended source changes. Local install is not a public release or physical-projector certification.

## Completion evidence

- Installed 0.9.6 through the existing launcher; 96 packaged file hashes verified.
- 54 native assertions and seven deployed suites passed. Four existing geometry/scene/project/dynamic check executables also passed.
- Visible screenshots proved moving chrome, stable test-media pixels and a frozen paused window.
- Real rollback to 0.9.5 reopened the synthetic saved project, then returned to 0.9.6. All 95 retained runtime files verified.
- All ten protected project/media files retain their original SHA-256 hashes; original recent-project index restored and original work reopened.
- Combined measured storage approximately 1.823 GB under the 1.9 GB cap. No public release, Store certification or physical-projector test claimed.
- Automatic approval review rejected deletion of the initial mislabelled staging folder. It remains intact and included in the storage count.
