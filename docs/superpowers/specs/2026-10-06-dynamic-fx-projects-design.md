# Dynamic FX and managed portable projects

Shane approved building this design on October 6, 2026 and authorized the launcher to manage the Potato Mapper installation. Camera work remains deferred. The next release is 0.8.1; subsequent small increments stay in 0.8.x until larger milestones justify 0.9 or 1.0.

## Portable installation and projects

Keep protocol 1 compatible with existing launchers: root PotatoMapper.exe and versions/<version> runtimes, installation.txt, current.txt and previous.txt. The main launcher selects the active runtime; versioned files retain rollback. Add Projects as the obvious default save location and expose it through File. Organize recognized legacy mappings found directly in the install root or its runtime folders by copying them into Projects. Do not scan unrelated computer folders or recursively collect development fixtures. Opening an external mapping can collect it and its referenced media into Projects; originals remain intact. Disambiguate names and retain a migration index so repeated launches do not keep importing the same file. Validate collected projects and retain actionable errors. A failed load leaves the current scene untouched.

Refresh the root launcher from the installed release using a small native helper after the old launcher releases its executable. Bundle the replacement and helper inside the versioned payload, so older protocol-1 launchers deliver the upgrade. Failures retain the old launcher and report the problem. Do not overwrite unknown files or delete old projects/media. The launcher owns program selection and housekeeping, not arbitrary personal data. Deleting the launcher requires a replacement download; an absent executable cannot run itself.

The reported File > Open failure has not been reproduced: no failing project/error was supplied. Improve load errors with the selected path and failing surface, and inspect the existing local mapping. Do not call that particular user failure fixed without evidence. Default saves use Projects; explicit Save As remains available. Recent projects and opening a bare .pmap/.hmap path through the launcher use the same load/collection flow. Older mappings continue loading. Atomic saves retain a previous-save backup.

## Dynamic FX

Add a scene-level Dynamic FX group without changing stable per-surface pattern IDs 0-13. Users choose participating visible surfaces; one group clock coordinates them. Hidden or removed surfaces are excluded safely. Playback does not dirty the project or add Undo; settings/group changes do. Store effect, members, speed, density, palette, glitch strength, seed and playback settings in project JSON. Phase restarts on reopening, as with existing FX. Old apps ignore Dynamic FX; saving with an old app cannot preserve this new feature.

Hot-Ass Potato is a polygon snake performance: explore the selected mapped surfaces, peel small fragments toward their inner perimeter, let fragments settle and breathe, stage a bounded visual overload, briefly assemble POTATO MAPPER, then run a coordinated ambient field with intermittent snake visits. Controls: effect, participating surfaces, speed, density, colour, glitch intensity, pause, replay intro and skip to ambient. Defaults favor cyan/violet holographic geometry, fine outlines, a tapered articulated body and smooth movement. A single surface works; zero participants produce no effect. The simulated crash is rendering only.

Route using nearest unfinished surface and closest boundary entry/exit candidates, with deterministic tie breaking and a visited set. This is a greedy visual tour, not a globally shortest path. Use normalized local coordinates for shapes, forward through the existing mapped mesh. Inter-surface gaps are hidden in this first release. Geometry changes rebuild the route; preserve the group clock. Membership changes restart the introduction rather than indexing deleted geometry. No maze, obstacle search, A* or external graph library is required.

Potato Focus uses the same group/timing foundation: luminous polygon networks, scanning bands, perimeter highlights and coordinated colour pulses. It is an original procedural effect inspired by the requested holographic mood, with no camera analysis.

Compute bounded geometry once per scene tick, cache it for both editor and projector, and draw batches through OpenGL. Limit fragment counts and body segments. Avoid CPU full-frame images, extra video decoders and unbounded particle growth. Reuse brightness, opacity and blend settings. Editor overlays stay out of projector output; the deliberate reboot title is part of the effect on both canvases.

## Acceptance

- Existing local mapping opens without editing its original; failed loads retain the current scene and identify the failing path/reason.
- Fresh and older portable packages open Projects, preserve project/media originals, deduplicate collection, retain previous saves and reopen collected copies independently of original media.
- An actual older launcher installs the final ZIP and the root launcher is refreshed; interrupted upgrade/replacement retains recoverable program/data files.
- Both effects animate across one and three transformed surfaces, pause identically in both canvases, survive member removal, save/reopen settings and leave the project clean while ticking.
- Inspect a motion preview of every snake stage and Focus; compare simultaneous output/preview cost with the existing grid using a short practical measurement.
- Publish a runnable 0.8.1 ZIP only after relevant checks and focused review. Physical-projector and the specific unprovided failing project remain separate verification limits.
