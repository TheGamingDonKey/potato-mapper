# Four Studio Themes Implementation Plan

> For agentic workers: use superpowers:subagent-driven-development for scoped implementation and independent review. Existing checkout, cached build and a new codex/four-studio-themes branch avoid duplicate runtimes; preserve the pre-existing proofs/surface-material/frost-menu.h change.

**Goal:** Install the four approved native themes as 0.9.7 and remove the two rejected choices without changing mapping behavior or user data.
**Architecture:** Extend the existing Qt material event filter, real widget stylesheet, icons/delegates and bounded chrome clock. Theme rendering never samples or alters projection content. Existing QAction/widget geometry and serialization stay intact.
**Tech stack:** C++17, Qt 6.10.3, existing OpenGL and cached MSVC/CMake. No new dependency.
**Spec:** Shane's November 1 approval plus `C:/Users/mrbos/OneDrive/Music/Documents/ChatGPT/Potato mapper/design/riff-four-2026-11-01/revised-themes.json`, live preview, workspace SVGs, and REVISION-HANDOFF.txt. Tidal Lens is implemented as presently reviewed; further art direction is deferred.

## Constraints
- Preserve the 8.8 toolbar/output/left-sidebar/canvas placement, all actions, projects/media and current working runtime until validation.
- Cap: 1,900,000,000 bytes, including design assets. Reuse out/build-desktop; package one fresh stage without a ZIP.
- Replace menu choices Transparent Glass 1 and Windows Liquid Glass 1 only. Preserve Classic 1, Material, AuroraClear 1 and Ambient Glass 1.
- Stable new IDs: lumenatelier1, nocturnecinema1, siennastudio1, tidallens1. Read old transparentglass1 as lumenatelier1 and liquidglass1 as nocturnecinema1. Unknown IDs remain Material. Preferences remain outside scene files.
- Shared existing ambient pause/speed/light preferences also control new themes via an additional Theme motion menu. Ambient Glass clear/smoked controls remain in their existing location and affect Ambient only.
- Save open unsaved work to a new recovery project before app restart. Keep 0.9.6 as useful rollback; no public release or Store submission in this task.

## Task 1 — native material system
- [x] New `src/studio-themes.h`: `namespace PotatoThemes`, `using Role=PotatoGlass::Role`, `isTheme(QString)`, `active()`, `currentId()`, `const Theme& current()`, `styleSheet()`, `paintSurface(QPainter&,const QRectF&,Role,const QPointF&,const QSizeF&,int,const QString& family=QString())`, `advance(double)`, `clearCaches()`, `opaque()`.
- [x] Theme fields are `QString id,name; QColor bg,ink,muted,panel,accent,primary,primaryInk,selection,selectionInk,cap,section,rail,light1,light2; qreal panelRadius,buttonRadius;`. Four authored values from approved assets. Do not reduce themes to one tinted material.
- [x] Family supports `toolbar` to distinguish Sienna's ink tool deck; button family names carry semantic icon family if needed. Painter uses state bits matching current material: hover=1, pressed=2, checked=4, disabled=8, focus=16, primary=32.
- [x] One <=800x500 cached original light environment, actual phase movement, pause handled by existing controller. Publish `potatoStudioThemeSeconds`, `potatoStudioThemeFrames`, `potatoStudioThemeEnvironmentPixels` for diagnostics. Cache clearing preserves phase. Reduced transparency/contrast is readable and static.
- [x] Style sheet covers real native controls and menus/dialogs, preserves existing sizing metrics and establishes dark text on light profiles, readable Nocturne, Sienna toolbar text, all control states.

## Task 2 — icon and list treatment
- [x] `src/studio-widgets.h` only: include new header and apply each theme's real Fluent glyph colours and tile anatomy. Preserve action/icon restoration for older themes, compact surface-row buttons and existing delegates' geometry/hit areas.
- [x] Surface/library delegates and their labels/selection/indicators use new theme tokens. No changes to scene controls, labels, item IDs or positions.

## Task 3 — integration and focused checks
- [x] Root integrates appearance IDs/migration, main menu/actions/local label colours, shared animation clock, material-menu routing, and style.h. Generic Theme motion dialog reuses existing controls; pause/speed/light do not dirty projects.
- [x] New `tests/studio-themes-check.h` diagnostic `runStudioThemesChecks(Window&,QApplication&,const QStringList&)`, CLI `--smoke-studio-themes`: actual four actions and missing old actions; temporary INI migration/roundtrip; fixed scene and undo preservation; same widget/layout order; native frame distinctions; movement and exact pause; static projection pixels; reduced-motion/transparency; context menu Delete/Undo; cache bounds and no real settings writes. Reuse existing smoke fixture/helpers, do not add test banks.
- [x] Adapt existing theme tests to the actual retained/new profile list instead of requiring removed actions. Build and run appropriate native material/theme/ambient/appearance/connection checks.

## Task 4 — package and install
- [x] Bump CMake and Windows resource to 0.9.7, configure/rebuild cached tree. Stage via package.ps1 -SkipZip under app/.updates, verify the actual deployed executable at normal and scaled DPI.
- [x] Capture and inspect all four actual native themes against approved studies; correct readable-state or material failures before switching the launcher. Preserve layout rather than copying preview-only sample labels/data into the app.
- [x] Hash protected projects/media, exercise update/reopen with synthetic saved work, validate installed and rollback inventories, reopen user's recovered work. Keep useful 0.9.6 rollback. Leave Lumen selected for user inspection.
- [x] Update README/DEVELOPMENT/ROADMAP and concise 0.9.7 handoff with actual evidence, storage and remaining limits. Source checkpoint; no public publishing claim.

## Review focus
Retired preference IDs must migrate without losing scene state. Light themes must not inherit white local label styling. Theme switching during open menus/dialogs must retain size/scroll/selection. Animation must not advance while paused/minimized or invalidate projection content. Package update must preserve personal data and a real runnable rollback.
