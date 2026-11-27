# Halloween research brief and twenty candidates

Date: 15 October 2026. Status: research and original art-direction proposals, not an implemented update.

## User intent and scope

Shane rejected the visual quality of 0.8.8 and explicitly requested deep research, twenty distinct ideas, a shortlist of five or intelligently merged ideas, over a dozen specialist agents, independent scrutiny of their research, and a demanding critic. He then selected **dark enchanted forest and spectral creatures** as the dominant direction. Deliver original, dramatic, spooky, mesmerizing animation concepts suitable for one home projector. A high target score does not authorize inflated judgments.

## Baseline inspected by root

Actual checkout: `C:\Users\mrbos\Downloads\Potato Mapper`; clean main at `38f430b` at the start. Read AGENTS, README, DEVELOPMENT, ROADMAP, model.cpp and model.h; inspected native `out/halloween-check/shape-review.png`. Current characters are small friendly icons in mostly empty panels; rain is a flat, fairly uniform field. Existing functional evidence does not establish compelling art direction. Prior package, persistence and update checks are not being rerun as substitutes for visual work.

App supports meshes, looping media, Screen/Add, native GL3.3 effects. Video audio starts muted (model.cpp:36), but the choreography reviewer verified the existing Mute audio checkbox can enable it (main.cpp:154–155,387). Silent-first is an art requirement, not a missing-audio claim. The offline specialist also verified that Scene::source caches MediaSource by exact media path (model.cpp:260–264), so surfaces using the same file share its player. Distinct media files have independently started players; no shared cross-file transport was established. Thus arbitrary multi-file synchronized choreography or audio-reactive visuals are not existing guaranteed capabilities. Use a single composed video (optionally reused across surfaces) for initial proof. Dated Radeon 840M measurements are not new performance tests.

## Working art rules to challenge

Environment first; creature belongs to the forest. One clear visual event per act. Readable large silhouettes, near/middle/far depth with occlusion, a restrained light hierarchy, distinct motion identity per scene. Black pixels cannot remove existing room light. Avoid generic neon tunnels, randomly moving sprites, constant maximum glow and cute mascot expressions. Loops should resolve through meaningful occlusion or transformation, not a visible cut or perfunctory global fade. Silent readability is required; audio is an optional later enhancement.

## Twenty candidate hypotheses

These are original design hypotheses; references inform techniques and do not prove the proposed result looks good. Do not score as rendered work.

| ID | Concept | Defining transformation / motion | Main risk | Initial disposition to challenge |
|---|---|---|---|---|
| 01 | The Hollow Crown | Crooked branches align into an enormous antlered forest spirit; a slow head turn reveals that the grove is its body. Antlers unravel into roots. | Deer becomes a cute glowing mascot or obvious line drawing. | Hero candidate |
| 02 | The Lantern Procession | A chain of low amber lights winds behind trunks; occasional light reveals tall faceless travelers. The final lantern moves with no carrier. | Parade of repeated floating sprites. | Hero candidate |
| 03 | Murder of the Moon | A visible flock follows one coherent arc; crows veil the moon, form one vast wing, then break into dark leaves. | Generic particle vortex; small birds unreadable. | Hero candidate |
| 04 | The Mycelium Oracle | An underground capillary network carries a wave into an ancient bark face; its eye opens, the wave retreats to a seed. | Neon sci-fi circuitry or accidental smiley face. | Hero candidate |
| 05 | The Forest Behind the Wall | Root seams split the mapped surface into apparent depth; a forest watches through the opening, then bark knits the surface shut. | Assumes arbitrary house geometry; generic portal. | Hero candidate |
| 06 | Cathedral of Roots | Roots bend into a ribbed nave; a moving band of moonlight reveals impossible scale before the nave becomes trees. | Too close to 01/05; camera movement weakens mapping illusion. | Merge structural idea into 01 |
| 07 | The Wrong Shadow | Branch shadows lag the wind, then one stands upright while its tree remains still. | Too subtle on a bright wall. | Merge into 02 as one reveal |
| 08 | The Pond Below | The surface appears to become black water; reflected trees ripple before a submerged creature passes beneath. | Projection cannot make wall physically black; viewpoint dependence. | Alternate / depth test |
| 09 | The Moth Moon | A giant lunar moth opens with tree-ring wing markings; dust motes drift inward as wings close. | Decorative fantasy wallpaper; overlaps 03 wing climax. | Alternate to 03, not simultaneous |
| 10 | The Wicker Heart | A hanging woven cage slowly beats; each beat lights a different bone-like rib of the surrounding grove. | Repetitive scale pulse masquerading as choreography. | Reserve |
| 11 | The Witch in the Bark | A weathered tree cross-section folds into an elderly presence; wood grain keeps moving against the face. | Portrait detail collapses at distance; uncanny caricature. | Reserve / 04 material study |
| 12 | The Ash Hound | An ember trail becomes a long hunting creature seen between trunks; its body cools into falling ash. | Requires convincing creature gait and complex simulation. | Reserve, high production cost |
| 13 | Autumn Runs Backward | Fallen leaves return to bare branches against the wind; the rebuilt canopy briefly outlines something enormous. | Ordinary fall screensaver before reveal. | Quiet interlude / 05 recovery idea |
| 14 | The Bell Under the Roots | Buried circular rings pull nearby roots inward; silent visible resonances reveal a distant hanging figure. | Depends on sound for meaning; overlaps oracle pulse. | Merge timing cue into 02 only if legible silently |
| 15 | Thorn Bloom | A thorn crown unfolds into a luminous poisonous flower; its petals are layered spectral silhouettes. | Pretty abstract flower overwhelms Halloween identity. | Reserve, 04 texture alternative |
| 16 | The Antler Mirror | A deer reflection turns before its owner; both dissolve into the same branching water pattern. | Two focal creatures, demanding matched perspectives. | Retain head-turn wrongness for 01, not whole scene |
| 17 | The Weaver Above | Silk appears along real panel edges; one huge spider moves behind the canopy while dew marks its path. | Cliché / phobia dependence; too many tiny strands. | Reserve, strong rival to 03 |
| 18 | The Shed That Walks | One large hollow bark plate unfolds on three stiff contacts, takes a sideways step, and collapses behind a trunk. No antlers, eyes or cloak. | Rubber-like folding unless grain and contact points are respected. | Independent rival to01; replaces weak Black Rain after specialist09 challenge |
| 19 | The Returning Roost | Three crows land; their supports vanish beneath them. On departure, broad leaf-like pieces restore the missing branch silhouettes. | Landing weight and missing supports must read clearly. | Independent rival to03; replaces Drowned Portrait after specialist09 challenge |
| 20 | Last Harvest Fire | A low copper fire reveals leafless silhouettes; sparks slowly become distant departing spirits. | Too gentle and familiar to lead the requested collection. | Retain as optional autumn coda, not first five |

## Evaluation proposal

Score dimensions 0–10: forest/spectral fit 20%, distinctive visual idea 20%, motion/choreography 20%, projected readability 15%, production feasibility 15%, evidence quality 10%. These are concept-selection estimates, not audience test results. A 9.5/10 finished-animation target needs independently reviewed actual loops, aspect and mesh evidence, real playback measurements, and eventual projector observation. No paper concept earns that claim. Reviewers should expose score uncertainty, duplicated candidates, over-combinations and sources that do not support their claims.

## Proposed first moving proof (not yet built)

One 24-second Hollow Crown loop at a single 16:9 surface, fixed camera, three depth layers, one large creature, one decisive slow turn, a branch-occlusion reset. Compare original-size and quarter-size output, then a deliberately lifted-black simulation. All three are proxies until physical projection. Rendered-video and restrained-native options must be compared against actual complexity and current app controls before choosing production.
