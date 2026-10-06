# Dynamic FX and projects implementation plan

**Goal:** Ship a runnable 0.8.1 with organized portable projects, launcher refresh, Hot-Ass Potato and Potato Focus.

**Architecture:** Retain installation protocol 1 and append scene-level dynamic settings. A bounded geometry engine supplies cached local polygons to a shared OpenGL renderer; project storage and native launcher refresh are separate modules.

**Tech stack:** C++17, Qt 6.10.3, OpenGL 3.3, existing MSVC/CMake.

**Spec:** ../specs/2026-10-06-dynamic-fx-projects-design.md

## Constraints

- Version 0.8.1; no camera work or new libraries.
- Preserve existing projects/media and stable pattern IDs 0-13.
- Reuse protocol 1 and previous-version recovery.
- User explicitly approved execution in chat; proceed through implementation without repeat approval gates.
- Proportional checks; no unrelated test banks or artifacts.

## Tasks

- [ ] Project storage: add src/projects.h/.cpp for managed Projects, bounded legacy collection, referenced media collection, duplicate handling and backups. Add a focused temporary-fixture test proving originals unchanged, independent reopening and repeated collection stability. Wire File/Open/Save/recent projects in src/main.cpp; inspect current local mapping without saving it. Improve loader failures without changing valid old formats.
- [ ] Launcher: extend src/launcher.cpp and add a native refresh helper. Include replacement/helper in the versioned payload via packaging; root selection remains current.txt. Check older-launcher installation, replacement retry/interruption and retained project/media files.
- [ ] Geometry: add src/dynamic.h/.cpp with group settings, local primitives, stage output, nearest-surface route and bounded perimeter fragments. First check empty/single/three participants, stage progression, finite geometry and repeatability with focused fixture assertions.
- [ ] Integration: add group state/timing/persistence to src/model.h/.cpp, controls to src/main.cpp and dynamic OpenGL drawing in src/canvas.h/.cpp. Add pause/save/membership/render checks to existing diagnostic workflow. Preserve independent existing FX.
- [ ] Finish: inspect motion stages, run final extracted diagnostics, rehearse update through an older launcher, request a focused whole-change review, fix actual issues, update README/DEVELOPMENT/ROADMAP, publish GitHub PR/release and provide download/preview.

## Review focus

Filename collisions and partial collection must preserve originals; unreadable/invalid projects must leave the scene unchanged; deleting a member must not invalidate geometry; two canvases must use identical time; root replacement must handle a still-running older launcher.

## Execution record

- October 6: Source inspection confirms File > Open calls Scene::load directly; the user failure remains unprovided. Approval covers folder management within the installation and both dynamic effects, with camera deferred.
