# Whole-app material finish implementation plan

> **For agentic workers:** Use superpowers:subagent-driven-development for the independent layout, material and acceptance-check tasks. Continue through integration and verification without another authorization loop.

**Goal:** Apply the accepted real Qt surface-menu material finish throughout Potato Mapper, retaining the 0.8.8 layout and every existing capability.

**Architecture:** Keep current models, project formats, OpenGL output and updater. Restore the tagged 0.8.8 two-toolbar/left-inspector/right-canvas shell, adapting later features into the existing sidebar and a temporary animation browser. A shared bounded painter layer provides material depth; QMenu retains actual popup behavior.

**Tech stack:** Existing C++17, Qt 6.10.3 Widgets/Multimedia and OpenGL 3.3; no framework migration or dependency downloads.

**Spec:** `docs/superpowers/specs/2026-10-29-windows11-visual-style.md`, `docs/superpowers/specs/2026-10-29-mapper-interaction.md`, narrowed to layout/material/access preservation. Shane explicitly approved the one-piece proof and authorized whole-app implementation October 31. The request does not authorize unrelated interaction redesigns, new seasonal art or public publication.

## Global constraints

- Total project storage must remain below 1,500,000,000 bytes, including external concept images.
- Preserve source/Git, personal projects/media, the working app and a useful rollback.
- Retain original labelled project toolbar, visible output/Scan band, persistent surface list and labelled surface actions, left scrollable sections and right canvas.
- Preserve newer animation, connection, actor, appearance and project compatibility functionality without the rejected permanent library/right inspector.
- The approved effect is app-rendered material; do not claim native Windows DWM Acrylic/Mica.
- Only root builds, packages, operates installed app and commits. Agents edit disjoint files.
- Work on `codex/material-finish` in the existing checkout to reuse its build/runtime and avoid a duplicate tree. Pre-existing two-space indentation edit in the proof remains intact.

## Review focus

1. Original commands become unreachable or move into overflow at ordinary window sizes.
2. Newer-format projects lose data when restoring an old presentation.
3. Global material painting leaks onto OpenGL projector output or blurs it.
4. Popup target/cancellation/Delete and Undo change unrelated surfaces or media.
5. Packaging/switching overwrites user work or exceeds the cap through duplicate runtimes.

### Task 1: Restore the familiar shell, retain newer capabilities

**Owner/files:** layout agent, `src/main.cpp` only. Root integrates test dispatch/version.

- [x] Compare tagged shell with current source and feature-access audit.
- [x] Restore project/output control order, original labelled sidebar controls and original section sequence; append Connections/Whole scene actors as collapsed sections.
- [x] Retain newer model/project/update functions and expose animation search/category/apply/drag in a temporary chooser.
- [x] Use `MaterialSurfaceMenu(QWidget*)`; share context/sidebar Delete operation with a real checkpoint; retain Connect/Disconnect.
- [x] Run layout/access and existing behavioral checks; review diff against tag and newer feature inventory.

### Task 2: Shared material rendering

**Owner/files:** material agent, `src/material.h`, `src/material-menu.h`, `src/style.h`.

**Interface:** `installPotatoMaterial(QWidget *mainWindow)`, `MaterialSurfaceMenu : QMenu` with existing proof ctor/exec API; retain `potatoStyle(theme)`.

- [x] Lift approved popup painter into a production component without modifying the proof.
- [x] Add cached procedural frost/grain/sheen/rim treatment to editor chrome, sections, controls, owned dialogs and popups. Retain normal Qt interactions and clear hover/pressed/disabled/focus states.
- [x] Keep static chrome cached; sample own editor only once for transient menus. Never run capture/blur per animation frame or decorate projector widgets.
- [x] Inspect actual full-window, expanded-sidebar, dialog and menu renders; correct concrete defects before acceptance.

### Task 3: Behavioral/layout acceptance and integration

**Owner/files:** acceptance agent `tests/material-ui-check.h`; root `src/main.cpp` test dispatch, CMake/version resource, docs/package as necessary.

- [x] Add a small `--smoke-material` check for restored spatial arrangement/output visibility, later controls, right-click target/Delete/Undo, media bytes and fixed projector pixels using disposable fixtures.
- [x] Root wires check, builds Release in existing `out/build-desktop` and runs it. Existing model/storage/animation/connection/appearance checks remain behavioral gates; adapt only tests that explicitly encode rejected Studio layout.
- [x] Review material/layout source independently; run extracted runtime and old-launcher upgrade/reopen with synthetic saved work and preserved bytes.
- [x] Hash personal project/media files before and after installation; stage a fresh local runtime and preserve 0.8.8 rollback before changing the version pointer.
- [x] Inspect the actual running installed app, save compact screenshots and report implemented/tested/installed versus public/physical/Store states accurately.

## Execution notes

Initial storage is 1,129,997,628 bytes. Reuse `out/build-desktop`; one staged package plus extraction/upgrade fixtures must be bounded and measured. Remove only freshly verified disposable outputs if necessary; no arbitrary Downloads cleanup. No public push/release or new Store package is required for this visual milestone.
