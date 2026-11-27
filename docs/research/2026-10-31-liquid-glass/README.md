# Windows Liquid Glass 1: reference and decision pack

Researched October 31, 2026 before implementation, using Windows 11/Fluent 2, Apple's 2025 Liquid Glass design and current desktop application evidence. This is a guidance pack, not a claim that native platform effects or Store certification have been obtained.

1. [Microsoft and Apple: material, optics, hierarchy, desktop geometry and accessibility](windows-apple.md) — 12 primary pages, WWDC25 transcripts, official visual references, native API versus Qt limits.
2. [Seven actual app references](app-references.md) — Sketch, Craft, Pixelmator Pro, Final Cut Pro, Raycast, Linear and PowerToys. Separates statements in release notes from details observed in official images. Three palette alternatives with contrast calculations.
3. [Qt implementation and storage feasibility](qt-feasibility-and-safety.md) — existing source map, Qt 6.10 references, bounded optics/cache design, output/data boundaries and freshly inventoried generated artifacts.
4. [Chosen whole-app design specification](../../superpowers/specs/2026-10-31-liquid-glass1.md) and [implementation plan](../../superpowers/plans/2026-10-31-liquid-glass1.md).

## What the evidence changed

| Temptation | Chosen treatment and reason |
|---|---|
| Recolour Material | A separate optical renderer: generated environment transmission/lensing, directional reflection and meniscus, plus its own full control palette |
| Float/rearrange everything | Retain familiar tool positions and compact desktop controls; improve their actual finish |
| Blur every child | One coherent shell with stable field/list beds; no layered blur over numeric labels |
| Make every action colourful | Neutral smoke/pearl with jade for selection/focus; semantic status colours remain distinct |
| Copy native Apple effects | Implement an honest Qt interpretation and document the difference; retain Windows-owned caption/file picker |
| Run continuous shimmer/capture | Cached optical plates, bounded opening-only menu sample, local pointer response without an idle animation timer |

## Limits

Product images demonstrate composition, not source-level implementation or performance. The palette's calculated opaque contrast must be checked against actual rendered controls. Native screenshots, meaningful regression checks, runtime/data verification and storage measurements are recorded in the release handoff after execution. User visual acceptance remains Shane's decision.

The requested cleanup of specific duplicate outputs was blocked by automatic review (“blocked by policy”). Nothing was removed. A separate, already-generated 0.9.2 validation payload was hash-checked (96 files, 36 dependency DLLs matching installed 0.9.3, no personal files) and moved into this task's candidate folder for normal staging reuse. This avoids another full runtime copy while preserving the installed versions and ZIPs.
