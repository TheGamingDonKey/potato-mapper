# The Hollow Wood — Halloween research and art direction

15 October 2026. **Research deliverable; replacement animations have not been implemented or published.** Source remains `C:\Users\mrbos\Downloads\Potato Mapper`. Shipped baseline inspected: clean `main`, `38f430b`, version0.8.8. Shane rejected its visual quality and selected **dark enchanted forest and spectral creatures** as the new direction.

## Decision

Advance five **design hypotheses** for comparison:

1. **The Hollow Crown** — an enormous dead tree becomes an antlered forest revenant. Recognition, then a deliberate turn toward the viewer.
2. **The Lantern Procession** — weighted, partially concealed travelers establish ordinary walking before one lantern continues without its carrier.
3. **The Mycelium Oracle** — a signal reaches a junction in decaying roots; layered wood opens into a reflective cavity that appears to attend to the observer.
4. **The Weaver Above** — a giant canopy creature releases two apparent tree supports as legs; its pull travels through broad silk arcs and bends the grove.
5. **The Returning Roost** — crows settle onto branches, then a substantial support vanishes while one bird calmly keeps its perch in empty air.

This supersedes the provisional01/02/03/04/05 slate in `CONCEPTS.md`. Two separately assigned reviewers selected **01/02/04/17/19**; their shared evidence makes those judgments correlated. The existing V1/V2 mood boards depict the earlier slate: they are exploration, not visual proof of Weaver or Roost. The alternative storyboard studies are separately labeled. No still image demonstrates successful motion.

**First moving proof to pursue:** Hollow Crown, fixed camera, one original composed loop, with one readable trunk-to-body correspondence and head turn. Prove the creature without detailed bark, glow or fog first. Do not build all five at once or release another pack before the core shot is convincing.

## What failed before, and what changes

The inspected native square/wide/tall contact sheet shows small friendly symbols, sparse staging and a flat rain treatment. Prior compatibility and updater checks remain engineering evidence. They do not rescue rejected artwork. The new brief asks for large-scale presence, apparent depth, material behavior and a scene that develops over time.

The research changes the production question. Keep the existing C++/Qt/OpenGL mapper. Compare a richer offline-authored loop through existing video support with native effects where their adjustable behavior actually helps. This avoids making a new renderer a prerequisite; it does not promise that a detailed still can automatically become a convincing movie.

## Evidence that changed the design

| Evidence | Design consequence | Limit |
|---|---|---|
| URBANSCREEN describes FinsterWald as changing living forest imagery; Moment Factory describes transforming natural surroundings with light, projection and illusions. | The environment participates in the supernatural event. | Creator accounts establish intent and production, not our animation quality. [FinsterWald](https://www.urbanscreen.com/finsterwald/), [Forest of Shadows](https://momentfactory.com/products/forest-of-shadows). |
| Blaschke's production accounts discuss shaped light and controlled dark-scene tones. | Place a threatening silhouette against one purposeful broad light field; keep the palette restrained. | Film exposure is not projected-wall contrast. [ASC](https://theasc.com/article/conjuring-a-coven-for-the-witch/), [Focus Features](https://www.focusfeatures.com/article/nosferatu_interview_jarin-blaschke). |
| Original animator craft accounts distinguish recognition holds, preparation and living held poses. | Recognition precedes the uncanny breach; duration follows the event rather than forcing all scenes into24seconds. | Exact times below are authored trials. [Burgess](https://www.animationmentor.com/blog/timing-the-12-basic-principles-of-animation/), [Hurtt](https://www.animationmentor.com/blog/anticipation-the-12-basic-principles-of-animation/), [Kelly](https://www.animationmentor.com/blog/why-all-animators-need-to-master-the-moving-hold/). |
| Bridson's original curl-noise paper provides coherent flow with static/rigid boundary constraints. | Use coherent forest flow rather than unrelated random dots; reacting to a changing creature is an authored goal for the composite. | Deforming-body flow is explicitly future work in the paper's section2.5, not a solved creature-fog method to import. No aesthetic/performance result is established here. [Paper](https://www.cs.ubc.ca/~rbridson/docs/bridson-siggraph2007-curlnoise.pdf). |
| Barco distinguishes mixed-image contrast from sequential contrast; AVIXA connects important element size to viewing ratio. | Design broad local separation and important silhouette landmarks; do not count on tiny bright eyes or bark strands. | No universal creature size/gray-code threshold is supplied. [Barco](https://infopages.barco.com/rs/141-TQB-685/images/Whitepaper%20TOBE_ANSI%20rev1.pdf), [AVIXA](https://www.avixa.org/resources/display-image-size-calculators/learn-more-about-display-size). |
| Cornell documents crow shape and rowing wingbeats; Kew describes fungal hyphae/networks. | Keep large crows anatomically consistent; distinguish fungal material from roots and a literal reptile eye. | Vanishing supports and sentient wood are original fiction. [Cornell](https://www.allaboutbirds.org/guide/American_Crow/id), [Kew](https://www.kew.org/read-and-watch/fungi-hidden-dimension). |

The empirical perception studies in04-fear.md inform testable hypotheses, not universal scare formulas. Attention capture, uncanny processing and fear are different outcomes. Ask viewers what happened before asking whether it was scary. Research-access limitations and inferred transfers remain explicit in each report. The source auditor found a working [original Saygin article at PMC](https://pmc.ncbi.nlm.nih.gov/articles/PMC3324571/); this repairs access for the sampled study without retroactively claiming that04read its failed lab PDF.

**A motion reference changed the transfer.** Following the auditor's challenge, specialist13sampled the original Transfiguration film during live playback through an opening material change, including6.49,22.27,34.68and38.55seconds. Coarse head/torso/leg organization persists while water sheets and flames disrupt precise outer contours. Discrete observed poses and the artist's walking account do not establish which cue primarily carries identity or prove continuous cadence. Consequently our stationary branch/antler correspondence is **our hypothesis**, not a reference-observed fixed-silhouette method. Compare a held shape-transition with a continuous weight-transfer/head-arc version in Crown's actual animatic; preserve coarse anatomy while allowing material change. [Original artist page](https://www.universaleverything.com/art/transfiguration-2024), [bounded observation and timestamps](13-motion-observation.md).

## Twenty ideas screened

All twenty receive a reasoned comparison in [10-selection.md](10-selection.md), with machine-readable criteria in [20-candidate-scores.csv](20-candidate-scores.csv). Weights: forest/spectral fit20%, distinctive idea20%, choreography20%, projected readability15%, feasibility15%, evidence10%. Scores below are **subjective concept-screen estimates**, rounded to one decimal; they are not finished-animation scores, measurements or statistically precise ranks. Reviewer uncertainty spans roughly±0.8–1.1 points. Close scores do not establish superiority.

| ID | Idea | Screen estimate | Decision and reason |
|---|---|---:|---|
|01|The Hollow Crown|8.2|Select: strongest single protagonist; expensive correspondence rig is the core risk.|
|02|The Lantern Procession|8.2|Select: ordinary body/light relation teaches a specific supernatural absence.|
|03|Murder of the Moon|7.9|First general reserve: broad eclipse reads well; flock/noise and familiar spectacle weaken distinctness.|
|04|The Mycelium Oracle|8.1|Select: different macro scale and causal material opening; avoid neon circuitry.|
|05|The Forest Behind the Wall|7.9|Site-specific reserve: strong surface illusion, higher geometry/attachment/viewing burden.|
|06|Cathedral of Roots|7.7|Take structural framing into01 only; a separate nave repeats root alignment.|
|07|The Wrong Shadow|7.8|Optional comparison beat, never a second compulsory subplot; contrast is fragile.|
|08|The Pond Below|7.5|Depth reserve; dark water and reflected geography are projection-sensitive.|
|09|The Moth Moon|7.6|Alternate wing study; symmetric unfolding can become wallpaper.|
|10|The Wicker Heart|7.4|Reserve: large readable object, insufficient consequential change.|
|11|The Witch in the Bark|7.3|Material study; face detail and caricature risk duplicate04.|
|12|The Ash Hound|7.7|Ambitious reserve; gait, anatomy, contact and ash continuity increase production cost.|
|13|Autumn Runs Backward|7.5|Quiet interlude or restoration study, weak as a lead scare.|
|14|The Bell Under the Roots|7.1|Reserve: meaning and visible causality are less explicit when muted.|
|15|Thorn Bloom|7.5|Material alternate; beauty may overwhelm agency and overlap04.|
|16|The Antler Mirror|7.5|One-beat comparison only; second deer and water multiply demands.|
|17|The Weaver Above|8.0|Select: overhead articulation and tension add a distinct spatial action; avoid thin-web dependence.|
|18|The Shed That Walks|8.0|Strongest Crown rival; raw score exceeds17, excluded for overlap with deadwood becoming an agent.|
|19|The Returning Roost|8.1|Select: missing load-bearing support is a specific, large negative-space contradiction.|
|20|Last Harvest Fire|7.3|Optional autumn coda; too gentle/familiar to lead this brief.|

18 and19 replace the weaker Black Rain and Drowned Portrait after the creature specialist challenged filler and repeated motifs. The final pool remains exactly20. This was a selection change, not renaming the rejected rain scene to make it seem new.

## Choreography of the five

Durations are trial edits. They may change after actual motion observation. A moving hold has material consequences; it is neither a frozen picture nor random swaying.

**Hollow Crown,28s.** 0–3: establish a nearly ordinary tree. 3–8: a root takes weight, pulling its shoulder into place. 8–11: three main antler forks and torso void become readable. 11–13.3: preserve that recognized pose. 13.3–16: optionally compare a shadow leading a controlled head turn; omit the discrepancy if it reads as a sync defect. 16–19: the entity's attention settles toward the observer. 19–28: a broad bough covers head, shoulder and root-leg in sequence while anatomy returns along the same landmarks. A head-only cover cannot conceal an exposed torso reset. Key rejection: a pasted-on head, rubber morph, unstable crown or cute deer.

**Lantern Procession,32s.** 0–11: three unequal entries establish weight, steps and lantern lag. 11–18: bodies stop and turn in sequence; lantern sway settles afterward. 18–22: carriers cross behind trunks. 22–26: one lantern continues through a clearly empty carrier position. 26–32: it passes into cover while a distant approach carries through the seam. **Drop the wrong-way shadow subplot.** No antlers, faces, hat-man cameo or evenly bobbing duplicates. Key rejection: the absent carrier can be explained as merely hidden behind a tree.

**Mycelium Oracle,20s.** 0–6: one signal takes three unequal routes into a junction, with material response following its arrival. 6–8: bark plates tighten. 8–11.5: a heavy cavity opens and holds while the network dims. 11.5–15: one small change of attention registers. 15–20: plates close, tension releases outward, the hidden interior resets. This is an invented woodland entity, not biological evidence of a fungal mind. Key rejection: a painted eye, toothy smile, reptile iris or animated circuitry with no wood deformation.

**Weaver Above,26s.** 0–5: broad slack arcs tighten between three canopy anchors. 5–10: two apparent supports peel away as articulated legs. 10–14: the body is recognized as one enormous mass against fog. 14–19: one leg plants and pulls; tension reaches the corresponding anchor and bends its canopy support. 19–26: body and legs fold behind cover; arc, anchor and cast shading settle into their initial state. Thick arcs and weight-bearing contacts carry the scene, not hairline silk. Key rejection: decorative web footage, simultaneous leg noise, or a leg sliding through a fixed trunk. Duration and mechanics are proposed here;17's new rig is untested.

**Returning Roost,22s.** 0–4: one large crow approaches an intact bough. 4–8: landing visibly loads and dips that bough. 8–10: a substantial support segment disappears with its two end stubs still visible. 10–14: the bird calmly retains its perch over the broad gap. 14–18: departure follows the original flight tangent; a few broad wood/leaf pieces return toward the missing support. 18–22: the branch is restored while the bird's return path remains hidden. Key rejection: an unnoticeable gap, a crow already flying, or a substitute twig silently supporting the feet. If this causality fails, promote03's broad eclipse instead of adding more birds.

## Deliberate disagreements and fallback rules

- **19 versus03:** a missing support is more specific than a familiar moon-flock spectacle, but03 may read faster at distance. Compare their central silhouette events before detailed production.19 is conditional, not proven superior.
- **17 versus05:** a large canopy creature avoids a wall-specific rupture, but convincing leg contact is substantial work. If17 only becomes a web drawing,05 remains a strong alternative on a known flat surface.
- **01 versus18:**18 scores competitively and has no antlers/eyes. It could replace01 if the tree/stag correspondence cannot hold. Both in the first five would repeat deadwood-to-agent recognition.
- **Merge limit:** borrow a structural cue or one behavioral beat; do not combine full subplots, every species, a portal and five glowing faces into one busy scene. Share the world and varied light causes, not identical ornament.

## Production and evidence gates

Keep the mapper baseline and saved work. Existing path caching means surfaces using the **same media path share a source/player**; different files do not have an established shared start/seek clock. Existing audio is **muted by default and manually enableable**. A silent-first review is deliberate; no audio-reactive feature is assumed.

For the first proof, test one original composed video with creature, forest and occlusion already combined. The current surface shader has no forest depth/shadow buffer; Screen/Add cannot subtract an underlying bright layer. A rich native extension is possible but would require authored poses, shading/period/control work and profiling. Media does not inherit procedural Size/Density/Flow/Reverse controls automatically. [Source inspection and technical research](05-native-graphics.md).

The offline route can use an original rig, constrained fog, recoverable frame sequence and intentional SDR encoding. Source documentation supports the workflow, not rendering speed or a convincing asset. Qt's repeat flag is not an explicit gapless boundary guarantee. [Pipeline report and original documentation](06-offline-pipeline.md).

| Gate | Reviewable output | What fails it |
|---|---|---|
|Value blocking|Three simple key poses for each selected scene: established rule, supernatural breach, recovery.|Meaning depends on bark hairs, tiny eyes, particles or explanation.|
|One full animatic|Complete Crown loop, actual geometry/material transition, normal speed and consecutive repeats.|Frozen image with glow, poor mechanics, hidden-reset leak, no recognition hold.|
|Look test|Moving light/material samples; small and lifted-black stress views.|Fine texture conceals weak staging; glow flattens depth; decoder banding swallows the subject.|
|Mapper playback|Actual media in disposable mapping; controls/save/reopen; editor+output; deformed panel; repeated EOF boundaries.|Stall/flash/seam, broken references, unmeasured load, assumed cross-file sync.|
|Distribution|Collected original media, documented references, extracted app check and retained prior runtime/data.|Artwork exists only in generation cache; update loses media or old saved work.|
|Physical projection|Intended wall, size, light and distance; watch actual moving output.|Desktop proxy presented as acceptance.|

The flat-value blocking artifact exists as [editable SVG](15-value-storyboards.svg) and [rendered PNG](15-value-storyboards.png); it was parsed, rasterized and visually inspected. This checks the static artifact, not success of the narrative gate. Weaver's strip begins **after** the creature is revealed and isolates its pull/contact mechanism; it does not yet depict ordinary canopy supports misread as legs. Crown's exposed forks in recovery do not prove that a changing crown can reset unseen. Roost's third pose isolates support restoration without its departure. The generated alternative storyboard has additional pose/restoration ambiguities; use it for exploration, not proof of a complete loop.

No motion, playback, distribution or projector gate has been passed by prose or still art. A9.5/10 **finished-animation** judgment cannot be earned here. Reviewers must state lower scores and unresolved evidence. Their anchored assessments and any final research-quality judgment are in the audit files; do not reinterpret them as public release approval.

## Team, audit and current handoff

Fifteen distinct specialist agents performed the research, critique and diagrams. [The roster](AGENT-ROSTER.md) records their actual collaboration task identities and artifacts. The final independent judge audits the source auditor and art critic as well as the synthesis. The three-worker limit requires successive waves; these were not15simultaneously running workers, blind endorsements, or human experts. Later reports saw a shared brief and therefore have correlated judgments.

Full specialist reports are supporting evidence. Start with this synthesis,10-selection.md,11-source-audit.md,12-art-critique.md,13-motion-observation.md and the final audit. `source-links.csv` is a link manifest, **not** a count of independently verified sources; some reports explicitly record failed/partial access. Mood-board prompts and generated images are included. No third-party artwork is packaged for redistribution.

Current milestone: research and art-direction comparison after visual rejection. **Next concrete work:** original Crown value poses and one complete low-detail moving proof, then judge the visible result before detailed production or another update. Current application, install, mappings and media have not been replaced.
