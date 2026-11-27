# Windows Liquid Glass 1 Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development. Check each task against the linked spec; do not expand the layout or animation scope.

**Goal:** Deliver a researched, visibly optical third whole-app theme in the working native mapper.

**Architecture:** Extend the existing theme preferences and bounded ChromeMaterial/MenuMaterial painters. A shared code-native optical environment and edge lensing supply glass depth, while Qt owns labels, input and interaction. Reuse the existing build tree and generated staging runtime under the cap.

**Tech Stack:** C++17, Qt 6.10.3 Widgets/Multimedia, existing OpenGL.

**Spec:** [Windows Liquid Glass 1](../specs/2026-10-31-liquid-glass1.md).

## Global constraints

- Preserve existing profiles, familiar layout, all features, user projects/media, source/Git and rollback.
- Cap: 1,500,000,000 bytes. No framework/dependency downloads or new build tree.
- Internal theme `liquidglass1`; exact name Windows Liquid Glass 1.
- No native DWM/Apple API, public release, physical projector or Store certification claims.

## Review focus

Theme changes while a menu/dialog is open; compact toolbar overflow; optical cache invalidation after moving/resizing; high contrast/reduced transparency with readable inputs; preservation of user's real saved work during runtime promotion.

## Tasks

- [x] **1 — Register preferences and fallback behavior.** Root modifies `src/appearance.h`, theme actions in `src/main.cpp`, CMake/resource version to 0.9.4. Keep old defaults and compatibility key. Add reduced-transparency/increased-contrast actions scoped to chrome. Verify old tests still describe prior defaults.
- [x] **2 — Implement shared optical shell.** Renderer agent owns `src/liquid-glass.h` and `src/material.h`. Add bounded procedural environment, actual environment lensing at curved edges, pearl meniscus, contact edge and local pointer reflection without a timer. Classic/Material branches remain unchanged. Test actual rendered control detail, cache bounds and layout via task 4.
- [x] **3 — Complete controls and transient surfaces.** Menu/style agent owns `src/style.h` and `src/material-menu.h`. New profile gets cohesive readable QSS, optical menu using opening-only real editor sample, controls/selection/focus/tooltips. Use shared `PotatoGlass` interface from task 2; no background capture loop or geometry changes.
- [x] **4 — Theme regression and review.** Test agent owns `tests/appearance-theme-check.h` (or included focused header). Extend persistent preference/switch tests for third profile; scene/output/layout/Undo/geometry invariants; structural difference and fallback contrast. Run focused tests after implementation, then independently review failure-prone code. Root performs actual native visual inspection.
- [x] **5 — Build, stage and deliver.** Root builds in `out/build-desktop`, validates generated stage before reuse and refreshes only replaceable package files. Run staged native suites without developer PATH and inspect actual full app, menus and animation dialog. Preserve open work before close, promote validated runtime with existing launcher, verify hashes/storage, commit source excluding user proof whitespace, update README/DEVELOPMENT/ROADMAP and release handoff with precise evidence/limits.

Storage note: automatic review rejected the requested bounded cleanup with “blocked by policy.” No deletion or alternate deletion API will be attempted for those targets. Reusing an already-generated staging payload is an independent normal packaging action, not a claim that cleanup succeeded. If it cannot fit safely, stop promotion and deliver the verified build separately with the specific remaining limitation.
