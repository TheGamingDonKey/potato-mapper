# Halloween Drop: themes, attached surfaces and reactive characters

16 October 2026. **Proposed engineering design for review.** Shane authorized building and publishing after the animations and update are ready, and requested an engineering breakdown first. This document defines that new scope; it is not an implemented feature or a release claim.

Source: `C:\Users\mrbos\Downloads\Potato Mapper`. Starting commit `1574606`, C++17 / Qt 6.10.3 / OpenGL 3.3; latest locally recorded shipped version is 0.8.8. The prior [dark-forest research direction](../../research/2026-10-15-hollow-wood/RESEARCH.md) is accepted. New app settings, persistent topology and reactive actors expand that design substantially. Four separately assigned specialists inspected the actual UI/catalogue, topology/mesh, character rendering, and storage/rollback code. Their findings are incorporated below rather than collected into another large research pack.

## 1. What Shane's request means

| Requested behavior | Engineering term | Concrete interpretation |
|---|---|---|
| Halloween drop section | Content catalogue and event pack | A named Halloween collection containing actual installed scenes and characters. |
| Settings at the top of the software | Native menu and preferences controller | Add Settings between Edit and Help, using the existing Qt application. |
| Choose which special events to activate | Independently enabled event packs | Halloween now; an extensible registry for later Christmas/autumn additions. No empty Christmas content category. |
| Spooky, lively app | Application theme and decorative animation | Halloween colours, accents and a small animated header area; controls stay readable and stationary. |
| Editable built-in animations | Parameterized authored scenes | Embedded art/rigs expose useful composition, motion and behavior controls that save with the mapping. |
| Two panels attached at chosen edges | Surface topology / boundary seam | Store an explicit connection between selected edge intervals, including part of a longer edge. |
| An edge connected to a point on another surface | Navigation transition | Store an explicit landing point and continuation path; this differs from sewing two boundary intervals. |
| Snap together | Explicit geometric alignment | Preview a move/rotation/scale operation separately from making a logical connection. |
| Skeleton walking literally around edges | Combined contour routing | Remove shared seam arcs and follow the exposed boundary of the connected panels. |
| Skeleton upside down | Tangent/normal orientation and grounded gait | The support edge determines the character's up direction, including ceilings and vertical edges. |
| Harry Potter tiny battle | Cartoon wand-duel scene | Small expressive wizard characters approach, face, cast, dodge, shield and counterattack. |
| Smart, active, reactive animation | Deterministic behavior state machines | Local code reacts to layout, opponent/spell positions and route availability. No language model or online AI dependency. |
| Don't cripple the app; roll back | Compatibility and recoverable delivery | Isolate the changes, protect original mappings/media, retain a working runtime, and test old-to-new update plus downgrade behavior. |

The dark enchanted forest remains the main atmospheric collection. The explicitly requested cartoon skeleton and wizard duel form a separate playful collection. This latest request authorizes cartoon art for those characters; it does not turn all five forest scenes back into cute icons.

## 2. Approach and boundaries

**Recommended: authored articulated art with a native scene/actor runtime.** Reuse the existing mapper, mesh, media, scene clock, output and updater. Add focused modules for preferences/catalogue, surface topology, actor behavior and styled draw packets. Static texture/cutout assets can enrich the art; the scene's pose, timing, routes and behavior remain controllable in the app.

Alternatives considered:

- Prerender every scene as a complete movie: suitable for some rich fixed shots, but fails the requested editable, surface-aware characters and constrained crossing behavior.
- Replace the mapper with a game engine: brings a different renderer/editor/runtime and migration burden. The inspected app has useful integration hooks, so a rewrite is unnecessary for this scope.

This drop provides parameter editing and authored behaviors, not a complete animation-authoring program. Future event packs reuse the catalogue and scene interfaces. Full automatic wall scanning, continuous geometric constraint solving and user scripting are outside this drop.

## 3. Settings and event-library behavior

Menus:

```
Settings
  Themes
    Original
    Halloween
  Special Events
    Halloween
    Autumn                         [existing real content]
  Reduce interface motion
```

Only one application theme is applied at once; multiple real content packs can be enabled. Turning Halloween on enables its category and applies Halloween appearance immediately. The user can then choose Original appearance while keeping the Halloween content enabled. Turning Halloween off restores Original only if Halloween remains the active theme.

Store theme, enabled packs, interface-motion preference and last valid browsing category in the existing application preference domain using QSettings. They do not enter project snapshots, Undo, or the mapping dirty flag. No automatic calendar activation is required.

Add a Category selector above Animation: All available, Core, Autumn, Halloween — The Hollow Wood, and Halloween — Cartoon Characters as applicable. Categories organize actual installed content; their positions are never persisted as animation identities. Enabling a pack changes browsing, not what is assigned to any surface.

**Disabling a pack must never remove saved artwork.** A loaded or currently selected animation remains renderable and editable through a Current animation entry even when its browsing category is disabled. Opening a project does not silently change global preferences.

The current picker equates row index with saved pattern ID (`main.cpp:175–176,382`). Filtering therefore requires `itemData(stableId)`, `currentData()` and `findData()` under signal blocking. Preserve IDs 0–18, append new scene IDs, and keep Dynamic FX IDs separate. Do not reuse an old ID for new artwork.

Halloween UI decoration lives in a fixed header/toolbar region, never over handles or projector content. One owned timer runs only while the decoration is visible, Halloween appearance is applied, the app is active and reduced motion is off. It stops on hide/minimize/deactivation. Reduced motion preserves static Halloween styling and does not pause projection. Existing Scene/media timers already wake periodically; this change promises no extra inactive decoration work, not zero whole-app wakeups.

Use semantic palette roles/object names for existing hardcoded label/button accents so local styles do not defeat the selected theme. Keep ordinary menu access, keyboard focus, checked-state visibility, text contrast and control dimensions.

## 4. Attachments: exact semantics

### Boundary seam — edge interval to edge interval

Right-click a surface, choose Attach, select a source edge interval, select the target interval, preview endpoint correspondence, then Apply. Endpoints are adjustable markers with normalized Start/End values. Reverse correspondence swaps the paired endpoints. Esc cancels without an Undo entry.

For the example, A's full short edge has interval `[0,1]`; B's chosen 4-unit portion of its 20-unit edge can be `[0.30,0.50]`. A source position maps using `q=(a-a0)/(a1-a0)`, then `b=b0+q*(b1-b0)`, or `1-q` for reversed correspondence. These are normalized relationships, not measured centimetres: the app has no physical-scale calibration. Projector pixels supply motion distance/size.

**A 2-unit segment cannot physically coincide with a 4-unit segment without scaling/deformation.** Logical attachment alone does not resize either surface. Offer an explicit previewed Fit selected surface to span operation for straight compatible edges: move/rotate/scale the selected surface, retain its mesh structure, validate the candidate, then commit geometry and attachment as one Undo transaction. The UI must state that Fit changes surface size. A rigid move/rotation alone cannot solve unequal lengths.

Bent mesh edges require evaluating the entire boundary curve. Matching two endpoints does not prove the curves coincide. If full correspondence is not geometrically continuous, show the discrepancy and leave a continuous-walk seam inactive until aligned or explicitly treated as a gap transition. Do not silently force arbitrary mesh points or run a constraint solver each frame.

A sewn seam changes the outside contour: split boundary arcs at the span endpoints, remove the shared arcs from the exposed walking route, and splice corresponding endpoints. The skeleton walks the resulting exposed combined contour. Traversing seam midpoints would reproduce the existing snake portals and would fail this requirement.

### Route link — edge/point to a point on a surface

Attach also offers Route link for an interior target or point connection. The user chooses one source crossing position, a target UV point, and a continuation anchor/path on the target. A span collapsed into a point is not invertible and cannot define a sewn exterior. Preserve that distinction in the UI and saved data.

The continuation supplies support tangent/normal; never guess a route by relocating the actor to the nearest edge. At forks, use a saved turn choice or stable seeded policy; avoid immediate backtracking when another route exists. At dead ends, stop and turn predictably.

Continuous crossing requires compatible visible geometry. Gaps default to Stop at gap; an explicitly selected hop/portal treatment may connect disconnected regions. The mapper does not project a fabricated floor through unmapped gaps. Link guides and selection markers are editor-only.

### Persistent identity and lifecycle

Add unique `routeUid` per surface, independent of existing `Surface.id`, because accepted old projects may contain duplicate legacy IDs. Never persist surface-vector indexes. Each attachment has its own UID, kind, enabled flag, both endpoint descriptors, and correspondence orientation. One record is traversable in both directions; do not store two reciprocal copies.

Missing topology in old mappings means no connections. Generate unique route UIDs for those surfaces without changing legacy IDs. In new-format files reject duplicate route UIDs, malformed/nonfinite anchors, invalid ranges, required unknown types and dangling references before replacing the open scene. Missing optional fields receive documented defaults, never guessed links.

Surface deletion removes incident links and dependent route anchors atomically; Undo restores them. Hiding temporarily removes a surface from traversal while preserving links. Duplicating gives a new route UID and no inherited external links. Reordering changes no identities. Editing mesh/corners/output size invalidates route geometry, not saved connectivity.

## 5. Shared routing and character runtime

Compile actual mesh-boundary samples and authored links into cached routes with cumulative output-pixel arc length, surface ownership, tangent and support normal. Rebuild on topology/mapping/visibility edits, not every frame. Use all boundary mesh segments; the old Dynamic tour currently exports only four corners (`model.cpp:239–243`), which is insufficient for bent-edge connections.

Keep the old nearest-surface Hot-Ass Potato tour unchanged. New characters consume explicit topology. They use one Scene-owned fixed-step simulation/shared clock and immutable frame result for editor and output. Pause/Speed0 freeze behavior timers, poses and spells; Replay resets the seed and encounter. Animation ticks never call checkpoint/touch or dirty the mapping.

**Skeleton rig:** original cartoon skull/jaw, rib cage, pelvis, articulated limbs, hands and distinct feet. Stance feet remain locked to route contacts; two-bone leg solving and an authored swing arc prevent skating. Pelvis weight transfer, arm counter-swing and head lag support the walk. Character height and speed use output pixels, not per-panel UV percentages.

On a bottom edge the inward support normal makes the skeleton upright; on a side it rotates; on a top edge it hangs upside down. A deliberate corner step/pivot or bounded rounded trajectory replaces an instantaneous 90-degree spin. Crossing a touching seam uses one pose clipped across participating surfaces, not two complete clones or a restarted gait.

Large actors spanning warped mesh cells require triangle intersection and barycentric/triangle-local mapping. The existing single-center `pixelOffset` Jacobian is explicitly approximate and is not sufficient for that guarantee. Reuse matching mesh diagonals and ensure overlap ownership prevents double-bright actors.

**Wizard duel:** tiny expressive Harry Potter-style wand combat, with original drawn cartoon artwork and readable silhouettes. A shared director coordinates approach, facing, anticipation, casting, recovery, threats, dodge/shield, countercast, retreat and reset. Spell origin follows the current wand tip; targeting follows the opponent's current position. Incoming trajectory/time-to-impact triggers a feasible dodge or shield. It is not an unchanging prerecorded exchange with decorative bolts.

Seeded authored choices keep exchanges reproducible while reacting to layouts/positions. Layout edits invalidate routes and deliberately re-anchor/restart actors at safe positions. No negative-time simulation is implied: expose route direction separately; existing FX Reverse remains unchanged. A duel need not expose reverse-time playback unless deterministic rewind is actually implemented.

## 6. Full Halloween content and meaningful editability

| Scene | Core moving action | Editable properties |
|---|---|---|
| Hollow Crown | Tree/roots become one load-bearing revenant; recognized body turns and recovers coherently. | Hero size/position, speed, wind, secondary foliage, palette and reveal intensity. |
| Lantern Procession | Different weighted travelers establish carrier/light coupling; one lantern continues unsupported. | Path layout, spacing/count within a bounded rig budget, pace, lantern sway/light and palette. |
| Mycelium Oracle | An arriving signal tensions layered bark; a cavity opens and attends before discharge. | Branch-path emphasis, propagation speed, opening intensity, material palette and surrounding motion. |
| Weaver Above | Canopy supports become articulated legs; planted pull affects a corresponding anchor. | Anchor placement, body scale, pace, tension response, broad silk amount and palette. |
| Returning Roost | A crow lands with weight; support disappears beneath fixed feet, then returns. | Perch placement, bird scale, pace, gap size within valid limits, foliage and palette. |
| Skeleton Walk | Appealing articulated skeleton follows exposed connected contours, corners and ceilings. | Route/component/start, direction, height, walking speed, bounded actor count and colour accents. |
| Wizard Duel | Two characters navigate, track each other and complete legible reactive exchanges. | Safe starts/routes, spell colours, pace, aggression/response tendency, character scale and seed. |

Expose only controls that actually affect the selected scene. Density changes secondary detail, not five copies of the Crown. User edits save/reopen and participate in Undo. Runtime poses, projectile lists, contact states, particle caches and clocks remain transient.

The renderer needs explicit fill/stroke/glow/material style: the current Dynamic path fan-triangulates filled polygons and outlines/glows every primitive. New artwork uses authored convex/decomposed pieces or proper prepared triangles, with deliberate holes/occlusion and optional shaded cutouts. Do not make bones/robes neon wireframes by reusing the old stroke path unchanged. Preserve the legacy path's output.

Five forest scenes must have actual changing anatomy/material/occlusion, not a detailed generated still with drifting fog. The earlier reference only preserved coarse anatomy amid changing contours; stationary tree-to-stag correspondence remains a production hypothesis to test. No critic score substitutes for moving output.

## 7. Project format, collection and rollback

Global preferences do not upgrade a mapping. Preserve version1 for legacy-only project content. New scene IDs/configuration, attachments or actors require version2, so 0.8.8 rejects the newer file rather than clamping new IDs or discarding unknown objects on save. The new app reads both versions.

Before overwriting a version1 file with version2, retain a distinct pre-Halloween recovery copy in a location that keeps its media references valid; verify preservation, then perform the existing atomic save. The rolling `.bak` is overwritten on subsequent saves and is not the permanent pre-upgrade copy. Failure to preserve leaves the original untouched. No automatic flattening to legacy format.

Both `PotatoProjects::validateDocument` and `Scene::restore` currently accept only version1 and need coordinated changes. Parse/validate a complete temporary project state before assigning surfaces, topology, scene configuration or actors to the live scene. New persistent fields must round-trip through scene snapshots, Save/Open and Undo/Redo. Do not consume a failed Undo snapshot or silently discard unsupported required feature data.

Project collection currently enumerates only `surfaces[].media`; embedded shipped assets remain in Qt resources. If any new scene/actor references an external asset, extend enumeration, relative-reference rewriting and preservation. Existing full-project-byte fingerprinting covers JSON fields but does not discover new asset references automatically. Preserve linked originals and collect only explicitly referenced data.

Runtime rollback is not data rollback. With a version2 mapping open, Use previous app version offers its preserved legacy copy, or the old runtime without a project. Keep the new version2 mapping intact. Repeated saves/updates must not replace the permanent compatible recovery copy.

## 8. Module boundaries

- Preferences/catalogue: application-level theme/pack state, stable scene metadata and UI signals; no Scene ownership of global preferences.
- Topology/routes: scene-owned connections, normalized anchors, compiled contours and route policies; independent of artwork.
- Actors: authored rigs, local behavior director, contacts and project clock; independent of menus and OpenGL widget lifetime.
- Seasonal scenes/render packets: layered native art and controls, prepared triangles/materials, shared by both canvases.
- Scene/persistence integration: transactional edits, snapshots, validation and media-reference ownership.
- Existing UI/Canvas: attach interaction, catalogue controls and rendering integration, with targeted changes rather than a whole-file rewrite.

Keep current C++/Qt/OpenGL, saved media behavior, old patterns, Dynamic FX, Potato Scan, display selection, blackout and updater. New typed settings and small modules should avoid placing all topology/rig logic in main.cpp.

## 9. Delivery order and proof of success

These are scoped increments of the authorized full undertaking, not permission to label the first increment the complete Halloween drop:

1. **Event/theme foundation:** real Settings flow, Halloween appearance, reduced motion, stable-ID categories and saved-work invariance.
2. **Topology and character slice:** partial boundary joins, explicit route links, previewed fit, one appealing skeleton on a rectangle and a touching T-layout, including underside and warped-edge behavior.
3. **Reactive duel:** two wizards share the same layout and complete an approach/cast/response/counter exchange with editable settings and frozen pause.
4. **Forest content:** complete one low-detail Crown loop before detail production; then finish all five distinct forest scenes and their controls. Art work can run beside isolated engine work once the shared interfaces are agreed.
5. **Integrated drop:** usable examples, actual moving preview, bounded performance measurements, independent review, extracted-distribution and real update/rollback rehearsal, then publish the checked app and verify public downloads. Publication is already authorized at task level; it is conditional on this completed scope and its evidence.

Acceptance must include:

- Halloween on/off and category filtering leave a frozen current mapping/output/settings/Undo unchanged; disabled-pack content still opens and edits.
- The 2-to-4 interval example maps only the selected part of the long edge; fit is explicit; the skeleton follows the exposed combined boundary rather than the hidden seam.
- Corner, underside, reversed correspondence, warped edge, branch/dead-end, gap, overlap, hide/show, delete/Undo, duplicate legacy ID and reorder cases behave deliberately.
- Stance feet remain planted; one pose crosses a seam at constant perceived size/speed; no one-frame turn, duplicated actor, unrequested teleport or stray drawing outside mapped output.
- A wizard aims from the current wand position; opponent response changes with trajectory/layout; pause and replay are deterministic; live mapping edits do not leave stale navigation.
- Each forest scene's central event is readable at normal/reduced viewing size and across consecutive loop joins. Character charm and forest dread are judged separately.
- User edits persist and undo; animation ticks remain clean; editor/output show the same pose. All existing meaningful smoke, geometry, storage and update checks remain relevant.
- Bound actor/projectile/particle counts and post-clipping triangles. Cache routes on edits. Measure actual CPU/GPU behavior with visible editor/output, representative overlapping/warped panels and the Radeon840M; no promised FPS follows from a paper budget.
- New-format save retains a usable old-format recovery copy and original media; old app rejects version2 without changing it; malformed new data preserves the healthy open scene and history.
- Test the final extracted app and actual 0.8.8-to-new launcher update/reopen, interruption/incomplete payload and version rollback with synthetic projects. Physical projector appearance remains separate human/hardware evidence.

## 10. Status and review

This is the requested concrete breakdown and proposed architecture. No new application code, animations, settings, attachments, schema migration, package or release has been produced for it. Current installed apps and personal mappings/media remain untouched. Git source history and recorded v0.8.x tags are present; application rollback also requires the runtime/data paths described above.

Review points: the two attachment meanings; explicit fit rather than silent geometry distortion; global event appearance versus project content; native editable rigs rather than complete-video-only scenes; protected version2 content; and the full-drop acceptance boundary. Following design review, prepare the implementation plan around these increments and execute with disjoint specialist ownership and moving-output checks.

Primary API grounding: [Qt6.10 QSettings](https://doc.qt.io/qt-6.10/qsettings.html), [QComboBox item data](https://doc.qt.io/qt-6.10/qcombobox.html#itemData), [Qt stylesheet rules](https://doc.qt.io/qt-6.10/stylesheet-syntax.html). These support the API choices; they do not establish that this feature or its animation quality is implemented.
