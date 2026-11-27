# Specialist 07 — projection contrast and readability

15 October 2026. Focused research and visual critique only. I read `CONCEPTS.md` and `00-brief-and-candidates.md`, and inspected the actual `hollow-wood-concept-v1.png` through the image viewer. No projected image, moving render, physical surface, light reading, calibrated display or audience test was available. The recommendations below are authoring hypotheses, not measured acceptance results.

## Finding

The board has a compelling severe woodland vocabulary, but its miniature panels depend heavily on photographic bark, roots, trailing fibers and twig networks. The main projection risk is **insufficient separation between important neighboring shapes**. Merely lifting the entire image would also lift the foreground clutter and weaken the intended darkness. Redesign the lighting and negative space around the action first. Keep darkness in large occupied masses; reveal the action against one controlled field of lighter fog, wall or moon.

The existing layout also biases comparison: Hollow Crown occupies most of the board, while the other four treatments are compressed into small tiles. This does not establish that the Crown would outperform them at equal projected size. Titles, border and decorative type are presentation furniture; they should not appear in the projected effects.

## Three primary sources actually accessed

1. **Barco, The importance of ANSI contrast ratio in cinema**, six-page vendor technical whitepaper, opened successfully as PDF. Pages 3–5 distinguish sequential contrast from simultaneous checkerboard contrast and explain how ambient light, projector scatter and room reflections raise measured blacks. This is directly relevant to a bright moon beside dark branches: an on/off specification does not describe that exact mixed image. Transfer limit: the paper discusses cinema systems, not Shane's unknown home projector. It supplies mechanisms, not a guaranteed home contrast ratio. [Primary PDF](https://infopages.barco.com/rs/141-TQB-685/images/Whitepaper%20TOBE_ANSI%20rev1.pdf).
2. **Christie, Guidelines for government AV specialists designing high-performing, secure, and reliable display solutions**, seven-page vendor whitepaper, opened successfully as PDF. Page 4 discusses dark-content contrast and ambient light; light-colored room surfaces can reflect light back onto the image. Reducing room illumination helps preserve contrast. Transfer limit: its equipment and venue sales guidance is not a purchasing prescription for this project; its useful contribution here is the environment dependency. [Primary PDF](https://www.christiedigital.com/globalassets/help-center/whitepapers/documents/government-solutions-white-paper.pdf).
3. **AVIXA, Learn More About Display Size**, official standards explanation, opened successfully as HTML. It defines element height relative to active image height and ties minimum element size to viewing ratio, with separate analytical/basic decision-making acuity factors. This supports sizing the information that matters, rather than relying on output resolution alone. Transfer limit: its information-display table is not a horror-art standard, and an antler branch is not equivalent to a text character. I do not apply its table as a creature-recognition threshold. [Primary page](https://www.avixa.org/resources/display-image-size-calculators/learn-more-about-display-size).

Access note: Extron's videowall shape/size and font-legibility pages appeared in search results, but direct opens returned request-rejection pages. They are not counted as accessed evidence or used to justify a numerical design rule.

## Physics versus invented design rules

**Physical relationships / sourced mechanisms:** Projected black cannot subtract illumination already reflected by the wall. In a simplified additive model, observed contrast between two patches is `(L_high + L_ambient)/(L_low + L_ambient)`. The model assumes the same additive ambient contribution to both patches and holds optical/room behavior fixed. Real walls and mixed images vary spatially; room bounce can depend on the image itself. Raising ambient drives that ratio toward one. A nominal black source value therefore does not guarantee a black-looking wall. Barco and Christie explain the corresponding real-system losses above.

Spatial detail must subtend an adequate angle at the eye. For a feature of physical size `s` viewed at distance `d`, angular size is `2*atan(s/(2*d))`; for small angles this is approximately `s/d` radians. If a 1920-pixel-wide image occupies a 3 m wall, a four-pixel-wide feature is 6.25 mm wide before mapping and optical losses; at 4 m it subtends about 5.4 arcminutes. This is a geometric example, **not** a recognition threshold or a claim that those are Shane's dimensions. The same feature may blur, merge with adjacent branches or lose local contrast even if the source file contains it. Viewing distance is viewer-to-wall distance; projector throw distance is a different variable.

**Authored hypotheses for the next comparison:** three major value masses, a small number of large silhouette landmarks, wider antler gaps, limited luminous paths, and a few broad light pools. There is no universal rule that a projected creature must occupy a specific percentage, that all lines must exceed a fixed pixel width, or that a particular gray code survives a home projector. Proposed counts and proxy settings below are deliberate stress choices to be revised after observation. Do not label them AVIXA requirements or calibrated projector simulation.

## Critique and concrete redesign of all five

### 1. The Hollow Crown

**Actually visible:** a large skeletal stag/tree hybrid with a clear hanging head, hollow rib cage, two small amber eyes and a bright mist region behind its upper-right antlers. The left third contains enormous near-black trunks. The antlers, background boughs and hanging moss overlap in a dense network. Much of the torso's material identity comes from fine brown ridges against similar brown/gray forest textures.

**Failure risk:** eyes may survive as two dots after the neck, shoulder and antler structure disappears; that is weaker than recognizing a forest body. Thin antler forks can merge into background branches. The hero looks detailed because the panel is large, not because its full silhouette is clean. The hottest apparent sky patch may draw attention above the head rather than into the transformation.

**Redesign:** place the shoulder, neck and three main antler forks against one broad fog opening with no competing branches crossing those landmarks. Keep only two foreground framing trunks and move one away from the torso. Make the rib opening one simple dark cavity rather than many equal thin bars. Use a broad shoulder/top-spine light plane and a short cheek plane; reserve amber eyes as a secondary cue. During the turn, retain a visible gap between jaw and trunk, then let one deliberately broad foreground bough conceal the reset. Preserve the severe skeletal anatomy; do not solve separation by outlining the whole deer in neon.

**Proposed reject check:** hide the eyes and bark detail. If the creature's shoulder, turned head and antler gesture cannot be identified, the composition still depends on ornament.

### 2. The Lantern Procession

**Actually visible:** a looming close traveler on the left, several progressively smaller hooded figures, a winding track, tiny amber lanterns and a separate dark hat-wearing figure on the right. Ground reflections are small irregular highlights. Robes and surrounding trunks share long vertical dark shapes.

**Failure risk:** the lanterns can read as floating dots because robe/hand relationships are fragile. Distant travelers become repeated tree-like strokes. The extra hat figure complicates the collective identity. Tiny pools cannot convincingly reveal where feet meet the path at distance.

**Redesign:** build a procession of three clearly different body poses distributed across near/middle/far depth, with only one near figure carrying the main visual weight. Give two lanterns broad, soft pools that reveal a hand or knee and a short section of ground. Separate hoods from trunks with a lighter mist strip behind the shoulders. Make the empty lantern's carrier absence a large interruption in an established relationship: show a readable hand above the lantern, conceal the body behind a trunk, then return the lantern into the same pool without the hand. Remove the unrelated hat figure from this comparison. Keep distant lights as depth cues, not additional full-size events.

**Proposed reject check:** temporarily replace bright lantern cores with medium gray. The route and bodies must remain readable before evaluating the missing-carrier reveal.

### 3. Murder of the Moon

**Actually visible:** one large foreground crow with a very readable wing fan, a pale moon in the upper right, a dense diagonal band of hundreds of birds and fine bare forest silhouettes below. The moon already provides a useful broad light/dark opposition.

**Failure risk:** the flock's tightly packed small silhouettes become granular noise, especially where they cross the moon edge. Many isolated bird flecks demand resolution without adding a distinct action. A large bright moon could also raise room bounce and reduce the dark foreground's apparent depth; that depends on the actual setup, not merely source RGB values.

**Redesign:** keep one foreground crow, a few mid-distance birds with distinct wing phases, and one coherent, simplified distant band. Preserve open gaps inside the arc so the direction is visible without tracing individual specks. Dim/tarnish the moon enough to maintain hierarchy while keeping it broad enough for a clean eclipse; judge this physically rather than imposing a brightness percentage now. Form the impossible wing as one large silhouette with a decisive lower contour. Remove leaf particles until the wing-to-leaf transition reads in large shapes.

**Proposed reject check:** reduce the flock to flat masses. If the arc and eclipse lose all interest, the concept currently relies on particle density rather than choreography.

### 4. The Mycelium Oracle

**Actually visible:** a close amber slit eye surrounded by deeply layered bark, several bright branching mycelial strands across the lower-left foreground, tiny mushrooms and shallow-focus material detail. The upper eyelid's heavy arch is a strong form. The luminous network is considerably finer than the eye.

**Failure risk:** the eye reads more reliably than the claimed propagation network, so the board presently supports a bark-eye encounter more than a causal organism. Tiny filaments can become scattered sparkles. The eye and network can compete if both receive equal glow. Mushroom textures do not carry the dramatic act.

**Redesign:** simplify to three broad asymmetric root paths converging on one knot below the lid. Light only one traveling segment at a time, with a visible bend/crease preceding the eye opening. Enlarge the lid fold and give its underside a broad dark wedge against a modestly lighter lower bark plane. Make the iris a small number of broken ring masses, not fine radial engraving. Suppress the glowing roots while the eye opens; return them during closure. Remove mushrooms from the proof unless they provide a readable scale cue.

**Proposed reject check:** replace all bark with three flat tones. The opening lid must deform and the traveling pulse must arrive at a recognizable junction before the eye responds.

### 5. The Forest Behind the Wall

**Actually visible:** a fairly bright cracked plaster border surrounds a root-lined irregular opening, a distant forest, and a partly hidden tall antlered creature. The light outer wall and darker opening provide the board's clearest large-scale value grouping. Roots and cracks are finely detailed; the creature's thin legs overlap trunks.

**Failure risk:** the portrait-shaped opening resembles an ordinary framed picture. Its thick bright surround may dominate the dim entity. Fine crack networks cannot carry the illusion that the physical wall is pulling apart. The depicted plaster color may mismatch the actual projection surface, and apparent depth is viewpoint-dependent.

**Redesign:** use the actual mapped panel boundary as the anchor in the proof, with one broad seam that visibly begins inside that panel. Keep an asymmetrical hinged edge, a consistent apparent thickness and a moving contact-shadow strip; remove hairline crack webs. Reveal three staggered trunk masses behind it. Show one broad shoulder/head interruption moving through gaps, rather than thin full legs. Reduce the outside wall's luminous area and keep a neutral alternative whose look can be tuned to the real surface. Close by restoring a few large edge landmarks precisely. Test the root opening against a plain rectangular-video control to establish whether it adds a surface illusion.

**Proposed reject check:** hold the entity out of view. If seam motion and edge attachment do not suggest the surface changing, adding a creature will only decorate a portal.

## One low-detail comparative board specification

Create a new board titled **The Hollow Wood — readability study, unprojected**. Use five equal 16:9 scene panels, with a separate sixth legend panel, arranged as a three-column/two-row sheet. Each scene must also be available individually at the same image size; a contact sheet is for comparison and must not become the actual projection layout.

Use a deliberately limited five-step grayscale palette and one muted amber accent. These are illustration constraints, not a calibrated luminance scale. No bark microtexture, moss hairs, tiny mushrooms, fine crack webs, tiny crow flecks, bloom fog over every edge, or decorative projected labels. Allow at most three major depth groups, each established by overlap and a clear value boundary. Bright accents must explain a nearby form, not exist as detached dots. Give every panel one conspicuous action landmark:

| Treatment | Large proof shapes in the new board | Keep visible when detail is removed |
|---|---|---|
| Hollow Crown | Two framing trunks; broad mist aperture; bent shoulder/neck mass; three antler forks | Jaw gap and antler gesture, even with eyes hidden |
| Procession | Curved ground strip; three distinct hood/robe masses; two broad light pools | Hand/lantern relationship and an unoccupied return position |
| Moon | Tarnished moon disk; one foreground crow; one open flock band; low canopy | Clean arc and one impossible wing contour |
| Oracle | Heavy bark lid; three asymmetric root channels; one junction; broken iris rings | Causal path to the eye and lid thickness |
| Wall | Neutral panel field; one hinged seam/edge; three depth trunks; partial shoulder | Surface attachment before the creature arrives |

The legend must say **original proposal; not an animation, projector simulation or physical test**. Do not give one concept a larger tile. Pair each scene with a small silhouette-only inset if space permits; omit the inset before reducing main scene size.

## Proposed checks and what each can establish

1. **Source-frame screening:** inspect each equal-sized scene at native size, quarter linear size and in grayscale; hide texture/eye/glow layers when applicable. Record which landmarks remain, rather than assigning an unsupported pass percentage. This screens composition, not home projection.
2. **Deliberate image degradation:** compare a linear-light additive veil and mild blur at several explicitly arbitrary settings. Label settings and preserve the original. This can expose fragile low-contrast boundaries; without measured room/projector response, it is not a projector simulator. A dark gamma transform or black-code lift alone does not model room reflections or optical scatter.
3. **Moving proof:** watch one full act at a time with fixed camera and no sound. Verify that fog reveals rather than repeatedly hides the only landmark, that near/middle/far bodies occlude correctly, and that the reset does not require the viewer to miss a cut. A still board cannot answer these questions.
4. **Actual one-projector observation, still pending:** log projector model/mode, surface color/material, image width/height, ambient lighting condition and viewer distance/position. Examine a full-black patch, a mixed bright/dark pattern and the actual scene at the same settings. Observe from intended near/far and side positions. Report separately whether viewers recognize the entity, track the action, and perceive depth; a prettier phone exposure is not proof of live visibility. If using a meter, describe its type and sampling method before reporting contrast numbers.

No physical test has been run and no projection-ready artwork or animation has yet been produced by this specialist pass. The immediate useful revision is the equal-sized low-detail board; the strongest failure gate is whether each main act survives removal of its microdetail.
