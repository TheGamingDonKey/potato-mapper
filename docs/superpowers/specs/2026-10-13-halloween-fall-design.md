# Halloween and fall collection — 0.8.8

Shane asked on October 13 for a creative Halloween/fall Mapper update, then explicitly requested multi-agent brainstorming and a deep plan followed by execution. The dinner is finished; this task is the existing native projection app. Three read-only agents designed Halloween art, autumn art, and integration/verification before implementation.

## Intended result

Ship three distinctive, moving native scenes with individual examples, an October Porch composition and a shareable preview. The scenes should be readable on real mapped panels, feel alive through small character actions and weather, and work through the existing FX controls. A tested public Windows update is the delivery; physical-projector appearance remains a separate user check.

## Selected scenes

| Stable ID | Name | Composition and twelve-second choreography | Defaults |
| --- | --- | --- | --- |
| 16 | Pumpkin Mischief | Three ribbed jack-o'-lanterns, carved amber faces and violet ground shadows. A small pumpkin gently hiccups embers, the large one looks toward it and winks, then all settle. Opening faces stay illuminated. | Size 24, Density 24, Flow 60, Original colour |
| 17 | Ghost Parade | Rounded pale ghosts with scalloped cloth and dark expressive eyes follow a lantern guide on a closed oval inside each panel. The shy follower lags, stretches while catching up and relaxes; the guide waves. | Size 24, Density 32, Flow 60, Original colour |
| 18 | Rainlit Window | Navy glass, two layers of visible blue-grey rain, a soft amber reflection, subtle bronze mullion/sill and two copper leaves. Rain falls, a slow gust bends its direction and stretches the reflection, then settles. | Size 24, Density 24, Flow 50, Original colour |

Pumpkin Density changes 2–6 embers while retaining three characters. Ghost Density changes 2–4 ghosts (32 gives three). Rain Density changes spacing. Size scales the artwork and details together; the ghost route stays inside the panel. Flow changes expressive motion, cloth/gust/drift and reflection distortion; Flow 0 retains basic breathing, route travel or rain. Speed and Pause retain their existing meanings. Reverse plays the same stateless choreography backward and does not change a paused frame.

Use dark backgrounds with selective readable highlights. Shapes use aspect-correct coordinates. Event envelopes fade to zero away from the cycle seam; motion uses integer harmonics of one twelve-second phase. Rain tiling must repeat its seed lattice and translate an integer spatial period in twelve seconds. No per-frame reseeding, unbounded particles, ray marching, external art or rapid full-panel flashing.

## Alternatives considered

Rainlit Window was selected over Copper Canopy and Last Ember because glass, rain and reflection add a distinct autumn mood beyond the shipped maple leaves and lanterns. Pumpkin Mischief and Ghost Parade were selected over House Is Watching because their grounded character comedy and airborne procession complement the window scene. Cross-panel ghost routing, cameras, Raspberry Pi and a scheduler are outside this increment.

## Integration contract

- Keep C++17, Qt 6.10.3 and OpenGL 3.3; no dependency or project-format change.
- Preserve IDs 0–15, their defaults, palettes, shader bodies and timing. Append IDs 16–18.
- Centralize the CPU seasonal period: IDs 14–18 use 12 seconds; older IDs retain 10000 seconds.
- New IDs preserve shading under chosen palettes. Original colours are their defaults.
- Add default Flow helper for the new IDs; use it for missing JSON values and selecting a new preset. Retain selecting/loading behaviour for IDs 0–15.
- Keep shader work fixed: three pumpkins/six embers, four ghosts/one lantern, or two rain layers/four candidates per layer plus two leaves. Derivative antialiasing must stay outside spatially varying early exits.
- Include self-contained Pumpkin Mischief, Ghost Parade, Rainlit Window and October Porch `.pmap` examples inside the versioned runtime. Examples open Potato FX controls and leave Dynamic FX off.
- Changing settings keeps existing Undo/Redo/dirty behaviour; animation ticks stay clean.
- Preserve installation/project/media locations and the existing updater protocol. Older apps can replace unknown new IDs when saving: document that boundary.

## Acceptance and release gates

1. Native picker tests fail for missing IDs before production implementation.
2. Actual renderer: new IDs/defaults, visible per-panel motion, complete 96-frame twelve-second capture, phase `t`/`t+12` including negatives, ±10000/±4 equivalence and small changes across ±12 seams.
3. Controls: paused Reverse preserves the frame, resumed Reverse decreases phase; Speed 0 and Pause freeze; ticks preserve settings/history; user setting changes support Undo/Redo and save/reopen. Size, Density and Flow visibly affect their intended content. Representative extremes cover size 5/45, density 4/80, Flow 0/100, angle -180/180, palettes 0–5, edge 0/25 and appearance 0/50/100 without a Cartesian test bank.
4. Editor/output interior samples agree at paused phase. Outside mappings and gaps remain black. Normal/Screen/Add are numerically checked over a known underlying colour by the existing FX sweep. Artwork is inspected on portrait, landscape and deformed mappings.
5. Fix the evidenced profile-density diagnostic mismatch: apply requested density to ordinary FX and report actual model density. Measure new effects at default and maximum density, 1080p output with preview, one/four layers. Record GPU pass costs and repaint/heartbeat limits; optimize only an evidenced bottleneck.
6. Run full FX, harvest, Dynamic FX, entrance and mapping checks, plus geometry/storage checks. No success claims from source inspection or a critic score.
7. Package 0.8.8 once final code passes; extract fresh with only Windows system directories on PATH, check all examples and actual root-launcher child. Rehearse real 0.8.7→0.8.8 install/reopen, interruption, cancellation and incomplete-payload cases using synthetic work; retain original bytes and prior runtime/launcher.
8. Review final diff, push/merge, publish checked ZIP and GIF, verify public latest metadata and canonical downloaded bytes/hash. Finish the handoff on clean main.

## Execution authority

Shane's latest instruction authorizes the sequence: agents brainstorm and plan deeply, then execute. The prior request includes publishing the update when ready. The written design and plan are reviewable artifacts; routine implementation and the verified release do not require repeating that authorization.
