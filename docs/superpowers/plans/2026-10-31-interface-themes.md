# Complete interface themes and Material refinement

> **For agentic workers:** Use superpowers:subagent-driven-development for disjoint settings, material and acceptance work; root integrates/builds/deploys. The user explicitly authorized continuing this design direction; no further design-selection loop is needed.

**Goal:** Make Classic 1 and Material distinct interface designs, then refine the approved Material finish without changing the familiar mapper layout or losing features/data.

**Architecture:** Separate interface profile (`classic1`/`material`) from the existing accent ID. Classic uses the tagged v0.8.8 stylesheet and bypasses Material chrome/menu painting and capture. Material uses the existing bounded painter, refined toward calm persistent surfaces and frosted transient surfaces. Switching styles retains the same widgets, project state and controls.

**Tech stack:** Existing C++17/Qt 6.10.3 Widgets/OpenGL; no dependency downloads or framework/native-backdrop experiment.

**Spec:** Shane's October 31 clarification that the old version and new version are separate complete themes; approved material proof and `docs/superpowers/specs/2026-10-29-windows11-visual-style.md`. Earlier layout/material preservation constraints remain binding.

## Global constraints

- Keep 0.8.8 layout, command order and all original/later operations.
- Preserve projects/media, project formats, output pixels, selection and Undo/Redo across style switches.
- Total project storage stays below 1,500,000,000 bytes; initial usage 1,386,498,481. Reuse `out/build-desktop`, stage one approximately 66 MB runtime, and avoid a second archive/extraction/deployment copy.
- Preserve the existing uncommitted indentation edit in `proofs/surface-material/frost-menu.h`.
- No public release/Store certification or new seasonal-content promise in this increment.

## Review focus

1. Classic accidentally keeps Material menus, painter backgrounds or capture after a live switch.
2. Old `ui/theme=classic1` preferences unexpectedly switch current users away from Material.
3. Repeated switches change scene JSON, output, geometry, selection, Undo/Redo or lose existing popup/browser objects.
4. Material polish flattens or obscures the finish the user approved, or introduces per-frame blur/capture.
5. Packaging duplicates exceed the remaining storage budget or replacement discards open user work.

## Tasks

### 1. Interface profiles and settings

**Owner/files:** interface agent: `src/appearance.h`, `src/main.cpp`, new `src/classic-style.h`.

- [x] Extract original v0.8.8 QSS as `potatoClassicStyle()`.
- [x] Add normalized `interfaceThemeId()` and `Preferences { interfaceTheme, accent }`, with `readPreferences(QSettings&)` and `writePreferences(QSettings&,const Preferences&)` for isolated preference checks.
- [x] Read `ui/interfaceTheme` default Material; read accent from `ui/accent`, falling back to legacy `ui/theme`. No write is required merely to migrate a read.
- [x] Settings → Appearance → Interface theme exposes `interfaceTheme_classic1` and `interfaceTheme_material`; existing `theme_<accent>` IDs remain under Material accent. Set app properties before stylesheet, persist live choices outside diagnostics, retain window/splitter/scroll context.

### 2. Real rendering separation and Windows-style polish

**Owner/files:** material agent: `src/material.h`, `src/material-menu.h`, `src/style.h`.

- [x] `potatoStyle(accentId="classic1", interfaceTheme="material")` dispatches exact legacy style versus Material.
- [x] Observe `potatoInterfaceTheme` live; Classic skips custom paint and menu sampling, restores opaque legacy menu treatment. Existing menus/dialogs/combos switch both directions without recreation or broken dismissal.
- [x] Preserve the approved frost/depth/rims while calming grain and decorative glare; retain strong text and clear hover/pressed/disabled/focus affordances, bounded caches, OpenGL exclusion and opening-only menu capture.

### 3. Acceptance, local runtime and handoff

**Owner/files:** test agent `tests/appearance-theme-check.h`; root version/resource/docs/build/deployment.

- [x] Extend real theme checks beyond palette strings: grayscale/edge differences between the same control's Classic/Material render, repeated live changes of existing menus/browser, unchanged layout/scene/output and real Undo/Redo behavior. Retain category/search/apply coverage.
- [x] Check preference normalization/migration/persistence using temporary INI QSettings through the production helpers; no new test-only application CLI/profile is required. Root verifies actual restart persistence through native UI.
- [x] Root builds 0.9.3, checks actual native Classic/Material renders and meaningful existing layout/feature/output suites; independent review checks source and screenshots.
- [x] Package one fresh local runtime with existing dependencies, validate its manifest and run that deployment. Save open user work before switching versions; retain 0.9.2 and original projects/media. Reuse the same staged runtime during deployment rather than making another full copy; no new ZIP is required for this local increment.
- [x] Record installed versus unpublished state, actual storage and verification limits in project handoff.

## Design sources

Microsoft's [materials](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/materials) distinguish persistent Mica-like foundation from transient Acrylic; [layering](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/layering) informs restrained separation. These are design references, not claims of native DWM integration. Qt's [widget stylesheet behavior](https://doc.qt.io/qt-6.10/qwidget.html#styleSheet-prop) and existing painter system remain the implementation route.
