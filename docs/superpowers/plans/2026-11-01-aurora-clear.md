# AuroraClear implementation plan

**Goal:** Install the approved clear, richly coloured Figma theme in Potato Mapper, with a separate transparent-glass option and the complete familiar 8.8 arrangement.

**Architecture:** Extend the existing C++17/Qt widget material painter and real actions. Add `auroraclear1` and `transparentglass1` preferences without altering project formats, canvas/output rendering, existing theme profiles or navigation. Shared stationary backdrop coordinates keep translucent panes coherent; cached chrome never samples the animated projector.

**Approved reference:** Figma file lEnOLt3t4PNUnFnLo7TlO6, workspace 16:3630, controls 16:3421. Local reference files: `C:/Users/mrbos/OneDrive/Music/Documents/ChatGPT/Potato mapper/design/figma-liquid-glass1/aurora-generator.cjs`, `aurora-clear-figma.jpg`, `workspace-preview.png`. The user approved implementation on November 1, 2026.

## Constraints and ownership

- Combined source/app and chat/design storage cap: 1,800,000,000 bytes, replacing the old 1.5 GB cap. Maintain measured reserves and bounded artifacts.
- Preserve source/Git, projects/media, current working runtime and one useful rollback. Save open work before restart. No public release or Store submission is part of this update.
- Preserve all commands, widget locations, shortcuts, Undo, connection operations, built-in animations and projector pixels. Theme preference changes must not dirty scenes.
- Reuse the existing build directory on a new codex branch rather than duplicating the checkout/toolchain within the storage cap. Leave the pre-existing proof-header edit untouched.
- Root: build/storage/versioning, integration, installed-runtime verification, promotion and handoff.
- Renderer agent: new painter and `material.h`, `material-menu.h`; independent profiles and bounded caches.
- UI agent: `appearance.h`, `style.h`, `main.cpp`, `studio-widgets.h`, `dynamic-ui.h`, `updates.cpp`; preferences, rich icon tiles, readable states and complete dialog/menu styling.
- Test agent: existing theme/material test headers only; real switching, geometry/action/data/output invariants. Root alone builds/runs tests.

## Execution

- [x] Implement two independent glass profiles from approved material geometry/palette; never replace existing UI widgets or media rendering.
- [x] Extend meaningful native checks for selection/persistence, old-theme restoration, layout/action retention, project cleanliness and unchanged paused output pixels.
- [x] Update storage guards to the authorized cap and include the entire known chat/design folder without double counting external preview manifests.
- [x] Build 0.9.5 using the existing cached Qt/CMake tree and package a fresh bounded candidate.
- [x] Run relevant theme/material/Studio/scene/FX checks against the packaged runtime. Inspect actual workspace, menus, animation browser and dialogs at normal/high DPI; correct visible discrepancies.
- [x] Validate synthetic saved-work reopen and update/rollback behavior, then preserve live open work and switch the installed launcher to the checked candidate. Keep 0.9.4 as the useful rollback and verify protected data hashes.
- [x] Record exact local installation/version/storage evidence; distinguish it from public release, Store certification and physical-projector testing.

## Review focus

1. Small-window toolbar labels stay visible and all old actions remain reachable.
2. Glass remains saturated and clear, without white/grey veils or heavy bevels; text/focus remain legible.
3. Switching profiles restores native Classic flags, icons and geometry; no cumulative signal handlers or scene edits.
4. Popup/dialog coordinates and cache keys follow movement/DPI/theme changes without expensive animation capture.
5. Update staging never changes user data or drops the last compatible working runtime.

## Completion evidence

Local 0.9.5 installed through the existing 0.9.4 launcher from the verified ZIP; actual saved-project reopen, rollback to 0.9.4 and return to 0.9.5 passed. Nine packaged native checks and four geometry/project/scene/dynamic checks passed. Actual screenshots and the final payload/data hashes are under out/aurora-clear; installed-proof.json records the selected runtime, persisted AuroraClear preference and protected files. No public release, Store submission or physical-projector result is claimed.

Automatic review rejected deletion of 28,811,604 bytes of compiler intermediates. They remain intact. Instead of creating another old-runtime copy, the verified local update/rollback ran against the authorized actual installation with synthetic work, followed by reopening the original project. This kept the combined workspace around 1.66 GB under the 1.8 GB cap. The synthetic recent-project index addition was reversed only after checking that all original entries were unchanged; all five protected file hashes match.
