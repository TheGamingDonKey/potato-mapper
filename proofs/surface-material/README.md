# Surface-menu material proof

One authorized, runnable C++/Qt component for visual review. This is not an app update or a complete replacement UI. The installed launcher remains on 0.8.8.

## What it demonstrates

`generate-proof.ps1` reads the immutable `v0.8.8` main window, sidebar and style from Git into the ignored build directory. Their arrangement is retained. The existing renderer/model are compiled without editing their source. A small synthetic two-surface scene opens automatically; no personal project is loaded or converted.

Right-click a surface to see a real `QMenu` with its target name, the existing Load/Clear/Rename actions and an added Delete action. Delete and the existing sidebar button share the same checkpoint/removal operation. Undo restores the surface data. The header is skipped by keyboard navigation.

`frost-menu.h` captures only the editor's own widget once on menu opening, downsamples and blurs the local backdrop, then paints a rounded translucent panel, restrained grain, vertical sheen and a light rim. Qt paints the actual menu items and handles their normal popup interaction. It reuses the bundled `windows11` Qt style for item painting, with a palette-aware Fusion fallback. No plugin installation, new framework or UI engine is required. The existing OpenGL renderer remains responsible for the mapping canvas and projector output.

This is **app-rendered sampled frost, not Windows DWM Acrylic or Mica**. The sample is static while the menu is open. An attempted DWM path did not produce a validated working popup and is not part of this target. Full accessibility, high-contrast, multi-monitor/DPI, performance and production integration remain unverified. The complete interaction specification, including global shortcut focus routing and selection after Undo, is not implemented by this proof. It does not establish feature parity for a future full UI migration.

## Build and run

Use the existing Qt 6.10/MSVC toolchain, CMake, PowerShell 7 and Git; no dependency download is part of the proof. From the repository root:

```powershell
cmake -S proofs/surface-material -B out/surface-material-proof -DCMAKE_PREFIX_PATH='<existing Qt 6.10 msvc2022_64 directory>'
cmake --build out/surface-material-proof --config Release --parallel 3
.\proofs\surface-material\Run-proof.cmd
```

`Run-proof.cmd` reuses the existing 0.8.8 runtime DLLs and keeps the proof's session under `out/surface-material-proof/session`. Its QSettings organization/application names are separate from the installed app. It is a development launch helper, not a distributable installer.

Run the focused check with `Run-proof.cmd --proof-check`. It exercises the actual canvas context-menu event and real action/model operations: pointer target rather than old selection, clean cancellation, Delete, exact surface JSON/order/identity after Undo, Redo, preserved temporary media bytes, and unchanged fixed projector framebuffer while the material menu is open. QAction activation is programmatic; this is not comprehensive mouse/keyboard or physical-projector acceptance. Native screenshots and the check log are retained in the ignored proof output directory.

Next step: Shane judges this one visible component before any wider application changes. No release, Store certification or full redesign acceptance is implied.
