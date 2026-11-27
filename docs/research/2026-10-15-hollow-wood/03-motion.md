# 03 — Mesmerizing woodland motion and spectral ecology

Research date: 2026-10-15. Research and original concept proposals only; no implementation, downloaded artwork, publication, new benchmark, or physical-projector test.

## Recommendation

Build the motion around **one large supernatural event within a coherent forest**, rather than independent decorative particles. The strongest candidate is a dead blackthorn tree that gradually resolves into a hollow spectral stag, takes one impossible step, and becomes the same tree again. A funeral procession of spectral hounds is the next strongest. A giant woodland widow offers the clearest immediate Halloween reading but requires careful restraint to avoid a conventional spider overlay.

Use an **offline-authored 24-second hero loop played as existing mapped video** for the first visible proof. A later hybrid can add inexpensive native movement when it has a specific purpose. Do not assume a new volumetric realtime renderer is required or affordable.

The current native foundation was verified in local files: CMakeLists.txt specifies C++17 and Qt6; assets/patterns.frag declares GLSL 330; README requires OpenGL 3.3 Core and documents mapped looping video. DEVELOPMENT.md records October 2026 Radeon 840M measurements with 1920×1080 output and 960×540 preview: roughly 20–26 output repaints/s in the default Halloween one/four-layer cases, with four overlapping new effects costing roughly 7–10 ms per output GPU pass. These are prior short desktop samples, **not a fresh measurement, guaranteed playback rate, total GPU time, or projector evidence**. They argue for profiling before adding expensive passes, not for declaring the GPU incapable.

## Six primary sources and evidence audit

The links below are original artist, author, publisher, or official tool documentation. No SEO inspiration lists are used. None of the linked films was watched in motion during this specialist pass. Thus no claim below treats a film's actual timing, loop smoothness, projector clarity, or emotional effect as directly observed.

| ID | Primary source and direct URL | Actual evidence retrieved | Supported finding and boundary |
| --- | --- | --- | --- |
| S1 | Robert Bridson, Jim Hourihan, Marcus Nordenstam, **Curl-Noise for Procedural Fluid Flow**, SIGGRAPH 2007, [author-hosted paper](https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf) | Full three-page PDF text, equations and figure captions. Screenshot request returned a reference but no inspectable pixels in this tool response; do not count as visual inspection. | Curl of a potential produces a divergence-free field; the paper handles spatial modulation and solid boundaries. Useful for fog that bends around trunks rather than unrelated drifting dots. This does not prove this app can afford a particular implementation. |
| S2 | Fabrice Neyret, **Advected Textures**, SCA 2003, [author-hosted paper](https://www-evasion.imag.fr/Publications/2003/Ney03/neyret161.pdf); [Eurographics publisher record](https://diglib.eg.org/items/c4bf80cc-23f0-4107-958d-e03c0b9c8ac4) | Search extraction of the primary paper abstract and author-hosted colour-plate captions. Direct PDF retrieval timed out; direct publisher open returned 403. **Limited audit, abstract-level support only.** | The abstract identifies the tension between temporal/spatial continuity and preserving texture statistics, using regenerated advected parameterizations. Supports avoiding indefinitely stretched smoke textures. No implementation detail beyond that is treated as verified. |
| S3 | Memo Akten, **Simple Harmonic Motion**, [artist's project/process text](https://memo.tv/projects/2019/shm/) | Full artist text including the explanation of custom-software agents, frequencies, and changing alignments. Listed installation-image and embedded-video links were not playback evidence. | Simple repeated actions can collectively produce complex changing order. Transfer: choose a few related rhythms that gather into alignment and then separate. This is artistic-method evidence, not a controlled study proving hypnosis. |
| S4 | Universal Everything, **Transfiguration (2024)**, [artist's project page](https://www.universaleverything.com/art/transfiguration-2024) | Full original project text and specifications: 4K video, 7m55s. Artist describes steady walking through material transformations. A linked still URL resolved, but pixels were not available for inspection here; film not watched. | Preserve an invariant action while material changes, so viewers can follow one entity through transformation. Transfer the principle; do not copy its figure, material sequence, soundtrack, or imagery. |
| S5 | Patricio Gonzalez Vivo and Jen Lowe, **The Book of Shaders: Fractal Brownian Motion**, [author tutorial](https://thebookofshaders.com/13/) | Full tutorial text and displayed code excerpts covering amplitude/frequency, octaves, ridges and domain warping. Interactive shader results not operated or timed. | Detail layers add spatial frequency; stop when details become unresolved. Domain warping can produce related organic structure. It is a rendering tool, not a substitute for an authored woodland subject. |
| S6 | SideFX, **Labs Make Loop 2.0**, [official node documentation](https://www.sidefx.com/docs/houdini/nodes/sop/labs--make_loop-2.0.html) | Full relevant node text. It documents half-timeline offset with sine blending, constant-topology/volume support, and particle age/life requirements. No Houdini node run. | A documented production route for looping animated effects. The text does not establish that a crossfade is invisible for sharp creature silhouettes, nor that Houdini is necessary for this project. |

**Rejected/uncounted:** An attempted Inigo Quilez domain-warping page was inaccessible; it is not a seventh verified source. Initial malformed Universal Everything domain searches were corrected to the artist's actual domain. Secondary summaries were used only to locate primary records, not as final support.

## Transferable motion rules

These are my design inferences from the sources and the projection brief, not scientific claims about trance or audience perception.

1. **Give the whole scene one flow direction.** Fog, hanging growth, fur-like wisps and leaf edges respond to the same broad passage through the grove. Add localized curls near trunks. Coherent forces make the forest feel inhabited; independent random trajectories make a screensaver. S1 informs the field approach.
2. **Preserve one thing during transformation.** Keep the stag's central spine, the hounds' gait, or the widow's anchoring points legible while matter changes. The supernatural event should be trackable. S4 informs this continuity rule.
3. **Use three time scales with jobs.** For a proposed 24s composition: slow scene pressure over 24s, creature respiration over 8s, a limited secondary ripple over 4s. These are starting values, not measured optimal frequencies. S3 informs layered order; do not make everything pulse together continuously.
4. **Anticipation must change the forest before the creature moves.** Branch tips bend inward; the fog pauses; a shadow leans ahead. Then the entity acts and the grove releases. This proposed staging creates a cause to watch for. It is my choreography, not an observed timing claim from the artists' films.
5. **Use recursive change at only a few readable scales.** A branch becomes an antler; an antler's broad fork repeats the trunk's fork. Avoid an endless zoom or dozens of tiny fractal branches. S5 informs frequency restraint; the Halloween reading must survive without microscopic texture.
6. **Let fine texture travel with the large form.** A wisp should follow a shoulder turn and curl after it, not dissolve and regenerate at an unrelated location each frame. Refresh strained texture invisibly. S2 supplies the continuity problem; the proposed treatment remains unimplemented.
7. **Spend darkness deliberately.** Hold broad black gaps between trunks and around the creature. Restrict bright edges to the event currently unfolding. Do not cover the whole field with equal fog, glow and particles.
8. **Build a seam into the choreography.** Return physical pose, material, illumination, texture phase and derivatives to the initial state. Merely making the velocity field periodic does **not** make advected particle trajectories close. This distinction is essential for a convincing native loop.

## Candidate A — Blackthorn Resurrection

**Image:** One skeletal tree across a dark forest clearing; its branches become the antlers of a towering, hollow woodland revenant. The body contains moving night and pale root fibres rather than cartoon anatomy. Sparse amber sap is secondary to moon-grey form.

| Time | Choreography |
| --- | --- |
| 0–6s | The tree appears still while a broad fog sheet passes behind it. Roots slowly tighten toward its base; nearby branch tips lean inward. Keep the central trunk readable. |
| 6–12s | An advancing deformation travels upward: trunk splits into long legs, shoulder mass forms from bark, and the crown resolves into antlers. The same silhouette landmarks remain visible. A hollow ribcage opens from negative space. |
| 12–18s | The stag lowers its skull, suspends one hoof, then steps forward without disturbing the ground. Its shadow steps a fraction earlier. Fog recoils and curls around the legs. Hold the full apparition long enough to recognize it. |
| 18–24s | The hoof roots into the clearing; shoulder fibres stretch back into trunk; antlers settle into their original branches. The fog broadens behind it as all geometry reaches its initial pose with a gentle stop. |

**Seam:** Pose/material curves have identical values and zero endpoint velocities at 0/24s. The background fog is independently cyclic, with an authored seam behind the trunk. Use cyclic deformations or baked topology correspondence; a dissolve between unrelated stag/tree images would look like two overlaid subjects.

**Why potentially hypnotic:** One reversible identity, slow upward flow, and the delayed recognition of a figure that was present all along. **Halloween identity:** dead forest, spectral anatomy, skull and hollow ribs, shadow acting ahead of its owner. A healthy glowing deer alone would read as fantasy wildlife; the revenant treatment is necessary.

**Route:** Offline hero video first. Simple native branch deformation could later be feasible, but convincing tree-to-creature anatomy is a poor first realtime shader target. **Risk:** antler detail disappearing at distance or a rubbery morph with no mass. Keep only a few large antler forks; let torso/hoof motion lead trailing fibres.

## Candidate B — The Hounds Between the Trees

**Image:** Two enormous, translucent woodland hounds travel slowly behind black trunks. Their bodies are braided fog with rib-like root arcs. Each remains a creature, rather than a loose collection of ghost symbols.

| Time | Choreography |
| --- | --- |
| 0–6s | Low ground fog moves left to right. A long shadow passes between trunks before its owner becomes visible. The first hound's shoulder condenses from the same moving fog ribbon. |
| 6–12s | The first hound crosses in a slow, complete stride. Its rib arcs follow the body's rhythm; vapour trails lag the head turn. The second begins to form farther back at a deliberately offset gait phase. |
| 12–18s | Both creatures slow as the nearer hound turns toward the viewer. Their gait rhythms briefly align; surrounding fog gathers toward them. One calm, sustained look replaces a jump scare. |
| 18–24s | The first exits behind a broad trunk and dissolves into the ground ribbon; the second follows at depth. The foreground shadow completes its passage and the fog returns to the opening configuration. |

**Seam:** Use a looping gait attached to a cyclic world path, hidden returns behind occluding trunks and outside the visible clearing. Give each body's density a cyclic envelope that is fully zero during its hidden return. Bake the shared fog as one composite so an opaque reset cannot leak through the skeletal body. Do not rely on a generic dissolve over an exposed silhouette.

**Why potentially hypnotic:** Repeated strides slide into a short moment of synchrony, then separate; the eye can track bodies and their trails. **Halloween identity:** phantom hounds with impossible preceding shadows, hollow bodies and a silent forest vigil. Keep heads angular and severe, eyes minimal, proportions long; puppy faces would reproduce the rejected cute direction.

**Route:** Offline creature/fog composite plus static foreground forest imagery. Later hybrid flow wisps are optional and must share the authored motion rather than obscure it. **Risk:** reading as a normal animal walk cycle with opacity lowered. Put the supernatural behaviour in the shadow timing and body construction, not only transparency.

## Candidate C — The Widow Grove

**Image:** A giant spectral spider seems to be made from the tree architecture itself. Its widely separated legs coincide with trunks; a few massive web arcs bridge the clearing. Bone-white web and desaturated violet body, with dark gaps retained.

| Time | Choreography |
| --- | --- |
| 0–6s | A sparse web canopy tightens from one anchoring point to another in a travelling wave. Trunks subtly lean with the tension; the centre remains almost black. |
| 6–12s | The central shadow unfurls into a spectral widow abdomen. Two leading legs peel away from apparent tree trunks, followed by staggered trailing legs. Preserve the original anchor points to make the reveal intelligible. |
| 12–18s | The widow advances one measured step, pauses, then pulls a single web strand. A broad pulse runs through the whole canopy, sequentially answering at the surrounding anchors. |
| 18–24s | The body folds back into canopy shadow while the legs nest into trunk alignments. The returning tension wave settles to the opening web configuration, leaving the clearing empty again. |

**Seam:** Use a cyclic anchor/tension rig with a closed sequence of web deflections. All anchor positions and velocities match at 0/24s. Fog returns independently behind the dark body zone. If using simulated cloth/web, author a looping cache and inspect the blend for doubled strands; S6 is a possible tool route, not proof of a clean result.

**Why potentially hypnotic:** One tension impulse propagates through an understandable network, making every movement appear causally related. **Halloween identity:** unmistakable giant spider, webbed dead grove, creature concealed in architecture. **Route:** Offline deformation/light render; cheap native curve motion is a later option. **Risk:** hair-thin webs and too many legs moving simultaneously becoming visual noise. Use several thick web arcs and one initiating leg pair, with the rest following.

## Route choice and stopping point

| Route | Best use | Main uncertainty |
| --- | --- | --- |
| Native GL3.3 shader | Broad 2D fog, subtle local warp, a few bands/curves, parameters that must stay adjustable | Cost multiplies across preview/output and overlapping surfaces. A paper's realtime demonstration does not establish this app's performance. No volumetric raymarching assumed. |
| Offline video | Creature anatomy, material transitions, shadows, precise occlusion and one authored 24s event | Decoder seek/restart may expose a pause despite identical endpoint images. Test actual packaged playback. Moving assets also become saved project data that must be preserved. |
| Hybrid | Authored hero loop plus purposeful, low-cost surroundings | Separate videos/shaders may not share exact phase. Current layer playback must not be assumed synchronized without evidence. Start with one composite when exact choreography matters. |

Recommended first proof after concept approval: one 24s composite of A, one mapped surface, black exterior, minimal duplicate layers. A proposed delivery target is 1080p at 30fps (720 frames); performance and codec choice need a real playback check before being promised. Encode frames 0 through 719, not a duplicated 720th endpoint that creates an extra held beat. Inspect at least three consecutive native playback loops, including the decoder boundary.

Approval-ready comparison should show a simple key pose, an actual motion preview, and seam behaviour. Check the same scene on a square, wide and tall mesh; at a small preview/thumbnail scale to expose illegible detail; and with representative overlapping output/preview load. Physical-projector testing remains separate. The pass condition is that the entity, supernatural action and loop survive those checks without relying on fine glow or hundreds of particles.

No source artwork or footage is proposed for redistribution. All three candidates are original scene proposals using transferable methods.
