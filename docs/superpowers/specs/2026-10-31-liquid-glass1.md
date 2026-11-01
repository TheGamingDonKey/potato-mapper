# Windows Liquid Glass 1 — whole-app optical theme

Authorized by Shane's October 31 request to research modern Windows/macOS design, then build this named theme. This supplements the approved familiar 0.8.8 layout; it does not authorize a layout rewrite or a public release.

## Evidence and decision

See [Windows/Apple primary guidance](../../research/2026-10-31-liquid-glass/windows-apple.md), [seven real app references](../../research/2026-10-31-liquid-glass/app-references.md), and [Qt/storage feasibility](../../research/2026-10-31-liquid-glass/qt-feasibility-and-safety.md). Apple WWDC25 describes lensing, spatial highlights, adaptive contrast and coherent layering. Microsoft distinguishes foundations, readable content and transient acrylic. Sketch's 2025 desktop redesign retained fixed panels after testing floating glass. These support an optical material system around a stable creative workspace, not moving tools or blurring all text.

The live 0.9.3 baseline was captured as `out/liquid-glass1/01-material-before.png`. Its nearly uniform rectangular plates, weak shell depth and orange active states do not meet this new brief. Preserve that Material option unchanged and introduce a distinct third profile.

## Fixed requirements

- Exact visible name: **Windows Liquid Glass 1**; internal ID `liquidglass1`.
- Classic 1 and Material remain selectable. Existing preferences migrate without silently replacing their chosen design. Select the new theme explicitly for the delivered preview.
- Preserve the 0.8.8 arrangement, existing button labels, sizes/order, feature reachability, context Delete/Undo, all animations, connections, actors and project formats.
- No edits to project/media/model/canvas/output rendering. Preserve saved work before restart. No scene dirty/Undo changes from choosing a theme.
- Stay below **1,500,000,000 bytes**, including existing counted concepts. No dependency/framework downloads, duplicate build trees, new full runtime copies unless capacity is verified.
- C++17, Qt 6.10.3 Widgets/Multimedia and OpenGL remain. This is app-rendered glass, not native Apple Liquid Glass or Windows DWM backdrop.

## Material architecture and palette

Use one smoked Regular-inspired family: foundation `#171A19`, glass bed `#272A29`, recessed fields `#1D2220`, primary text `#F2F3EF`, secondary `#AFB8B3`, pearl `#E8EEE8`, selected/focus jade `#8FD0B2`. Warn/error/status colors retain their semantic meaning. Seasonal accent choices remain Material-only and must not turn this profile orange/purple.

The full shell must visibly differ in grayscale structure as well as colour. Paint a continuous, low-frequency studio light environment in editor-relative coordinates. Refract that generated environment near curved glass boundaries; layer smoked body absorption, broad pearl reflection, narrow directional specular rim, inner meniscus and a soft lower caustic/contact edge. This environment is procedural, not a screenshot or wallpaper. Do not describe it as live content refraction. Real transient menus retain their bounded, opening-only editor sample and can lens that image at the edge.

| Existing region | New treatment |
|---|---|
| Window foundation/command and Output bands | Shared spatial light, polished smoked glass, restrained pearl edges; same geometry |
| Sidebar and dialogs | Thick coherent glass shells, stable dark text beds, no independent full blur layer on every child |
| Section headings/buttons | Compact polished rounded rectangles, curved highlight, double edge and local hover/press light; pressing reduces elevation |
| Fields/lists | Dark recessed interior with optical perimeter, crisp text, separate focus ring and selected row |
| Menus/combo popups/tooltips | Raised smoked glass, real readable action rows, visible separators/checks/shortcuts and keyboard semantics |
| Sliders/checks/scrollbars | Cohesive pearl/jade active states, visible disabled/focus treatment |
| Canvas/projector | Exact existing content; no theme effects |

Do not make every control a bright independent capsule. Concentric compact desktop rounding must fit existing allocations. The text region must remain quiet, even where glass edges are bright. Avoid graphic motifs, shimmer loops and unrelated background illustrations.

## Interaction, accessibility and resource bounds

Qt retains all hit areas, shortcuts, focus, menus and accessibility roles. Local pointer-reactive reflection may be added without timers or pointer tracking outside the app; no perpetual animation. Reduce interface motion must leave a static finish. Provide reduced-transparency and stronger-contrast editor preferences if the custom material does not automatically inherit platform settings; describe their scope accurately. Disable embellishment on disabled controls, recede inactive windows, preserve explicit focus beyond colour alone.

Keep raster/cache budgets bounded (existing 16 MiB cache, maximum approximately 2 MP per plate); no per-frame screen grab, blur or refraction. Cache keys must include theme, dimensions, scene-relative coordinates where needed, state, DPR and relevant accessibility preferences. No data/output ownership expansion.

## Acceptance gates

1. Research documents and this spec precede product implementation.
2. Third profile persists/reopens and repeatedly switches with existing profiles while preserving scene JSON/dirty/Undo, output, layout, dialog geometry and scroll.
3. Native checks cover labeled project actions at 960/1024/1280, real surface Delete/Undo, animation browser and connection reachability.
4. Grayscale optical/rim evidence distinguishes new glass from Material; actual native screenshots at the same layout verify whole-app coverage, dense inputs, menus and dialog. A critic score is not evidence.
5. Measure representative steady-state/repaint cost and ensure no capture/timer loop is introduced. Contrast checks use actual composited beds, with fallback preferences verified.
6. Validate a self-contained staged runtime without development PATH; reopen saved work through normal installed launcher, compare protected hashes, retain current runtime as rollback, verify storage below cap. No physical-projector or Store-certification claim.
