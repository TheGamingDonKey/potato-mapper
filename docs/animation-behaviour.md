# Native seasonal collection

Implemented source, unreleased. IDs 0–18 and the Pumpkin Mischief / Ghost Parade / Rainlit Window choreography are retained. This increment adds four original, self-contained GLSL 3.30 scenes; it needs no downloaded art, media decoder, new framework or generated asset cache. It is a bounded native interpretation of the woodland direction, not completion of the five cinematic proposals in the Hollow Wood research.

The existing Speed, Pause, Reverse, angle, palette, brightness, opacity, blend and soft-edge controls apply. Each new scene repeats after **24 seconds at Speed 100%**. Reverse runs the same choreography backward; it does not simulate physical snow moving forward in reversed time. All event envelopes settle before the seam. Snow and offscreen travel wrap only by whole periodic lattice/travel spans. Existing IDs 14–18 retain their twelve-second periods and previous defaults.

| ID / scene | Scene size | Density label / default | Flow label / default |
|---|---|---|---|
| 19 Hollow Crown · woodland revenant | 24 | Forest lights / 28 | Awakening / 65% |
| 20 Lantern Keepers · autumn journey | 24 | Travelers / leaves / 36 | Expression / 60% |
| 21 Midnight Workshop · winter visitors | 24 | Snowfall / 28 | Workshop bustle / 55% |
| 22 Snowfall · shared winter sky | Snowflake size 24 | Snowfall / 36 | Wind / 40% |

Use Original colour first. Other palette choices preserve luminance shading instead of painting the illustration a solid colour. Snowfall returns transparent sky between the flakes, so it can be layered over other surfaces with Normal, Screen or Add.

## Directed action

**Hollow Crown.** Broad receding trunks and one cool light field frame the central tree. From 3–9 seconds, connected root/hip/chest/head landmarks take weight, the crown spreads asymmetrically and the narrowing tree head becomes a broad pale wooden skull. Recognition holds before attention turns from 11–15 seconds. The eyes close once at 16.7–17.03 seconds. From 19 seconds the body settles toward its tree pose while a broad foreground bough travels over the head and shoulders. The bough settles by 23.9 seconds. Root, neck and antler attachments remain continuous; the effect does not crossfade between two independent character images. Awakening adjusts weight/head displacement while retaining the reveal at zero. Forest lights changes two to seven small fixed-grove lights; their gentle paths and pulses share the loop clock.

This is a restrained 2D revenant illustration. It has no simulated forest depth, volumetric shadows or anatomical 3D head rotation. The moving bough adds occlusion but should not be described as proving that every exposed silhouette reset is hidden. Judge the unadorned weight transfer and connected head/crown motion in output before adding detail.

**Lantern Keepers.** Two to four unequal hooded woodland travelers walk through a warm clearing. Each has an individual local 24-second journey, offset by 5.88 seconds. A traveler approaches, eases to a stop around local seconds 8.6–13.2, lifts its lantern and looks toward it, then resumes walking. Feet alternate through a low swing arc; the torso rises twice per stride. The lantern trails the hand and continues a smaller settling swing during the hold. The smallest travelers answer with a rounded surprised mouth; the leader keeps a quiet smile. Individual blinks occur around local seconds 4–5.35. Broad side trunks and travel beyond the panel cover the return. Density adds travelers at 32 and 64 and progressively adds five broad maple leaves; Expression adjusts the lantern lift, lean and trailing motion. Walking persists at zero.

The path is an authored clearing journey. It does not use the surface-boundary actor/navigation system and does not claim to walk across project topology. A panel crops this composition as it crops any other per-surface scene.

**Midnight Workshop.** An open-front timber workshop contains a small red-capped maker, a golden spinning toy and a robin visitor. The maker first observes, raises an articulated mallet during seconds 3.5–6.15, then makes a quick contact at 6.5 seconds. The toy starts afterward and turns exactly three complete revolutions by 11.9 seconds, returning to its initial orientation. The robin startles after contact, leans back toward the toy and blinks independently. The maker has its own blinks before and after the main action. A table provides a visible contact plane. Evergreen swag, berries, broad snow on the roof and restrained amber light establish Christmas. Workshop bustle changes the anticipation and visitor's lean; the main action remains at zero. Snowfall controls exterior snow spacing. Scene size scales the workshop; it also scales the snowflakes modestly.

The maker's mallet contact and toy response are authored timing, not a rigid-body simulation. Review the contact in real moving output; a readable still does not prove it works.

**Snowfall.** Two layers use the same normalized projector coordinates across every surface. Larger bright flakes and smaller blue-white distant flakes descend through one coherent gust field. Each layer translates exactly 13 or 17 lattice cells in 24 seconds. Seeds depend only on the periodic world cell and depth layer, so splitting a panel does not reseed the sky. Wind adds broad horizontal gusts and a smaller second harmonic; zero Wind retains vertical fall. Density changes lattice spacing, Size changes flake radius. Shared snow matches when phase, angle, speed, density, size and flow match across the participating panels. Deliberately different controls produce different fields. Snow rendered behind the workshop is covered by its solid cabin and cast.

## Renderer contract and checks

The fragment shader additionally consumes `in vec2 mappedOutputUv` and `uniform float outputAspect`. `mappedOutputUv` must describe the actual mapped output location, including deformed mesh positions, independent of editor zoom/pan/viewport. `outputAspect` is output width divided by height. Editor and projector must bind the same output-space values. Snow uses these values; the other compositions retain per-surface coordinates.

`runSeasonalChecks` in `tests/seasonal-check.h` creates only a disposable mapping. The root CLI can dispatch it as `--seasonal-check`, with optional `--snapshot <png>`, `--frames <folder>` and `--demo <pmap>`. Frames captures 96 samples at quarter-second intervals over a complete loop. The fixture checks shader availability and GL errors, stable IDs/defaults, independent movement, loop seams, Size/Density/Flow effects, editor/output agreement, save/reopen and matching snow before/after splitting one field across two adjacent panels.

At authoring handoff, these checks have been written but not executed by the animation specialist. Root integration owns compiling and running the single controlled build. Read the actual root verification output before claiming checks passed. Captured playback review, extracted distribution testing and physical projection are separate evidence. This document does not establish an artistic acceptance score or user acceptance.
