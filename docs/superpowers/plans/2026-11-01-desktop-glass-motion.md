# Desktop glass and motion implementation plan

**Goal:** Make Lumen Atelier genuinely translucent over other windows/desktop and allow faster shared theme motion without moving the 0.8.8 controls.

**Design:** The user's latest request explicitly authorizes faster motion and actual see-through gloss. Use the existing Qt/OpenGL stack with a Windows DWM client alpha surface. Full-app inspection established actual background visibility with zero extended-frame margins, a normal readable native caption and an opaque OpenGL canvas. Desktop Acrylic returned success but an opaque grey result; a Composition host brush obscured Qt content. Neither unverified blur path ships. This increment supplies clear glossy translucency, not a claim of Gaussian blur or Acrylic.

**Constraints:** Source, Git, projects/media, current working runtime and rollback retained. Combined project cap 1,900,000,000 bytes. No desktop capture, wallpaper assets, new framework, scene-format changes, projector effects or control relocation. Menus/dialogs retain their existing readable material. Windows/system and app transparency/contrast fallbacks remain opaque. Pause affects the app's reflections, not other applications behind it.

**Implementation:** Inline execution using the writing-plans and executing-plans workflow; existing isolated feature branch/build cache reused to avoid duplicated outputs.

- [x] Add a small Windows-native backdrop adapter; prepare alpha before creating the editor window, preserve native frame/window identity and enable only for Lumen without reduced transparency/high contrast. Handle composition/setting changes and failure with the original opaque material.
- [x] Make Lumen's editor foundation and panels translucent, with one shared moving gloss layer. Preserve readable controls, all existing profiles, opaque canvas/output and exact layout.
- [x] Extend Speed from 0.2–1.2 to 0.2–2.4 consistently in settings normalization, runtime clock and slider; retain existing values/default and pause/light behavior. Set the live session to 1.8 after installation.
- [x] Verify actual desktop object visibility/change, native resize/maximize/theme switching, pause/speed/contrast, saved project/media hashes and unchanged output. Run relevant packaged native suites and launcher update/reopen/rollback with a synthetic project.
- [x] Install a fresh 0.9.8 runtime through the launcher, retain 0.9.7, reopen the saved user mapping, record storage and handoff. No public release/Store certification claim.
