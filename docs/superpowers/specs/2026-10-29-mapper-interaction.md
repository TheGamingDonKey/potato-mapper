# Potato Mapper interaction contract

**DRAFT — NOT AUTHORIZED FOR IMPLEMENTATION.** 29 October 2026. Documentation for Shane's review alongside [Windows 11 visual style](2026-10-29-windows11-visual-style.md). Approval is required before implementing or installing this redesign. Shane's separately requested restoration of the existing 0.8.8 runtime is recorded in [the development handoff](../../../DEVELOPMENT.md); it does not implement this proposal.

## 1. Intent, evidence, and chosen foundation

Bring back the familiar **0.8.8 workflow**: labelled commands, one left sidebar, persistent surfaces, expandable settings, large right canvas, and visible projector controls. Retain later seasonal content, connections, and actors while making ordinary actions discoverable.

Inspected `v0.8.8` (`5d9c974`) and HEAD `1f3cd52` in `C:\Users\mrbos\Downloads\Potato Mapper`: `src/main.cpp`, `src/canvas.cpp`, `src/model.h/.cpp`, `src/connections.cpp`, `src/patterns.h`, `src/appearance.h`, and project guidance. This is source inspection, not running-app/projector testing. Shane's latest rejection supersedes previous 0.9.0/0.9.1 UI acceptance claims.

**Actual 0.8.8:** New/Open/Save/Save as/Undo/Redo, + Surface, Load media, Potato FX, and Dynamic FX are labelled top commands. A separate output band contains display selection, Start output, Stop, Blackout, and Potato Scan. The left sidebar contains a persistent list, labelled Duplicate/Delete/Back/Forward, Visible/Lock position, then scrollable Media, Appearance, Potato FX, Dynamic FX, and Mapping controls. Corners/mesh dragging, wheel zoom, middle-drag pan, arrow nudging, and F fit-view already exist.

**Important correction:** 0.8.8 right-click offers Load media, Clear media, and Rename. Current HEAD adds animation and connection actions, but **neither inspected menu contains Delete**. Adding right-click **Delete surface** is a proposed improvement, explicitly required here.

Alternatives: retain 0.9.1's rejected navigator/right inspector/bottom library, or revert everything and lose later features/newer-project support. Proposed approach restores 0.8.8's arrangement while adapting later features. Materials and dimensions belong to the companion specification.

Unless labelled “actual,” the following sections define **proposed behavior**, not verified implementation.

**Approval boundaries:** layout/label/feature-location parity restores the 0.8.8 workflow. Required access fixes add right-click Delete, truthful connection descriptions, and safe focus routing. Other functional proposals are separately reviewable: underlying-surface menus, transaction/cancel refinements, visible-corner-preserving Reset mesh, manual connection/span selection, per-link disconnect, enabled event packs, and application-wide emergency Escape. Those extensions are not claims about current behavior or prerequisites to understanding the familiar layout.

## 2. Command locations and scope

The menu bar remains File, Edit, Settings, Help. The project toolbar visibly exposes New, Open, Save, Save as, Undo, Redo, + Surface, Load media, Potato FX, and Dynamic FX. Commands use the same underlying action wherever repeated; labels and enabled states agree.

The output band remains visible above the workspace. The left sidebar keeps its list visible while only the settings below it scroll. Immediately below the list: Duplicate, Delete, Back, Forward; then Visible and Lock position. The inspector sections follow Media, Appearance, Potato FX, Mapping, Connections, Whole scene actors, and Dynamic FX. Headings explain their scope: **Selected surface: Porch window** or **Whole scene**. Selection changes update surface sections without moving scroll unexpectedly; whole-scene settings remain clearly identified.

Potato FX expands its section and focuses the current animation picker. A labelled **Browse animations…** opens a temporary chooser with categories, search, preview, Apply to selected surface, and Cancel. No permanent bottom library is required. Dynamic FX expands and scrolls to its scene section. Connect expands Connections. Keyboard users reach the same destinations through menus and Tab order.

Browsing never changes the mapping. Apply names its target; without selection, offer **Add surface with this animation**. Cancelling creates nothing. Replacing existing content keeps geometry and takes one Undo step.

## 3. Selection and context targeting

Maintain one selected surface for this increment. The list and canvas always agree. Selection, pointer hover, active handle, and keyboard focus are distinct visible states. Selecting does not dirty the mapping or enter Undo.

| Action or condition | Contract |
|---|---|
| Left-click visible surface | Select topmost hit surface; selected handles take precedence when directly grabbed. |
| Right-click visible surface | Select the surface under the pointer, then open a menu headed with its name. Never act on the previously selected surface elsewhere. |
| Right-click list row | Select exactly that row, including hidden or locked surfaces; use the same surface menu. |
| Overlapping surfaces | Normal hit targets topmost. **Select underneath** lists hit surfaces by name/order; choosing one selects it and directly opens that named surface's menu without another pointer hit-test. Shift+F10 or its list row can also target it explicitly. |
| Hidden surface | Excluded from canvas hits; remains selectable and editable through its named list row, marked Hidden. |
| Locked surface | Still selectable and eligible for content and organization actions; geometry handles are unavailable. |
| Blank canvas | Left-click clears selection. Right-click offers Add surface and Fit view, with no surface Delete or stale target. |
| Blank list area | Offer Add surface only; never reuse the current row as a hidden context target. |

Keyboard Menu/Shift+F10 opens the selected row's or selected canvas surface's menu; no selection opens the corresponding blank-area menu. Opening/dismissing a menu alone changes no project content.

Surface menus group directly labelled commands: Load media…, Choose animation…, Clear content — white grid; Rename…, Duplicate, Back, Forward, Visible, Lock position; Connect edges…, Manage connections…; then a separated **Delete surface**. Disabled actions remain readable and explain their reason in the status/help text. Delete is text, not an unexplained trash icon. Ellipses indicate a chooser or preview before application.

## 4. Delete, duplicate, history, and locks

**Delete surface** removes only the selected surface from the open mapping, along with its incident connections and Dynamic FX membership. It does not remove neighboring surfaces, their content, or any original/collected media file. Scene actor settings remain; routes recompute from remaining eligible surfaces. Undo restores surface identity, geometry, content references, order, visibility, lock, incident links, and membership atomically. Select the restored surface. Redo removes the same surface again. Deleting the final surface leaves an empty mapping with Add surface available.

Deletion is immediate and undoable, with a named status message: “Deleted Porch window. Undo restores the surface and its connections.” No repetitive confirmation is needed for an in-project removal. This contract contains **no filesystem media-delete command**. Clear content and removing a mapping surface never authorize deleting user media, exports, backups, or projects.

Duplicate makes a selected copy with a readable “copy” name, fresh surface and route identities, the same content/settings/mesh, and the familiar small output-coordinate offset. It inherits visibility and position-lock state, has no external connections, and is not silently enrolled in Dynamic FX. Same-path video sharing remains explicit. A hidden duplicate is identified in the list. One Undo removes only the duplicate.

Back/Forward move one stacking position; list order and canvas overlap update together. End-of-stack actions disable. Rename is available by double-click or F2, trims invalid empty input with an explanation, and preserves identity. Clear content returns to the white alignment grid while keeping geometry, appearance, and links; Undo restores the prior assignment.

Each completed drag, slider gesture, duplicate, delete, content assignment, or confirmed connection is one meaningful Undo transaction. A click without movement, invalid drag, cancelled dialog, view zoom/pan, selection, animation tick, or preference change creates none. A new project edit clears Redo; playback advancing or selection does not. Persisted playback-setting changes remain edits. History is bounded as today; reopening is not a promise to restore the previous session's Undo stack. Actions should read “Undo Delete surface” where the transaction is known.

**Lock position** protects corners, mesh, subdivision/reset, nudging, dragging, and geometric connection fitting. It allows selecting, renaming, visibility, appearance, content replacement, duplication, layer order, and explicit Delete. This is position locking, not archival protection. A tied group's movement is refused as a whole if any member is locked; show the blocking member's name. No partial group movement. Proposed logical link management is allowed without geometric change, but fitting requires unlocked affected geometry.

| Keyboard focus | Arrow keys | Delete/Backspace | Undo/Redo and letter shortcuts |
|---|---|---|---|
| Canvas | Nudge active handle, otherwise selected surface | Delete selected surface | Project history; B blackout, F fit view |
| Surface list | Navigate rows, never nudge geometry | Delete selected row's surface | Project history; B blackout, F fit view |
| Text/search/path/name, editable combo, spinbox, text dialog | Local navigation/value editing | Local text editing only | Local edit history, even when empty; never fall through to project history. B/F type text, never trigger projection/view commands. |

Editable controls consume these keys even when their text, selection, or local Undo stack is empty. Explicit menu commands remain available; lack of a local edit is never permission to delete a surface or undo another project operation.

## 5. Mapping, media, and timing

Start with Corners selected. Drag corners to match the physical panel; drag inside to translate. Choose Mesh points for local deformation. Invalid folds retain the last valid geometry. Escape during a drag restores pre-drag geometry without a committed Undo step.

**Reset mesh distinction:** actual code regularizes mesh UV coordinates against stored `Surface.corners` anchors; Fit can move mesh corner nodes independently, so actual Reset can move apparent physical corners. Proposed Reset preserves the four **currently visible boundary corner positions**, rebases the stored anchors to those positions, and regularizes the remaining mesh. Bent edges straighten; media/appearance remain. Preview changed geometry and affected connection continuity before Apply/Cancel, committing one Undo step. Invalid rebasing changes nothing. This is a proposed functional change, not current behavior.

With canvas focus, arrows nudge by one **output pixel**; Shift uses ten. Handle focus is visible. F fits the editor view without changing mapping coordinates. Wheel zooms around the pointer; middle-drag pans. Labelled controls provide alternatives. Editor guides never appear on projector output.

Load media applies the first chosen file to the selected surface; additional files create additional surfaces, matching the existing useful behavior. With no selection, create a surface after a valid assignment. Drop highlights its target before release; dropping on blank creates a new surface. A failed decode/file choice must retain the prior usable assignment and show the filename plus recovery action. Cancelling a chooser changes nothing. Replacing an animation with media keeps geometry. Stretch, Fit whole image, and Fill/crop remain surface choices; brightness, opacity, Normal/Screen/Add, and Reset appearance stay adjacent.

Videos show Play/Pause, Restart, seek position, duration, and Mute audio under Media. Images show no irrelevant transport. Actual source caches players by media path: surfaces using the same video share playback and mute. Preserve that behavior and display “Shared by 2 surfaces” where applicable. Do not imply independent playback for duplicates or frame-locked playback between different files. Video seek/play/mute are runtime transport; a future persisted-media-transport change requires its own design.

Native per-surface FX have their own saved speed, size, palette, density, flow, angle, reverse, soft edge, and pause settings. Labels adapt to the scene: Ghost count/Flutter, Forest lights/Awakening, Snowfall/Wind. Editor and output read the same scene phases, not separate simulations. **Whole scene actors** controls only the skeleton and wizard overlays; Dynamic FX and video retain their own controls. Actor enable, speed, size, and initial playback state are saved/undoable as in current HEAD; elapsed poses/frames are transient. Changing a theme must not restart any of these clocks.

## 6. Events, themes, and editable actors

Settings → **Special events…** presents installed content and editor appearance in separate labelled groups. Content includes Core patterns, Autumn/Thanksgiving, Halloween, and Christmas/Winter. Themes are Classic 1 (existing stable ID `classic1`), Halloween, Autumn, Christmas. Multiple content categories may be enabled; one theme applies. **Use matching theme** is explicit, never a consequence of assigning content.

These preferences persist locally, do not dirty the project, enter project Undo, or change projected colour/content. Reduced interface motion affects editor decoration only. Disabling an event category hides it from browsing but never unloads an assigned animation; its current controls and name remain reachable. Loading a project never changes the user's theme or enabled packs. Empty search results say “No matching animations” with Clear search/All enabled content; unavailable content is distinguished from an empty filter.

Keep every installed effect through Snowfall, not only the 0.8.8 scenes. Whole scene actors exposes Walking skeleton and Original wizard duel independently, plus labelled actor Play/Pause, Speed 10–300%, and Size 50–200%. Enabling a skeleton explains whether it will follow separate panel boundaries or a continuous connected contour. Wizards use eligible mapped surfaces. Hidden/fully transparent surfaces do not become invisible floors. Current actor controls are these bounded parameters; this proposal does not claim an already implemented full rig/path authoring editor, independently tunable aggression, or new motion quality.

Shared Snowfall remains a selectable surface animation; connected Snowfall surfaces use the existing synchronized settings/phase behavior. Show that group scope before an edit affects linked snowfall peers. Connections do not replace each surface's distinct media or animation. The wider October 16 forest/actor ambitions remain separately tracked: Mycelium Oracle, Weaver Above, Returning Roost, arbitrary point route links, and richer rig authoring are not silently declared delivered by this UI restoration.

## 7. Connect, preview, apply, disconnect

Current HEAD automatically proposes a whole source edge to an interval of a longer facing target edge. Its Fit implementation moves the **source edge mesh nodes**, not a whole-surface move/rotate/scale. Existing helper/tooltip descriptions disagree. The restored UI must state the actual operation consistently.

Proposed Connect edges opens a preview with named source/target surfaces and Top/Right/Bottom/Left edges, the target Start/End percentages, gap in output pixels, and highlighted endpoints. Source is the complete chosen edge in this increment; selecting an arbitrary source subinterval or interior point is not supported by the current connection data shape. Offer a suitable nearest candidate, let the user select another compatible edge/span, and validate before enabling Apply. Do not silently choose an unannounced neighbor.

**Keep mapped positions** is the default: create a logical link, change no geometry. Explain “Linked, gap remains” if the sampled edges are not continuous; a skeleton cannot walk across that gap. **Fit selected edge** explicitly previews changed source mesh nodes, including shared corner nodes. Other source nodes and the target remain fixed. Show before/after outlines. Refuse folds, occupied overlapping spans, missing identities, locked geometry, or insufficient mesh resolution with a specific correction. Nothing changes on refusal.

Apply commits the link and any fitted geometry as one Undo step. Cancel/Escape removes the preview without changing geometry, links, dirty state, or history. Partial seams remove only the joined target interval from the exposed walking boundary; remaining edge portions stay available. Guides are editor-only. **Tie group while moving** is separate from making a connection and names the affected group.

Manage connections lists each named neighbor and span, with continuous/linked-with-gap/tied status. **Disconnect this link** removes one selected connection; **Disconnect all from this surface** explicitly names the count. Disconnection previews the affected boundary, then Apply/Cancel; it preserves fitted geometry and content. Undo reconnects the same span. No misleading suggestion that disconnect automatically restores a pre-fit shape; Undo of the original Connect transaction performs that restoration when still available.

## 8. Output, emergencies, cancellation

| State | Visible behavior |
|---|---|
| Output off | Display selector and Start output available; Stop disabled; clear status. |
| Live output | Selected display named, Stop and Blackout always reachable; changing destination requires explicit restart. |
| Blackout | Persistent checked label/status in editor; projector black; mapping remains visible for editing. |
| Potato Scan | Stage/progress, Play entrance, Skip, On output start remain visible; Skip reveals current content. |
| Display lost/start failed | Stop safely, identify reason, retain mapping, offer Refresh displays and Display settings. |

B toggles blackout when a mapper canvas/list or output owns command focus. Blackout pauses scan and scene actor clocks; native surface FX, Dynamic FX, and videos retain their existing playback behavior. Play entrance deliberately clears blackout; label that consequence. Automatic entrance follows the existing preference; with it off, Start retains blackout. Playback does not create project edits.

**Proposed emergency Escape:** whenever output is live, Escape stops it from any mapper-owned window, including menus/dialogs, without a save prompt. It also cancels a pending gesture/connection preview without committing it. With output already off, Escape only dismisses/cancels the active transient action; it never exits the editor. This strengthens actual 0.8.8/current behavior, where Escape is handled by the focused output canvas. Save prompts cannot trap the stop command.

New, Open, example replacement, Exit, and restart use Save/Discard/Cancel for unsaved work. Save failure keeps the document and operation pending; Cancel preserves work. Updates additionally require the existing explicit stop/restart agreement. A casual context-menu dismissal or Escape never means Discard project.

## 9. Projects and compatibility

Preserve `.pmap` and readable legacy `.hmap`, recent projects, managed Projects collection, original-media retention, atomic saves, and external-change detection. Missing media keeps geometry and references and offers Locate/Load media; it must not erase the reference on save. Collection failures expose their destination and retain originals. Save as clearly identifies the resulting file location.

Actual HEAD reads versions 1/2/3: legacy-only content stays version 1; connections, actors, or IDs 19–22 require version 2; nondefault actor speed/size/playback requires version 3. Global appearance preferences do not upgrade a file. Unknown versions/animation IDs or invalid topology must fail before replacing the open scene, with an unsupported-format message rather than a blank mapping.

Before overwriting an older document version, preserve its **exact bytes** next to it. For `Porch.pmap`, current names are `Porch.pmap.v1-recovery.pmap` or `.v2-recovery.pmap`; a different existing recovery gets the hash-qualified `Porch.pmap.v1-<SHA256>.pmap` form. Never overwrite a different recovery. The rolling `Porch.pmap.bak` is the previous save, not the permanent conversion recovery. If either required protection fails, leave the original unchanged and keep open work available for Save as.

Downgrade cannot serialize a version 2/3 project into version 1 without losing features. Retain the newer project and use an actual pre-conversion version-1 recovery separately; a newly created modern project may have no such copy. Current portable rollback blocks while the open document needs a newer format. Explain that New/close the document first, then choose the retained runtime; never silently pass incompatible content to it. Runtime rollback does not revert saved data, and Store-managed updates have different runtime authority. Verify any eventual old-runtime recovery using copies, never Shane's only mapping.

## 10. Reachability and acceptance

| Capability | Familiar 0.8.8 access | Proposed restored access |
|---|---|---|
| Project/history | Labelled toolbar, File/Edit | Same; named Undo/Redo when available |
| Surface CRUD/order/flags | List and labelled buttons | Same plus complete named context menu |
| Media/video/appearance | Left expandable sections | Same; explicit shared-video status/errors |
| Corners/mesh/zoom | Mapping and direct canvas | Same; focus-safe shortcuts/cancel |
| Core FX IDs 1–13 | Potato FX picker | Same picker plus temporary browser |
| Maple Supper, Harvest Lanterns, Pumpkin Mischief, Ghost Parade, Rainlit Window | IDs 14–18 and examples | Same, in event categories and examples |
| Hollow Crown, Lantern Keepers, Midnight Workshop, Snowfall | Added after 0.8.8 | IDs 19–22 retained in picker/browser |
| Connections/ties/skeleton/wizards | Added after 0.8.8 | Left Connections/Whole scene actors sections |
| Hot-Ass Potato/Potato Focus | Dynamic FX | Labelled top action and left scene section |
| Output/Scan/recovery | Output band, File/Help | Same visible band; protected modern formats |
| Themes/events/reduced motion | Added after 0.8.8 | Settings with independent content/theme scopes |

At narrow supported widths, wrap labelled command groups and scroll inspector content vertically; preserve the list and output emergency controls. Do not convert essential labels to icons or recreate four navigation tabs. At high DPI, keyboard focus remains visible and text does not overlap controls. Screen-reader names identify action, target, checked/disabled state, and scope. Empty project, no selection, no search results, missing media, invalid project, absent projector, and graphics failure each receive distinct actionable text.

Future acceptance must record the candidate version, actions, actual observations, and failures. These are **unrun acceptance scenarios**, not passing checks:

1. From a blank mapping, add/rename a surface, load video, align corners, deform a mesh point, save/reopen, and start output without hunting for a panel.
2. Right-click an unselected overlapping panel, verify its named target, delete it, then Undo/Redo. Repeat from a hidden list row and a locked surface. Confirm original and collected media bytes remain present.
3. Edit a spinbox/search/name with Delete, Backspace, B, F, arrows, and Ctrl+Z; verify no surface disappears or projection command fires, including with empty text/history. List arrows navigate; canvas arrows nudge. Exercise surface actions by keyboard alone.
4. Fit one edge to part of a longer edge; inspect unchanged target/other nodes, cancel once, apply once, walk the skeleton across a continuous seam, disconnect one link, and Undo. Preview/apply Reset mesh after Fit; visible corner positions stay fixed and Undo restores the prior mesh. Reject a folded/coarse fit without mutation.
5. Assign Halloween content, switch to Christmas editor appearance, disable Halloween browsing, and reopen. Content/settings remain; theme does not recolour output. Edit actor speed/size/pause separately from video and Dynamic FX.
6. During Scan, a context menu, and an unsaved-work dialog, stop live output with Escape. Test blackout/Skip and display disconnection. Inspect clean output without editor overlays.
7. Convert version 1→2→3 on synthetic copies; compare recovery bytes, reopen both formats, exercise an occupied recovery name and failed backup, reject an unsupported file, and rehearse retained-runtime rollback separately.
8. Repeat the principal mapping task at the minimum supported window and 150%/200% scaling; confirm all labelled commands remain reachable. Then seek Shane's workflow/visual acceptance and separate physical-projector observations.

No critic score, document review, compile result, or local framebuffer alone establishes these user tasks or physical projection quality. After approval, implementation planning must map each contract to a reachable control and proportionate evidence; unfinished behavior stays explicitly unfinished.
