# Lumen soft glass correction

Authorized scope: replace the rejected white/petrol Lumen treatment with soft dark glass and a slight blue cast. Keep the familiar 0.8.8 arrangement, all controls/features, saved projects/media, existing motion controls and the 1.9 GB storage ceiling. No framework replacement or changes to projected content.

## Research translated into this app

- [Microsoft Acrylic](https://learn.microsoft.com/en-us/windows/apps/design/style/acrylic) combines softened backdrop, luminosity/tint and texture. Its depth comes from the backdrop, not a bright border. The official dark Acrylic reference visibly preserves broad background colour while removing distracting detail.
- [Fluent materials](https://fluent2.microsoft.design/material) and [Mica](https://learn.microsoft.com/en-us/windows/apps/design/style/mica) distinguish stable foundations from transient surfaces. Mica is not live see-through glass. Keep the editor's stable workspace and use stronger opacity for text-heavy controls/popovers.
- [Apple materials](https://developer.apple.com/design/human-interface-guidelines/materials) distinguishes regular and clear glass. [Meet Liquid Glass](https://developer.apple.com/videos/play/wwdc2025/219/) describes adaptivity, light and functional layering. Use restrained reflections and readable controls; do not claim Apple's optical implementation or stack decorative glass on every layer.

Our palette values are authored choices, not official Windows/Apple tokens: foundation #242930, pane #2E343D, raised controls #3C4551, input wells #1C222A, text #EEF1F5/#D0D6DF, focus #93B5D5 and primary #A7C4DD with dark ink. The real desktop supplies colour variation. Avoid saturated petrol, broad white haze and thick shiny borders.

## Native composition boundary

An isolated Qt 6.10.3 QWidget/QOpenGLWidget proof tested documented DWM_TRANSIENTWINDOW with legacy blur explicitly disabled, a legacy empty region, whole-client legacy blur, and redirection alpha TRUE/FALSE. Calls returned success but yielded black/opaque grey on this machine. NONE plus the existing alpha path visibly transmitted a separate patterned window. System transparency was enabled and battery saver was off. Test source/evidence lives under ignored out/native-backdrop and out/lumen-petrol/native-acrylic-result.png.

Qt itself configures legacy alpha during accelerated-window initialization; this was explicitly reset after initialization in the proof. References: [Qt implementation](https://github.com/qt/qtbase/blob/v6.10.3/src/plugins/platforms/windows/qwindowswindow.cpp), [DWM backdrop types](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/ne-dwmapi-dwm_systembackdrop_type), [modern legacy-blur limitation](https://learn.microsoft.com/en-us/windows/win32/api/dwmapi/nf-dwmapi-dwmenableblurbehindwindow), [OpenGL widget composition](https://doc.qt.io/qt-6.10/qopenglwidget.html#limitations-and-other-considerations).

This increment retains proven **clear desktop translucency**, not native frosted Acrylic/refraction. Do not introduce unsupported composition APIs, screenshot capture, a new window host or a title-bar rewrite to disguise that remaining limitation. The native caption stays opaque/readable. Canvas/projector stay opaque.

## Execution and evidence gates

1. Revise Lumen's material roles: neutral translucent foundation, subdued moving reflection, quiet pane contours, recessed inputs, modest raised keys and reserved steel-blue action/selection accents. Keep layout and hit areas.
2. Build in the existing cached directory. Inspect the actual native window, including bright and dark background areas, menus and output controls. Reject opaque-grey/black composition regressions.
3. Package once to a fresh update stage. Run existing studio-theme, all-theme and scaled studio checks against that package; no new dependencies or mirrored colour tests.
4. Apply via the existing launcher. Exercise installed 0.9.9 -> 0.9.8 -> 0.9.9 with a synthetic saved mapping, verify protected project/media/runtime hashes, restore recent indexing and reopen original work.
5. Record measured storage, actual checks and remaining blur limitation in the handoff. Keep one useful prior runtime. Public release and physical-projector appearance are separate.
