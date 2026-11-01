# Windows Liquid Glass 1: Qt feasibility and storage safety

Research only, October 31, 2026 (Eastern). No product source, build, running app, installation, project or media was changed by this investigation. The only created files are this document and ignored `out/liquid-glass1/storage-inventory.json`. Implementation and visual acceptance remain pending.

## Recommendation

The existing C++17 / Qt 6.10.3 Widgets / OpenGL stack can support a visibly different third interface theme with a shared refracted backdrop, frost, distinct optical rims and short control responses. Use a bounded CPU image compositor behind the existing real widgets. Keep Classic and Material intact and keep the 0.8.8 arrangement. No framework, Qt module, native DWM integration or downloadable asset is needed.

This would be app-rendered glass optics. Persistent chrome can refract a common app-owned light field; transient menus can refract a frozen sample of the editor. It cannot truthfully be described as live Windows desktop refraction or native Windows Liquid Glass. Merely lowering panel alpha and adding gradients would miss the requested distinction. The visual proof must show source features displaced through the plate, a frosted interior and sharper edge transmission, with text painted cleanly above them.

## Verified starting point

- Source: `C:\Users\mrbos\Downloads\Potato Mapper`, branch `codex/interface-themes`, HEAD `efbb852aca3a4958d1942e901ba2b76e03ab137c`. An existing modification to `proofs/surface-material/frost-menu.h` was observed and left alone.
- `CMakeLists.txt:2-6` specifies 0.9.3, C++17 and Qt Widgets/OpenGLWidgets/Multimedia/Network. `out/build-desktop/CMakeCache.txt` points to cached Qt 6.10.3 and Visual Studio 2022 x64. Reuse that build directory.
- `app/current.txt` is 0.9.3; `app/previous.txt` is 0.9.2. PID 9868 runs `app/versions/0.9.3/PotatoMapperApp.exe` with the saved recovery project. PID 11372 runs the separate `out/surface-material-proof/Release/PotatoSurfaceMaterialProof.exe`; protect that live proof tree.
- `src/material.h:85-88` excludes every QOpenGLWidget and widgets marked as projector output. `src/main.cpp:97-100` constructs separate editor/output Canvas instances. Preserve these boundaries: no glass drawing, effects, sampling timer or appearance settings belong in Canvas, models, scene serialization or projection shaders.
- Current ChromeMaterial uses event-filter painting below real Qt content, role-specific plates and a 16,384-KiB cache. It has no animation timer. Current MenuMaterial takes one bounded neighborhood sample on opening and clears it on hiding. Both already support returning to Classic.
- Current total from `packaging/Check-Storage.ps1`: **1,456,399,323 bytes**, leaving **43,600,677 bytes** before the 1,500,000,000-byte cap. This is insufficient for another approximately 66-MB self-contained runtime without cleanup.

These are current reads; older local handoffs describe historical evidence and do not substitute for running checks on a future glass build.

## Relevant Qt evidence

| Question | Primary evidence and implication |
| --- | --- |
| Can existing controls retain their interaction semantics? | [QMenu](https://doc.qt.io/qt-6.10/qmenu.html) provides real popup actions and submenu behavior. Retain those QMenus/QActions and paint their background; do not create clickable screenshot replicas. |
| How can a source image be distorted? | [QImage](https://doc.qt.io/qt-6.10/qimage.html) provides direct scanline access and explicit image formats/DPR. A bounded inverse pixel map can sample an existing image; bilinear sampling and rounded-edge masks are our implementation, not a built-in Qt glass API. |
| How are layers composited? | [QPainter](https://doc.qt.io/qt-6.10/qpainter.html) supports image source rectangles, clipping paths, gradients and composition modes. Use these for tint, frost, rim and specular overlays beneath Qt's foreground. |
| Can top-level translucency be enabled freely? | [QWidget](https://doc.qt.io/qt-6.10/qwidget.html#creating-translucent-windows) requires frameless windows for Windows translucency. Retain the main window and ordinary dialog frames; changing their flags would introduce a separate window-management task. Existing frameless QMenu handling can remain. |
| Can glass sample widgets every animation frame? | [QWidget::grab](https://doc.qt.io/qt-6.10/qwidget.html#grab) renders the requested region and its children. Calling it from persistent paint handling risks self-capture/reentrant painting. Snapshot only in an explicit preparation path with a guard. |
| Can OpenGL simply become transparent or use a blur effect? | [QOpenGLWidget](https://doc.qt.io/qt-6.10/qopenglwidget.html#limitations-and-other-considerations) documents stacking limitations; its framebuffer readback can stall the GPU. [QWidget::setGraphicsEffect](https://doc.qt.io/qt-6.10/qwidget.html#setGraphicsEffect) affects descendants and does not support OpenGL-based widgets. Keep the canvas opaque and exclude it from effects; never attach a whole-window blur. |
| How can responses stop when idle? | [QVariantAnimation](https://doc.qt.io/qt-6.10/qvariantanimation.html) interpolates values over a finite duration. Animate only background state, ending after 100-160 ms. Use the existing reduced-motion preference to stop and settle immediately. |
| What platform accessibility support exists? | [QAccessibilityHints](https://doc.qt.io/qt-6.10/qaccessibilityhints.html) exposes contrast preference in Qt 6.10; the installed header confirms that property, not a reduced-motion API. [QStyle](https://doc.qt.io/qt-6.10/qstyle.html#StyleHint-enum) exposes `SH_Widget_Animation_Duration`, where zero disables animation. Do not claim complete Windows reduced-motion synchronization without testing it; the explicit app preference is authoritative. |
| How are caches limited? | [QCache](https://doc.qt.io/qt-6.10/qcache.html) evicts entries according to supplied cost. Charge actual QImage bytes and coordinate transient/scratch allocations too. Cache maxCost is not a bound on the whole process or GPU memory. |
| Will the Windows file picker acquire app glass? | [QFileDialog](https://doc.qt.io/qt-6.10/qfiledialog.html#platform-notes) uses the native dialog by default; app widgets/QSS do not control its contents. Keep the native picker for this increment and state the boundary. Forcing `DontUseNativeDialog` would change a real user workflow and needs its own reviewed decision and validation. |

## Proposed compositor and interfaces

Add a small shared helper, provisionally `src/glass.h`, called by the current chrome/menu owners. Do not add independent render widgets or overlays that intercept input. Recommended responsibilities:

| Proposed interface | Responsibility |
| --- | --- |
| `GlassPalette` / `GlassRole` | Named foundation, shell, panel, well, control and transient parameters; cool neutral tint, ink, selection, disabled and focus colors. No scene colors. |
| `GlassBackdrop::ensure(logicalSize, dpr, paletteRevision)` | Generate one app-owned abstract light field and its blurred counterpart for a window. Return immutable QImages and a revision; invalidate on size/DPR/palette change. |
| `GlassBackdrop::sample(rootRect, role)` | Sample using root-window coordinates so adjacent panels share the same light field. Never restart a complete gradient in every button. No widget grab here. |
| `makeGlassPlate(source, rect, role, opticalParameters)` | Produce bounded background pixels with edge displacement, frost, tint and clipping. Cache using role, logical size, root position, DPR, backdrop revision and theme ID. |
| `paintGlassState(painter, rect, role, hover, press, focus)` | Cheap paint-time glint/rim/press overlay, separate from the expensive remapped plate. Hover never invalidates blur or refraction caches. |
| `TransientBackdrop::prepare(editor, globalRect)` | One guarded, clipped editor snapshot per opening; freeze it for the popup lifetime and release on hide. Fall back to the common app field if sampling is unavailable or exceeds budget. |
| `GlassMotion` | Short background-only hover/press interpolation on real interactive widgets, with QPointer lifetime guards and immediate settling for reduced motion, hide, disable or theme exit. |

The names are illustrative contracts, not implemented APIs. Prefer extending the current single event-filter owner and sharing helpers over installing a new app-wide filter for every control.

### Optical pipeline

1. Generate a continuous, low-frequency slate/blue/teal light field for the editor chrome. Use broad light areas, restrained color and one coherent lighting direction; reserve strong accent for selected or primary actions. The material should have enough structure to bend visibly without becoming a scene background.
2. Take the corresponding source rectangle, including a displacement/blur guard band. For permanent surfaces use the app field, never the rendered widget tree; this prevents text ghosting and a panel sampling itself.
3. Compute the signed distance and normal of a rounded-rectangle edge. Use an inverse map `samplePoint = sourcePoint + normal * displacement(distance)`, with displacement strongest in an approximately 6-12-logical-pixel edge band and a much quieter center. Clamp sampling to the padded source. This is an optical approximation, not a physically calibrated ray tracer.
4. Blend the source's softened version into the interior; preserve clearer, slightly magnified structure near the edge. Use a small bounded blur at reduced resolution, not six full-resolution neighborhood loops on every repaint. Convert input images to an explicitly supported format and respect bytesPerLine/DPR.
5. Composite a controlled dark absorption/tint layer to protect text contrast. Paint a bright upper/lateral rim, subtle opposite shadow and narrow specular crest, with a second restrained inner edge. Avoid an opaque fill that conceals the displaced source, and avoid large white streaks through text.
6. Let Qt paint the original labels, icons, shortcut strings, checkmarks, values, focus and accessible content. Blur/distort only the substrate. Any child-local QSS that fixes gold branding, chooser labels or updater headings must be inventoried so the new ink palette remains coherent.

Frosted wells need more diffusion and less gloss than raised controls. Disabled controls should retain a readable silhouette but suppress moving glints. Persistent panels should not brighten as a whole when the pointer crosses empty space. Keep layout radii/modulation within current geometry; painted movement must not change a widget's size, position or hit box.

### Bounded work and memory

- Keep shared field images at **1,000,000 physical pixels each**; cap material DPR to 2 and downsample larger windows. Retain a full and a soft field (approximately 8 MB combined at 32 bits/pixel).
- Use an approximately **8-MiB plate cache** charged by `sizeInBytes()`. Root position and backdrop revision matter: the present Material cache's size-only key is insufficient for a shared-coordinate glass source.
- Cap each transient source at **1,000,000 physical pixels**, with an explicit combined budget for the visible popup chain. Use one tiny shared noise tile, if any, instead of a full-size noise image per popup. An over-budget popup uses the shared field; opening a submenu must not allocate an unlimited history of captures.
- Proposed image accounting target: **under 40 MiB** for shared images, cached plates, visible transient substrates and scratch combined. Check capacity before allocating; degrade image resolution or use the field fallback. This is a target for owned buffers, not a measured process-memory claim.
- No new continuous timer for background blur, no desktop/screen capture and no `grabFramebuffer()` in hover animation. Paint-time response is a cached substrate plus a few vector overlays. Clear image caches when leaving Glass; release transient buffers on hide.
- During live resize/scroll, use the existing cached substrate scaled/clipped, then coalesce one optical rebuild after geometry settles (provisionally 80 ms). Rebuild on a real backdrop/DPR/palette change, never on unrelated scene animation ticks. Measure first before choosing thread complexity.

### Menus and dialogs

Extend `MenuMaterial` with an explicit glass branch, leaving Material's existing frost and Classic's restoration untouched. Preserve its existing hidden-only native-flag changes. A theme action can fire while the menu chain is still open: bypass the old painter immediately, clear old captures and defer native-flag work until hide. The next popup should be prepared at its actual clamped position.

Apply the glass substrate to existing owned QDialogs, QMessageBoxes, QInputDialogs, animation browser and updater client areas. Keep their real layouts, modality, keyboard/default/cancel behavior and native title bars. An owned dialog can use its own shared field aligned to the editor; a one-shot sample may be used only if captured before the dialog is visible and protected from recursive capture. A modeless dialog move should not start a scene-frame sampling loop. OS file pickers and native title bars remain platform surfaces. Unparented early startup errors are a separate fallback because current ownership checks intentionally exclude them.

## File mapping and compatibility

| Existing location | Bounded proposed change |
| --- | --- |
| `src/appearance.h:12-29` | Accept stable ID `liquidglass1`; preserve default Material, Classic and accent/legacy fallback. An unknown ID should still normalize to Material. No project-format migration. |
| `src/main.cpp:285-299` | Add third exclusive Interface theme action labelled `Windows Liquid Glass 1`; retain original actions and positions. Explain reduced motion truthfully. Keep Material accents separate; give Glass its own coherent default palette without rewriting the remembered Material accent. |
| `src/main.cpp:362-401` | Reuse applyAppearance's same-widget switch, revision guard, geometry/splitter/scroll preservation and persisted preferences. Retain the later-resize guard already added in 0.9.3. |
| `src/style.h` | Dispatch Glass QSS separately, preserving `potatoClassicStyle()` verbatim and the present Material result. Preserve size metrics, padding and control geometry while replacing ink/selection/background rules for Glass. |
| `src/material.h:55-143,154-268` | Keep ownership/output exclusions and semantic role discovery. Branch Glass into the shared compositor; do not redefine Material as Glass. Snapshot/blur preparation belongs outside paint callbacks. |
| `src/material-menu.h:20-30,93-198` | Distinguish the theme ID from the current `enabled()!=classic1` test, share optical helpers, clear captures on theme transitions and preserve real menu lifecycle. |
| `src/ui.h`, `src/main.cpp`, `src/updates.cpp` | Inspect local hardcoded foreground rules. Theme-aware palette use can repair incoherent labels; do not alter updater, save/open, rendering or project logic. |
| `tests/appearance-theme-check.h` and `tests/material-ui-check.h` | Extend existing native checks to three designs and test optical/source behavior, cache/motion lifecycle, all retained controls and exact output invariance. Preserve current Classic/Material checks. |
| `CMakeLists.txt` | Register a helper file if needed; no new linked framework/module. Reuse `out/build-desktop`. |

The present `enabled()` helper intentionally treats anything except Classic as Material. Adding only a preference/action would therefore produce a misleading recolor; the renderer and menu branches must identify Glass explicitly.

## Storage inventory: specific candidates, no cleanup performed

The ignored JSON records resolved absolute paths, all candidate file SHA-256 hashes, archive-entry hashes, duplicate-runtime comparisons and process/module observations at approximately **2026-11-01 02:53 UTC** (October 31 Eastern). These are cleanup candidates for the root task, not a command or completed deletion.

| Exact absolute candidate | Files | Bytes | Evidence |
| --- | ---: | ---: | --- |
| `C:\Users\mrbos\Downloads\Potato Mapper\out\releases\PotatoMapper-0.9.0-windows-x64` | 102 | 65,122,708 | Projects directory is empty; no reparse descendants. All 95 runtime files exactly match the retained `app/versions/0.9.0`, including the runtime inventory. Seven root packaging files remain accounted for. |
| `C:\Users\mrbos\Downloads\Potato Mapper\out\releases\PotatoMapper-0.9.0-windows-x64.zip` | 1 | 27,218,790 | All 102 file entries hash-match the above extraction. SHA-256 `2AFAC30EE98B7C2C878DED3581530F92BA253E2FF65D645712D254877DB8E407`. Redundant generated package; retained runtime remains protected. |
| `C:\Users\mrbos\Downloads\Potato Mapper\out\material-finish\robustness\.updates\download-zUeOLd\download.zip` | 1 | 27,892,368 | Historical failed mock-update archive. SHA-256 `40FAC1FE55D2AFC331915D025BE25750E7CA0BB6C03FDB603C66070E7A1276C1`. ZIP contents lack the required outer folder. `robustness.txt` records extraction failure; `robustness-final.txt` passes after the corrected package. Remove only this exact file, leaving fixture project/media and logs. |
| **Total** | **104** | **120,233,866** | No current process executable/command line references any candidate; both running Potato processes' readable module lists contain no candidate paths. This does not establish that arbitrary future file locks are impossible. |

Projected usage after those exact candidates: **1,336,165,457 bytes**. Reserving **68,000,000 bytes** for one staged runtime yields **1,404,165,457 bytes**, about **95.8 MB** under the cap before subsequent small docs/logs or build growth. The JSON and this document themselves add a small amount; run the real storage preflight again before allocating. Stage once, validate that stage and promote it without a second full extraction/copy. Do not create another ZIP unless its explicit peak-storage reserve also fits.

Do not broaden this to `out/releases` or `out/material-finish`: the 0.9.1 release tree contains three Projects files and maintenance/update state from real synthetic upgrade verification. `out/material-finish/extracted` and other evidence trees were not approved as candidates here. Protect `.git`, source, all `app/Projects` and referenced media, current 0.9.3, rollback 0.9.2, original 0.8.8 and other retained runtimes. The active isolated proof is also excluded.

`out/material-finish/progress.md` and `docs/0.9.2-material-finish.md` explicitly record that automatic approval review blocked a previous recursive removal of a temporary stage. The validated stage was later consumed by the normal authorized installation path. That history is not authorization to bypass the rejection through another shell, renamed path, helper or equivalent removal. No deletion was attempted here. If the root task receives a rejection for its concrete bounded cleanup, it should report the action and stated reason and stop that action; do not infer broad permission from this inventory.

## Risk and evidence gates for implementation

| Risk | Required evidence before calling the theme ready |
| --- | --- |
| Glass becomes another tinted gradient | A fixed source with a visible line/check pattern must demonstrably displace at rounded edges and soften in the center; compare source/plate and gradients, not screenshots' average color alone. Inspect the actual menu, toolbar, panel, input, selected/disabled row and dialog. |
| Text ghosting or unreadable transparency | Confirm foreground labels/icons remain undistorted and sharp; inspect minimum/hover/press/checked/disabled/focus states over bright and dark source regions. Measure worst-case contrast, and exercise the high-contrast fallback if implemented. |
| Layout or interaction regression | Three-theme cycles preserve identical widgets/actions/parents, left-sidebar/right-canvas arrangement, toolbar access at 960/1024/1280 widths, splitter sizes, scroll, a moved browser, keyboard focus, menus/submenus/Escape, combo popups and dialog Cancel/default behavior. |
| Reentrant sampling or memory/performance growth | Instrument capture/rebuild counts, cache bytes and active animations. One capture per popup opening, no idle optical work, no persistent scene-frame captures, bounded buffers during repeated menu chains, resize and repeated switches. Measure menu opening and interaction with scene/output both playing. |
| Reduced motion has no effect | With the preference enabled, trigger hover/press and verify immediate final state, zero active interface animations and no late glints. Switching away, disabling and hiding controls must stop animations too; scene playback is independent. |
| Theme touches output/history/data | Freeze a synthetic scene; require exact projector framebuffer equality across three-theme switches, hover animation and open popups. Confirm scene JSON, phase, clean/dirty state and real Undo/Redo checkpoints. Hash synthetic saved project/media before and after save/reopen/update. |
| DPI, position or source seams | Inspect 100/150/200% scale, a moved popup near available-screen edges, a resized editor, scrolled sidebar and monitor/DPR transition where available. Source coordinates should remain continuous and rims crisp. Label untested monitor configurations honestly. |
| Packaging destroys the baseline or exceeds cap | Rehash protected files immediately before a bounded cleanup, recheck candidates, rerun Check-Storage with reserve, build in the existing tree and validate the actual staged runtime without development PATH/plugin settings. Preserve 0.9.3 and synthetic update/reopen/rollback evidence before selecting a new runtime. |

First concrete implementation proof: one real toolbar region, one existing sidebar section and the real surface context menu using the shared compositor, while all three theme choices remain functional. Inspect optical identity and measure cost before extending the same helpers to all owned surfaces. A passing test suite or research document does not establish Shane's visual acceptance; the finished native app must still be reviewed. No public release, Store certification or physical-projector result is established by this research.
