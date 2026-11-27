# Specialist 06 — offline forest / creature animation pipeline

15 October 2026. Research and proposed production route only. No installation, implementation, render, encoding, app playback test, or publication was performed. Read `00-brief-and-candidates.md`, README, AGENTS, `src/model.cpp`, and the relevant media/color conversion in `src/canvas.cpp`.

## Recommendation

Make **one original 24-second Hollow Crown movie**, with a fixed camera and one large, rooted creature, before building five scenes. Author the environment, creature, light, occlusion, and reset together in Blender; render recoverable image frames; encode one silent SDR H.264 MP4; load that one file on one mapper surface. This is a credible route to sculpted form and directed lighting without making the mapper itself a creature renderer. It is a production recommendation, not evidence that the result will be good or run at a particular speed.

The first proof should avoid a walking deer. A convincing walk requires gait, weight shifts, planted feet, contact shadows, and deformation in addition to atmosphere. A rooted bark presence can achieve a demanding visual result with a torso/neck/head turn and expressive antler silhouette. Its restraint must read as deliberate scale and menace, rather than as an unmoving asset hidden by smoke.

## What the actual mapper permits

- `model.cpp:35–44`: a media source constructs a `QMediaPlayer`, defaults its audio output to muted, assigns a video sink, requests infinite looping, and starts playback after setting the file source. **Review correction:** `main.cpp:154–155` exposes the existing Mute audio checkbox, which can enable audio; line387 restores its selected state. Muted-by-default does not mean audio is unavailable. The proposed first proof is deliberately silent; no actual speaker playback or mute persistence was tested in this research.
- **Important qualification:** `Scene::source`, `model.cpp:260–264`, caches media sources by exact path. Two surfaces using the same path reuse that source/player. Distinct files have distinct players; this inspection establishes no common start/seek clock for them. A coordinated multi-file show remains unproven. Same-file reuse also does not provide separate crops of a master canvas by itself.
- `model.cpp:55–79` accepts a restricted SDR NV12 fast path, including BT.709, with image conversion as fallback. `canvas.cpp:188–195` chooses a matrix and expands limited-range luma using the frame's color metadata. HDR transfers are excluded from that native path. File tags and decoded format therefore matter.
- README documents meshes, looping video, brightness/opacity, and Normal/Screen/Add. These do not provide depth-aware compositing between a movie creature and a separately animated tree. In the proposed proof, essential foreground/creature occlusion is baked into the movie.

Qt documents `Infinite` as repeated playback. It does **not state a gapless, frame-continuous boundary guarantee** on this page. The deployed-version [Qt 6.10.3 QMediaPlayer reference](https://doc.qt.io/qt-6.10/qmediaplayer.html#loops-prop) supports the repeat request; it does not prove this application's decoding latency, frame delivery, or boundary appearance.

## Route comparison

These are engineering/art judgments from the inspected architecture, not measured frame rates.

| Route | What it buys | Real cost / constraint | Decision for this proof |
|---|---|---|---|
| Blender offline → one movie | Artist-controlled shape, material, light, depth, shadow, and exact event timing; complex scene work becomes video playback at runtime. | Requires modeling, animation and look development. Rendering can take hours. Baked camera and lighting cannot respond to arbitrary mapped geometry. Codec and playback still need testing. | Preferred. Separate the art question from writing a new renderer. |
| Native Qt/OpenGL 3.3 effect | Small distribution, adjustable speed/color/density, deterministic shared procedural clocks where implemented. | Convincing anatomy, articulated silhouettes, shadows, depth fog, antialiasing and choreography all become new authoring/rendering work. Existing small cartoons do not establish a high-quality creature pipeline. A 30 fps runtime has roughly 33.3 ms per frame for all work. | Suitable later for restrained environment accents; not the least-risk way to prove this creature. |
| Hybrid movie + native accents | Keeps the hero sculpted while adding adaptable edge activity or sparse ambient motes. | Separate clocks can drift; the native layer cannot infer where the movie's trunks occlude a particle. Extra brightness can flatten the hero. | Add only after the single movie passes. Begin with accents that do not need narrative synchronization or depth intersection. |
| Blender 2.5D fallback | A modeled/rigged head and layered silhouette grove can preserve scale, a controlled reveal, and the turn while reducing scene and fog cost. | More limited light interactions; cards can look like theatrical flats. Needs deliberate overlaps and no camera orbit. | Legitimate fallback if fully volumetric work is too costly. Reducing the hero to another tiny glowing icon is a quality failure. |

Offline production removes the requirement to render the forest in real time; it does not remove the need to render it efficiently, and it does not make a slow decoder fast. Blender renderer/device compatibility must also be checked separately from the mapper's GL 3.3 requirement.

## Five primary documentation sources and their limits

Accessed on 15 October 2026. The Blender `/latest/` pages retrieved directly returned Blender **5.2 LTS Manual** content; several web-tool opens failed, so those three pages were successfully read through `Invoke-WebRequest`. Do not substitute the search engine's 5.4 development pages. The Qt link is deliberately pinned to the app's documented 6.10 series. FFmpeg's live all-in-one manual is capability documentation; the README names bundled **FFmpeg 7.1.3**, and neither that deployment nor an installed encoding executable was tested here.

1. [Blender: armature introduction](https://docs.blender.org/manual/en/latest/animation/armatures/introduction.html). Bones can move/deform associated objects; bone animation belongs in Pose Mode. This supports a controllable rig, not an automatically convincing creature.
2. [Blender: EEVEE volumes](https://docs.blender.org/manual/en/latest/render/eevee/render_settings/volumes.html). EEVEE evaluates volumes in the camera frustum with 3D textures; resolution, steps and depth range trade quality/precision against memory. This supports bounded fog design, not a claim that dense fog is cheap on this host.
3. [Blender: rendering animations](https://docs.blender.org/manual/en/latest/render/output/animation.html). Frame sequences permit recovery and separate compositing/encoding; PNG is a documented lossless option. Intermediate frames are part of its recommended workflow. This supports a recoverable pipeline, not a render-time estimate for this scene.
4. [Qt 6.10.3: QMediaPlayer](https://doc.qt.io/qt-6.10/qmediaplayer.html). Establishes looping/playback API behavior, with no explicit gapless promise.
5. [FFmpeg: complete documentation](https://ffmpeg.org/ffmpeg-all.html). The `libx264` section documents CRF, presets, GOP size, B-frames, and closed/open GOP controls; the `scale` section documents output matrix/range conversion. This supports an encode specification and controlled experiments, not compatibility or performance certification in Potato Mapper.

## Practical scene construction

Everything in this section is proposed art direction using those capabilities.

**Original asset and rig.** Build an asymmetrical, elongated bark skull/chest with broad root shoulders. Its head/antler span should occupy roughly half the image width at revelation, with some antlers joining surrounding branches. Keep the mouth unreadable and the eyes subordinate to silhouette; no large round eyes, grin, or friendly bob. Use a compact rig: root, torso, lower/upper neck, head, and a few branch controls. Rigid antlers follow the head rather than rubber-bending with the skin. Test neck weights in the full turn before adding fine bark. A silhouette render should already communicate the event.

**Depth and light.** Use actual overlaps: near trunks framing and partially blocking the creature; the creature and root shoulders in the middle; receding, less contrasted trunks behind it. A fixed perspective camera and changes in occlusion supply depth without flying through a portal. Use a broad, cool rear/side light to separate one contour and a restrained warm accent near a hollow. Keep unlit mass in the creature. Detailed bark is a reward near the highlight, not the only evidence that something is there.

**Fog.** Begin with one bounded, low-density volume behind and around the middle layer, rather than filling the entire world. Restrict the useful depth range. Fog must separate planes and reveal selected light shafts; a white cloud over the whole image destroys the black reserve and creature contrast. Try a static density field with controlled cyclic light before attempting simulated smoke. Simulation caches and arbitrary advecting noise add reset problems without necessarily improving the shot. If needed, substitute sparse soft cards placed between layers; approve their visible intersections before committing.

**Render path.** Preview motion in solid/silhouette mode; approve a few lit key poses; then render image sequences rather than directly into the final MP4. EEVEE is the first candidate to benchmark, not a promised speed. If its artifacts or lighting limit the shot, compare a short Cycles sample and its cost before switching. If neither is practical, simplify fog, secondary geometry, and resolution in that order while protecting silhouette and timing. Do not attempt to cure bad silhouette with more samples.

## One 24-second hero, 720 delivered frames

| Time | Readable event | Reset discipline |
|---|---|---|
| 0–4 s | A deep grove is already present. A foreground branch moves slowly; a hollow receives a small light change. | Start in the same occluding configuration used at the end. No obligatory black opening. |
| 4–9 s | A side light makes scattered branches align into the enormous antlered presence. | The creature exists in depth; this is a directed reveal, not a new sprite fading in. |
| 9–15 s | One slow head turn breaks the alignment and exposes the neck/root mass. Hold long enough to recognize it. | No gait, lip animation, or flock competing with the turn. |
| 15–20 s | Near branches pass across the head as the light retreats; the grove takes ownership again. | Occlusion must hide every part that will reset, including its shadow and local fog/light contribution. |
| 20–24 s | Hidden head/neck return; the foreground branch and light return to their starting phase. The forest continues. | No whole-screen fade or conspicuous reverse playback. |

Set 30 fps before animation. Deliver frames 1–720: sample time `(frame−1)/30`. A helper pose at frame 721 equals frame 1, but **do not encode that duplicate endpoint**. Background motions must use compatible periods such as 24, 12, or 8 seconds, or reset entirely behind an opaque occluder. Match endpoint derivatives as well as poses; a matched position with reversed velocity still jerks. Motion-blur shutters crossing the boundary require cyclic animation outside the exported interval. Arbitrary particle and fog simulations do not become loops merely because the head does.

This schedule is achievable as a focused shot because it has one fixed viewpoint, one partial-body rig, and one reveal. It is still real art production. It is not achievable by renaming the current deer/ghost icons or adding a vignette.

## Output specification and preservation of dark detail

Proposed first delivery: **1920×1080, square pixels, progressive, constant 30 fps, exactly 720 video frames, silent MP4, H.264 8-bit 4:2:0 SDR**. Keep a 1280×720 alternate if decode/upload performance fails. Begin art reviews at 960×540, and also inspect the final image reduced to 480×270. These are test sizes, not claims about the user's projector's native resolution. Do not pay for a 4K first render while the silhouette and light remain undecided.

Retain the `.blend`, original created assets, and lossless frame sequence. Keep linear high-dynamic-range intermediates if compositing needs them; produce an explicitly display-referred SDR master before encoding. Do not tag raw linear values as display video. A Blender display transform and a video range conversion are different operations.

Start the H.264 encode experiment around CRF 18 and a medium preset, with a closed GOP and a self-contained initial keyframe; these are chosen starting settings, not a documented guarantee of invisible compression. A 30–60-frame maximum GOP is a reasonable test range. An all-intra comparison can help diagnose prediction artifacts but increases file size; it does not cure player restart latency. Confirm the actual encoder build supplies `libx264`; the mapper's decoder libraries do not establish that an encoding CLI exists. No installation is authorized by this recommendation.

For the delivery experiment, choose an intentional BT.709 YCbCr matrix and limited range, with matching tags; use actual conversion rather than metadata relabeling. FFmpeg documents [full versus limited conversion in its scale filter](https://ffmpeg.org/ffmpeg-all.html#scale) and [x264 controls](https://ffmpeg.org/ffmpeg-all.html#libx264_002c-libx264rgb). Document the master transfer and perform any needed transfer conversion explicitly. 8-bit limited luma nominally uses 16–235; the mapper has a metadata-driven expansion path, so a range mismatch can lift blacks or crush the reveal. Never compensate for a conversion error by arbitrarily darkening the art.

Use a separate diagnostic wedge containing black and several near-black steps through the **same** master→encode→decode→mapper route. Compare Normal blend first. Screen/Add can reveal a lower layer through black but cannot create physical darkness; a lifted encoded background can wash out the forest. Avoid one-pixel saturated veins, very dark fine texture and enormous smooth glowing gradients: chroma subsampling, quantization and projector contrast can erase or band them. Check the encoded result, not only Blender's render view.

## Does the loop seam survive decoding?

**Unknown until tested.** There are three different seams:

1. **Authored seam:** pose, velocity, fog, exposure, shadow and noise state must all be cyclic or hidden. Frames 720 and 1 should be successive motion samples, not necessarily identical images.
2. **Encoded seam:** the delivered file must decode to 720 ordered frames at 30 fps; a self-contained beginning and closed prediction structure avoid needing a preceding file segment. Compression can still change dark gradients or apparent boundary detail. Inspect decoded beginning/end against the lossless master.
3. **Playback seam:** QMediaPlayer may need to reset/demux/decode at EOF. Artistically invisible endpoints cannot rule out a repeated/held frame, flash, or timing pause introduced there. Watch and record at least ten **actual mapper** boundaries, including editor plus output, and compare with a separately encoded multi-cycle clip whose internal boundaries do not require EOF restart. This isolates authored/codec defects from restart behavior.

If EOF restart alone fails, report it as a playback limitation. A longer multi-cycle file reduces restart frequency but does not solve it. Seamless shared transport or a frame-sequence player would be a separate reviewed implementation; do not imply it exists. Do not split the hero into independently playing fog/creature/tree movies to dodge this issue.

## Work breakdown, measurements and hard failures

Planning estimate: several focused art days plus rendering for a practiced operator; a beginner may need substantially longer. No delivery date follows from the documentation. Gate each stage so expensive frames do not conceal an unapproved shot.

| Stage | Concrete output / acceptance |
|---|---|
| 1. Composition | Three original key-pose silhouettes at 960×540. Creature reads at 480×270; near/middle/far overlaps survive. |
| 2. Original asset and rig | Partial-body sculpt/model, antlers, grove and compact rig. Head-turn playblast has stable attachment and plausible weight; archive sources. |
| 3. Choreography | Entire 24-second unlit playblast repeated three times. One unmistakable reveal; occluder covers all reset effects; no snap. |
| 4. Look development | Lit samples at start, peak turn, occlusion and boundary; a short moving fog/light sample. No denoise/volume shimmer, swallowed head, or uniform bloom. |
| 5. Cost gate and final frames | Time several representative frames including the heaviest fog/occlusion; measure peak memory. Extrapolate with headroom. At 10/30/60 seconds per frame, 720 frames alone cost about 2/6/12 hours, excluding retries. These are arithmetic scenarios, not host measurements. Render recoverable frames. |
| 6. Media proof | Encode and decode inspection, black wedge, three-cycle comparison, at least ten mapper EOF boundaries, 1080p versus 720p if needed. Record hardware, app/runtime, format, elapsed playback and observed frame delivery. |
| 7. Projection acceptance | One saved `.pmap` with linked/collected media, reopened successfully; actual viewing on the wall checks contrast, silhouette size and seam. Desktop proxies cannot complete this gate. |

**Reject the proof** if the creature is small/cute, the forest is empty wallpaper, the head looks pasted over trunks, its neck collapses, occlusion order is wrong, fog veils the event, highlights remain uniformly maximal, the reveal only reads with sound, the reset is visible, dark gradients pulse/band badly after encoding, playback stalls or flashes at EOF, or reduced-size/lifted-black inspection loses the main event. A strong still does not pass a moving-shot gate. A smooth decoder does not pass an art gate.

This focused pass recommends the offline hero as the first quality experiment. The remaining uncertainty is the actual original asset, moving art quality, render cost, exact delivered decode path, and physical projection. None was established by this research.
