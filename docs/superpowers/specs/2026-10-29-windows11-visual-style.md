# Potato Mapper: Windows 11 material finish on the 0.8.8 layout

**DRAFT — NOT AUTHORIZED FOR IMPLEMENTATION**
Prepared October 29, 2026. This is a design specification for review. It does not authorize rebuilding, replacing, launching, installing, or publishing the application. No visual shown here is a screenshot of an implemented redesign.

Shane's separate request to restore the existing 0.8.8 runtime is recorded in [the development handoff](../../../DEVELOPMENT.md). That restoration is distinct from implementing this draft.

## 1. Decision and boundary

Shane chose the familiar **0.8.8 layout** as the foundation and rejected the 0.9.0/0.9.1 interface direction. The new finish must make that familiar mapper feel carefully made for Windows 11: restrained frosted material, coherent light and depth, good typography, clear control states, and purposeful seasonal artwork. Changing a handful of colours in the existing stylesheet does not satisfy this brief.

The companion [interaction specification](2026-10-29-mapper-interaction.md) owns commands, discovery, selection, drag and drop, connections, and scene controls. This document owns their visual presentation. Both must be approved before implementation planning. The visual work must retain C++17, Qt Widgets, Qt Multimedia, and the existing OpenGL renderer. There is no framework migration, web wrapper, or replacement projection engine in this proposal.

**Already chosen by Shane:** the 0.8.8 spatial foundation, a more polished Windows 11 finish, documentation before another rebuild. **Proposed by this document:** material placement, measurements, component treatments, seasonal art direction, rendering budgets, and approval gates. Proposed values below are starting points for a mockup, not approved design tokens.

The source examined is `C:\Users\mrbos\Downloads\Potato Mapper`, branch `codex/seasonal-studio`; CMake identifies the current source as 0.9.1. The immutable layout reference is tag `v0.8.8`, commit `5d9c9743cc254cf983133098ca69243e987258c8`. This design does not require resetting the source to that tag: later mapping, persistence, actor, connection, and update work must be preserved while its editor presentation is changed. The existing 0.9.1 handoff reports local packaging and checks; those reports do not establish Shane's acceptance of that interface.

## 2. What “the 0.8.8 layout” actually preserves

The [0.8.8 main window source](https://github.com/TheGamingDonKey/potato-mapper/blob/v0.8.8/src/main.cpp) establishes a preferred window up to 1280 × 800 logical pixels, a 760 × 540 minimum, project commands at the top, a projector band underneath, then one horizontal splitter. Its left sidebar is 260–600 pixels wide, initially 310; the right side is the mapping canvas. The sidebar keeps the surface list above a separate scrolling inspector. The list's minimum and maximum heights are 100 and 190 pixels. Media, Appearance, Potato FX, Dynamic FX, and Mapping are collapsible sections in that source order. These are source facts, not a newly invented workspace. The companion interaction proposal groups scene-level controls after the surface sections while retaining the same sidebar structure.

Preserve that silhouette and reading order. Keep labelled New/Open/Save, Undo/Redo, + Surface, Load media, Potato FX, and Dynamic FX actions. Keep the projector destination, Refresh, Display settings, Start output, Stop, Blackout, status, and Potato Scan controls in the visible band. Do not bury them under a gear to recover height. Keep labelled Duplicate/Delete/Back/Forward actions, Visible and Lock position, the persistent surface list, and the left inspector. Later Connections and Whole scene actors controls join the collapsible left-side flow as specified in the interaction document.

The following is an **annotated layout schematic**, deliberately without rendered colour, blur, or invented application output. It is a layout contract, not a finished mockup:

```text
┌─────────────────────────────────────────────────────────────────────────────┐
│ Native window frame / title                         Minimise Maximise Close │ A
│ File  Edit  Settings  Help                                                  │
│ Potato Mapper | New Open Save Save as | Undo Redo | + Surface Load media …   │ B
├─────────────────────────────────────────────────────────────────────────────┤
│ Projector output  [destination ▾]                  Refresh Display settings  │ C
│ Start output   Stop   Blackout [B]     Output status                         │
│ Potato Scan    Play entrance   Skip   □ On output start                      │
├─────────────────────────┬───────────────────────────────────────────────────┤
│ Surfaces                │                                                   │
│ ┌─────────────────────┐ │                                                   │
│ │ Selected surface    │ │          EXISTING MAPPING CANVAS                  │ D
│ │ Other surfaces      │ │          Same content, handles, and               │
│ └─────────────────────┘ │          projection geometry                      │
│ Duplicate Delete Back  │                                                   │
│ Forward  Visible Lock  │                                                   │
│ ───────────────────── │                                                   │
│ ▾ Media               │                                                   │ E
│ ▾ Appearance          │                                                   │
│ ▸ Potato FX           │                                                   │
│ ▾ Mapping             │                                                   │
│ ▸ Connections         │                                                   │
│ ▸ Whole scene actors  │                                                   │
│ ▸ Dynamic FX          │                                                   │
│     inspector scroll  │                                                   │
├─────────────────────────┴───────────────────────────────────────────────────┤
│ Contextual status / help                                                    │ F
└─────────────────────────────────────────────────────────────────────────────┘
```

**A–B:** one quiet shell material, native window behaviour, modest brand mark. **C:** a contiguous command surface; its rows remain recognisable. **D:** an opaque, neutral working stage; no wallpaper, frost, seasonal decoration, or shell shadow behind the mapped image. **E:** compact readable content surfaces, with actual controls rather than navigation tiles. **F:** low-emphasis information, never the sole location of an output warning. Wrapping an action row at narrow widths is allowed; dropping its labels or clipping commands is not. Final compact behaviour is governed by the interaction spec.

The rejected three-column Studio organisation, a separate right inspector, and a permanent bottom library are not part of this visual design. The temporary content chooser can use richer previews without consuming the canvas whenever it is closed.

## 3. Reference vocabulary and concrete visual target

Windows 11 does not use one glass treatment everywhere. Microsoft's [materials guidance](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/materials) separates the opaque wallpaper-tinted Mica foundation, frosted Acrylic for transient surfaces, and Smoke behind blocking dialogs. [Mica guidance](https://learn.microsoft.com/en-us/windows/apps/design/style/mica) places it behind the application and favours visibility near the title bar; [Acrylic guidance](https://learn.microsoft.com/en-us/windows/apps/design/style/acrylic) warns against stacked translucent panes and describes solid fallbacks. These roles inform this design; WinUI brush names are not Qt stylesheet features.

The desired gloss is **controlled light on a surface**: a fine upper-edge highlight, a quieter lower edge, a very small luminance gradient, and modest separation between shell and controls. It is not wet chrome, neon outlines, bevelled plastic, or a white shine crossing every button. In a still image, the interface should read as layered material even before a seasonal accent is noticed.

Primary visual references, inspected as documentation references on October 29, 2026:

| Reference | Direct image / page | What to borrow and what to leave behind |
| --- | --- | --- |
| Microsoft Mica material illustration | [Mica header image](https://learn.microsoft.com/en-us/windows/apps/design/style/images/materials/mica-header.png) | Borrow the subtle, broad material variation and the distinction between base and content. Do not copy its application layout. |
| Microsoft Mica application example | [Light Mica example](https://learn.microsoft.com/en-us/windows/apps/design/style/images/materials/mica-light-theme.png) | Use to judge integrated shell and readable content. It does not require Potato Mapper to adopt NavigationView. |
| Microsoft Acrylic sample | [Frosted material illustration](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/images/materials_acrylic_hero_1880.png) and [light Acrylic example](https://learn.microsoft.com/en-us/windows/apps/design/style/images/acrylic-light-theme-base.png) | Borrow blur, tint, and subtle grain for menus/chooser only. The mapping canvas must remain exact and opaque. |
| Microsoft shadow anatomy | [Fluent elevation illustration](https://fluent2.microsoft.design/_image?f=png&h=860&href=https%3A%2F%2Ffluent2websitecdn.azureedge.net%2Fcdn%2Fdesignlanguage-elevation-shadowsystem-03.DSL226Kd.webp&w=1536) and [elevation guidance](https://fluent2.microsoft.design/elevation) | Borrow coherent direction, stroke definition, and ambient depth. Avoid giving every control a floating-card shadow. |

These are third-party reference images, not Potato Mapper mockups, reusable product art, or proof of Qt feasibility. Keep them linked; do not add copies to the package or repeatedly download reference boards.

**Material reference: layered Windows 11 surfaces and soft edge depth.**

![Microsoft Windows 11 Mica material reference](https://learn.microsoft.com/en-us/windows/apps/design/style/images/materials/mica-header.png)

**Material reference: visible Acrylic frost and blurred light behind a readable surface.**

![Microsoft Acrylic material reference](https://learn.microsoft.com/en-us/windows/apps/design/style/images/acrylic-light-theme-base.png)

Judge the future appearance against these material qualities while retaining the 0.8.8 arrangement. These component references alone do not establish a finished application design; Gate 1 requires an app-specific visual proof.

## 4. Material roles and proposed tokens

All dimensions are **logical Qt pixels**, scaled by device pixel ratio when rasterised. The first approved mockup must show light, dark, inactive, and solid fallback treatments. Neither light nor dark mode changes the projection background or a surface's media brightness.

| Role | Treatment | Depth and edge | Fallback / limit |
| --- | --- | --- | --- |
| Window foundation | One system Mica backdrop where Windows and Qt composition permit; visible in shell strips and outer gutters | Native frame and window shadow | Opaque neutral base on unsupported systems or disabled effects |
| Project/menu/output bands | Shared base with a light content wash; visually contiguous rather than three unrelated bars | One separator at functional boundaries; no shadow per row | Painted solid material with the same hierarchy |
| Sidebar and inspector | Opaque/near-opaque neutral content surface; shell material may show only around it | Fine boundary; section headers sit in the surface | No desktop or scene image visible behind labels |
| Surface list | Slightly inset solid well; selection carries a small indicator and text emphasis | 1-pixel edge, 4-pixel corner starting point | Stable visibility/lock/selection in all themes |
| Buttons, fields, sections | Subtle top-to-bottom luminance change and edge highlight where useful | 1-pixel border, 4-pixel control radius; no routine shadow | Same geometry with solid fills |
| Menus and temporary library chooser | Acrylic-like frosted transient layer, if native composition is proven; conservative tint keeps text stable | 8-pixel overlay radius, one soft ambient shadow plus edge stroke | Opaque elevated fill; no screenshot blur of live projection |
| Blocking dialogs | Readable solid dialog over restrained Smoke within the editor, if needed | Highest editor elevation; preserve native dialog behaviour | No dimming or effect applied to projector output |
| Canvas surround | Matte, opaque, neutral | Clean rectangular stage boundary; round only an outer wrapper if it does not crop content | Identical stage pixels in every editor theme |
| Seasonal art | Small, bounded illustrative area in chooser/settings or shell margin | Sits within a defined material role | Removed in high contrast and optional minimal appearance |

Proposed starting token set:

| Token family | Starting values | Purpose |
| --- | --- | --- |
| Spacing | 4 / 8 / 12 / 16 / 24 | Icon-label gap, related controls, section padding, group separation, dialog separation |
| Sidebar geometry | 310 initial; 260–600 retained | Match 0.8.8, permit readable longer controls |
| Control height | 32 typical; 28 compact surface actions; 36 section heading | Desktop density without the old 11-pixel action labels |
| Radius | 4 controls/list; 8 menus/dialog content; native outer window | Coherent geometry, no pill treatment for ordinary commands |
| Hairline | 1 logical pixel, aligned for DPR | Material edge rather than heavy boxes |
| Material highlight | 4–7% white in dark mode; 45–65% white in light mode, composited once | Subtle top edge; exact values require contrast/visual review |
| Control gradient | 2–4% luminance difference across height | Restrained sheen, not a large colour ramp |
| Elevation | 0 shell/content; menu/dialog ambient blur 16–24, y offset 6–8, opacity 14–22% starting range | Clear transient hierarchy; platform frame owns the main window shadow |

These shadow numbers are a Potato Mapper starting proposal, not copied Windows platform tokens. Microsoft's [Windows layering guidance](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/layering) and [Fluent elevation guidance](https://fluent2.microsoft.design/elevation) establish hierarchy; native DWM shadows take precedence. Do not combine a DWM shadow with an extra painted window shadow.

Colours must resolve through semantic roles—base, content, input, selection, focus, primary action, warning, error, text—rather than replacements across a string of hexadecimal values. Starting neutrals may be warm charcoal and off-white, with a restrained potato-gold accent, but palette selection is secondary to the material system. Every final value requires review in its actual composited background; an RGBA foreground's hex value alone cannot establish contrast.

## 5. Type, icons, density, and states

Use the installed **Segoe UI Variable** when available and correctly rendered by the supported Qt runtime; otherwise use Segoe UI. Do not bundle Windows fonts or assume their presence on Windows 10. Prefer regular for controls and semibold for hierarchy. Microsoft's [Windows typography guidance](https://learn.microsoft.com/en-us/windows/apps/design/signature-experiences/typography) recommends sentence case and gives minimum legibility guidance; this proposal uses 13 logical pixels for normal controls, 12 for secondary text, 14 semibold for section titles, and 18 semibold for the compact product title. Normal text is not made bold merely to look modern.

Avoid all-caps microtext as the primary way of distinguishing sections. Rename the visual casing to “Surfaces”, “Projector output”, and “Potato Scan” while preserving action meanings. Keep percentages and units aligned; use font metrics to size number fields, not fixed guesses that clip at 150% scaling. Long surface names may elide in the middle with a full-name tooltip; destination identity, destructive action labels, and essential warnings may wrap and must remain distinguishable.

Use a consistent 16/20-pixel functional icon family with matching apparent stroke weight and optical alignment. Existing SVGs may be retained where they meet this standard. Windows' [Segoe Fluent Icons guidance](https://learn.microsoft.com/en-us/windows/apps/design/iconography/segoe-fluent-icons-font) supplies platform context, but a PUA glyph must not become an unexplained text symbol or a missing box on another machine. Prefer project-owned vector icons with explicit accessible names; any imported set needs its actual licence checked before incorporation. Icons supplement the labelled actions, particularly Delete, **Clear content — white grid**, Load media, output controls, and surface order. The proposed Clear command preserves the mapping; its historical 0.8.8 label was “Clear media”.

| Component/state | Required appearance |
| --- | --- |
| Default / hover / pressed | Button boundary is recognisable at rest; hover increases local contrast; press compresses luminance or sheen without moving surrounding layout. No hover-only discovery of essential commands. |
| Keyboard focus | A distinct 2-pixel focus ring with 1–2-pixel clearance; cannot be confused with selection or a hovered border. Retain it on sliders, menus, list entries, and section headings. |
| Checked / selected | Persistent indicator plus fill/weight change. Blackout displays its active meaning in text; selection is not a fully saturated seasonal rectangle. |
| Disabled | Muted label/icon and restrained boundary; never identical to enabled. A discoverable reason is provided where a selection or media type is required. |
| Destructive | Delete remains a normal readable action at rest, with clear warning emphasis when activated or confirmed; every delete control is not a permanent red beacon. **Clear content — white grid** remains visually distinct because it preserves mapping. |
| Slider / spin box | Visible thumb and track, selected value with units, clear focus; no oversized glow. Brightness/opacity appearance must not imply that the editor theme alters the scene. |
| Collapsible section | Chevron and title stay aligned; closed state remains obvious. Expansion preserves the persistent list and uses the inspector's existing scroll region. |
| Surface row | Selected, hovered, locked, hidden, and missing-media states are individually legible through icon/text/shape as appropriate. Translucent art never sits behind its name. |
| Output state | Stopped / Live / Blackout / unavailable have explicit labels and suitable icons. Colour reinforces state; it does not carry the meaning alone. |
| Error / tooltip | A restrained elevated surface with full readable text. Missing media and disconnected output messages must not disappear into a decorative status strip. |

Use [Fluent accessibility guidance](https://fluent2.microsoft.design/accessibility) as a check on contrast and scaling: aim for 4.5:1 normal text and 3:1 component/focus indicators against their actual adjacent surfaces. High contrast uses system colours, strong boundaries, and no material imagery. Check 100%, 125%, 150%, and 200% display scaling and enlarged text; a control may grow or wrap rather than clip. The nominal 760 × 540 minimum is a compatibility goal, not permission to shrink text to unreadable sizes; document any genuine limitation before changing it.

## 6. Themes are material and artwork systems

Keep the existing stable preference IDs (`classic1`, `halloween`, `autumn`, `christmas`) so changing the finish does not lose settings. Display names remain **Classic 1**, **Halloween**, **Autumn**, and **Christmas**. A theme describes shell texture/lighting, a small artwork treatment, and restrained decorative motifs. It does not alter button positions, output state colours, surface colours, animation settings, or available content. Enabled event packs and editor theme remain separate controls inside the Settings → Special events presentation defined in the interaction document.

| Theme | Material / artwork proposal | Motif and motion boundary |
| --- | --- | --- |
| Classic 1 | Fine neutral mineral/frost treatment, understated warm brand highlight, existing potato identity | No decorative animation required; the best baseline for comparing hierarchy |
| Halloween | Smoked-glass shell treatment; one small illustrated spectral-forest vignette or etched branch detail in the chooser/settings | Still artwork first; optional slow lantern luminance in that bounded area, never wandering ghosts behind controls |
| Autumn | Warm satin/frost material; a deliberate pressed-leaf or paper-cut botanical vignette with real shape and shading | Static leaf motif; no falling leaf particles across the work area |
| Christmas | Cool frosted shell with restrained warm point-light artwork in chooser/settings | Optional very slow, bounded light breathing; no snow overlay, sparkle trail, or flashing focus indicator |

Theme review must show differences with the accent colour temporarily neutralised: the artwork, surface lighting, and motif should still make each theme identifiable. This prevents another “four recolours” outcome. Conversely, switching themes must leave every label, affordance, and action in the same place.

Artwork is a later deliverable, not assumed available because a seasonal name exists. Begin with one small original vector or compact raster composition per theme; do not generate large wallpaper packs or video loops. Any optional UI animation is off under Reduce interface motion, OS animation restrictions, high contrast, battery-saving fallback, or active projection. Interface motion settings do not pause the user's actual scene. Proposed interaction feedback is 100–160 ms opacity/colour interpolation; an optional artwork loop is slow, local, and absent from the default performance comparison.

## 7. Qt and Windows integration boundary

Current `src/main.cpp` selects **Fusion** and applies a global stylesheet; `src/style.h` paints broad widget backgrounds and substitutes theme colours. That explains why adding rounded corners or replacement colours alone cannot establish native Mica or real blur. A global opaque QWidget rule can cover the OS backdrop completely.

Qt's [6.7 release notes](https://doc.qt.io/qt-6.10/whatsnew67.html) document the introduction of its Windows 11 widget style; the current 0.9.1 runtime contains `styles/qmodernwindowsstyle.dll`. Plugin presence does not prove the result will meet this design. Future implementation should inventory available runtime styles and visually compare its native fields/buttons with a narrowly styled custom material layer, using the existing [QStyle interfaces](https://doc.qt.io/qt-6.10/qstyle.html). The proposed path is **existing Qt Widgets + a small isolated Windows shell adapter + deliberate custom-painted material details where needed**. It must not replace every control with a drawing that loses keyboard, accessibility, menu, or text behaviour. Global QSS should not compete with native drawing for the same property.

For supported Windows 11 builds, [DWM_SYSTEMBACKDROP_TYPE](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type) and [DwmSetWindowAttribute](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmsetwindowattribute) provide a documented avenue for a long-lived window backdrop; that enumeration's minimum supported client is build 22621. This API support does **not** prove it will show through Potato Mapper's present Qt/OpenGL window composition. A bounded feasibility prototype must test that before the material is promised. Preserve the normal frame, resize borders, snapping, caption buttons, and maximised behaviour. Do not adopt undocumented backdrop attribute numbers to cover older builds.

Microsoft's alternative [Win32 MicaController example](https://learn.microsoft.com/en-us/windows/apps/desktop/modernize/ui/apply-mica-win32) requires Windows App SDK integration/runtime arrangements. That added deployment domain is not the default for this portable app. Adopt it only after a separate demonstrated need and approval; do not smuggle it in as a stylesheet dependency.

The canvas remains an opaque `QOpenGLWidget`. Qt documents [transparency/stacking restrictions](https://doc.qt.io/qt-6.10/qopenglwidget.html): making it see through widgets underneath does not work as ordinary QWidget transparency does, and forcing AlwaysStackOnTop breaks overlay ordering. Do not apply blur/shadow effects to a common parent containing the canvas or enable whole-window transparency as a blanket workaround. Any necessary top-level composition change belongs in the isolated feasibility proof and must preserve opaque canvas pixels, overlays, frame behaviour, and output. No shell theme work enters the GLSL projection pass, media decoding, scene JSON, animation clocks, undo records, geometry caches, or output window. A separate editor-only material component may observe settings; it must have no authority to rewrite scene data.

## 8. Performance, fallback, and storage contract

The project has a **1,500,000,000-byte disk cap**, including external previews tracked by `packaging/Check-Storage.ps1`; this is not a RAM allowance. Preserve projects, linked media, recovery files, and the usable baseline. A cap failure means stop bounded generation/build work and report sizes, not delete arbitrary files. This documentation step creates no build, asset cache, or new runtime.

Proposed additional limits for later review: at most 8 MB of new shipped decorative art, 16 MiB of bounded decoded shell/art caches, and 32 MiB added steady-state process memory attributable to the appearance feature. These are acceptance budgets to measure, not current measurements. Cache keys include scale/theme; release obsolete theme and DPI variants rather than retaining every selection.

The material layer creates **no continuous repaint timer while idle**. Gradients/noise are cached by size/DPI/theme; invalidation happens on relevant state changes, not each scene frame. No live desktop capture, full-window framebuffer readback, repeated blur passes, or animated library thumbnails run behind normal mapping. A transient chooser displays cached still previews; opening it does not create another scene render loop.

Measure the same simple scene, moving FX scene, and video fixture with output running, before/after on the same hardware. A 60 Hz presentation has about 16.7 ms for the whole frame; shell work is not entitled to that budget. Proposed gate: appearance adds no more than 2 ms to the 95th-percentile interactive UI handling/paint sample and no more than 5% to representative editor/output frame times relative to the same layout with solid appearance. Record CPU-side paint time, observed frame intervals, GPU timing if available, resolution, driver, and sample duration separately. If a baseline misses 60 Hz, report that fact; do not conceal it with an average or claim the theme achieved 60 FPS.

Fallback order: stop optional artwork motion → remove custom grain/gradient detail if necessary → use cached painted material with no live blur → use fully solid semantic surfaces. System transparency/high-contrast/power settings take priority immediately. Windows 10 and older Windows 11 builds receive the designed solid finish without missing controls. Inactive windows de-emphasise the shell while preserving legibility. Fallback must remain recognisably polished; it cannot revert to unstyled widgets. Native-effect failure is diagnostic information, not a launch failure.

## 9. Rejected interpretations and tradeoffs

1. **Reskin the 0.9.1 Studio:** rejected because it preserves the spatial change Shane rejected. A new gold/green/orange stylesheet cannot repair that mismatch.
2. **Reset the whole product to 0.8.8:** rejected because visual familiarity does not require discarding later compatibility, actor, connection, persistence, or updater work.
3. **Qt Widgets with a bounded native material adapter:** selected proposal. It preserves the established program and permits native shell behaviour, but demands real Windows/Qt composition proof and a deliberate solid fallback.
4. **All-Acrylic or a new WinUI/QML/web shell:** outside this scope. Full translucency competes with mapping and increases rendering/deployment risk; a framework rewrite does not follow from a request for a better finish.

Unacceptable results include a flat dark form labelled “Fluent”; four identical themes with different accents; empty glossy cards replacing direct controls; icon-only destructive actions; a reduced canvas to fit a permanent catalogue; tiny text used to avoid wrapping; smoky media or altered projector black; and an attractive generated screenshot that the actual Qt application cannot reproduce. A material prototype that loses window snapping or crops editor handles is also a failure.

## 10. Approval and evidence gates

**Gate 0 — this document:** Shane reviews this specification alongside the interaction document. All tokens and engineering routes remain proposals. Approval of prose and approval of visual appearance are distinct.

**Gate 1 — one focused visual proof, only after authorization:** produce a clearly labelled mockup of the actual 0.8.8 silhouette at 1280 × 800, then compact 960 × 640 and the retained minimum where workable. Show Classic 1 first, with a real selected-surface inspector, list entries, output states, one menu/chooser, and visible keyboard focus. Show a dark/light pair and the solid fallback. A schematic or generated concept may guide discussion but cannot substitute for an accurate native-looking layout. Shane's explicit visual approval is required before broad restyling or seasonal production.

**Gate 2 — bounded native feasibility proof:** after authorization, verify the chosen material route in the existing Qt/OpenGL composition. Capture native frame, resized/maximised/inactive states, a second monitor, popup clipping, DPI changes, and fallback. If true frost is unavailable, present the honest fallback beside the reference and obtain a decision; do not silently call a flat fill Acrylic. Preserve an easily restored working baseline.

**Gate 3 — usable application increment:** implement one complete Classic 1 appearance using approved measurements; then review the three seasonal treatments before expanding assets. Validate actual interaction and output isolation, not just a stylesheet's presence. Required visual evidence includes:

- Familiarity: one left sidebar, persistent list, collapsible inspector, canvas right, complete visible output band; no added right rail or permanent bottom shelf.
- Clarity: no clipped primary labels at reviewed sizes/scales; named output states and obvious focus/selection; readable disabled, hover, pressed, checked, and error samples.
- Material quality: at least base/content/control/transient layers are visually distinguishable; no excessive shine, muddy transparency, doubled shadows, or inconsistent corners.
- Theme substance: theme variants remain distinguishable without accent hue, while geometry and command positions stay fixed.
- Isolation: with a paused known fixture, capture editor and output before/after theme changes; projected content pixels and scene JSON remain unchanged. Editor-only material never appears in output.
- Responsiveness: record the bounded frame/paint/memory/disk comparisons above; solid fallback remains fully usable. No unexplained visual stutter or theme-switch hitch is accepted merely because a smoke check passes.

**Gate 4 — acceptance and delivery:** Shane's explicit visual approval is recorded separately from source checks, package checks, installation, public release, and physical projector review. A later package must be tested from its actual extracted runtime, with saved-work/update preservation checks appropriate to the changes. No release or installed replacement is part of the present documentation authorization.

The next concrete action is review of these two documents. The first future implementation milestone, if authorized, is a focused visual proof of Classic 1 on the preserved layout—not another complete unreviewed redesign.
