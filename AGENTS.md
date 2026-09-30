# Potato Mapper

Read README.md and DEVELOPMENT.md before changing the app. The goal is a practical, watermark-free mapper for a home setup with one projector.

- Use C++17 and the existing Qt/OpenGL stack. Extend the working app rather than replacing its framework.
- Prioritize an executable the user can try. Build, run the relevant behavior, fix actual failures, and continue in manageable steps.
- Keep verification proportional to the change. Do not generate large test banks, audit reports, placeholder features, or unrelated documentation.
- Save the user's open mapping before restarting the app. Preserve personal project files and media; they are intentionally excluded from Git.
- Keep editor and projector rendering in sync, and keep editor handles and labels off the projector output.
- Animation updates should not create undo records or mark a project dirty; user setting changes should.
- Explain what changed, how to try it, and what was actually checked. Do not claim physical-projector testing based only on rendering to a local output window.
- Use branches prefixed `codex/` for future feature work unless the user specifies otherwise.
