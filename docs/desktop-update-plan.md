# Desktop usability and updates

Approved direction: Shane asked on October 4, 2026 to build the previously discussed splash, clear-media controls, modest UI improvements, and GitHub updates. Raspberry Pi work is deferred.

## User experience

- Branded, centred, dismissible startup splash showing the actual version. Skip it for diagnostics.
- Clear media in the Media panel and surface/list context menus. Return to white grid, preserve geometry, support Undo, do not remove original files.
- Small usability additions: surface rename, useful content labels, disabled unavailable playback controls, unsaved indicator, remembered project folder, Help menu with version, quick guide and update commands.
- Manual Check for updates reads public GitHub releases, offers stable Windows x64 updates, downloads with progress/cancellation, verifies the asset digest, prepares the new version, then saves/cancels and restarts. Active output must be stopped before applying an update.

## Installation contract

Windows x64 first. Root PotatoMapper.exe is a small C++ launcher with no Qt dependency. installation.txt identifies layout protocol 1. current.txt names the selected numeric version; previous.txt retains the prior version. Each versions/<version> directory contains PotatoMapperApp.exe and its complete Qt/FFmpeg runtime. Projects and media remain outside update-managed directories; the application uses the installation root as its initial folder. Existing projects can stay at their existing absolute locations.

Release ZIPs contain a versioned runtime and update-manifest.json with version/protocol and file hashes. GitHub asset SHA256 is checked before extraction. Extraction rejects traversal, links and excessive entries/sizes. A native helper waits for the editor to exit, copies to a temporary version directory, renames it into place, and atomically changes the current pointer. Failed preparation leaves the selected version unchanged. The helper holds an install lock; the editor holds a runtime lock to avoid simultaneous app instances in one installation. Keep the previous version and provide a manual rollback command; rollback does not reverse edits to project files.

The launcher protocol is deliberately small and stable. Releases requiring a different launcher protocol instruct the user to download manually rather than silently install an incompatible runtime. The first transition from legacy 0.2.0 is manual: close the old app and copy the new package contents into its existing folder, keeping personal files. No automatic deletion of legacy libraries or user files.

## Implementation order and checks

1. Shared clear/rename operations, context menus, splash and Help controls. Check clear/undo/redo/project round-trip with geometry and another surface preserved; inspect real UI and splash.
2. Launcher and update controller, versioned packaging. Check version/asset selection, malformed/incomplete payload rejection and pointer/file preservation, cancellation and failed extraction.
3. Run packaged mapping/dots/colour checks and MP4 playback. Exercise a packaged upgrade in a disposable installation with sample project/media sentinels and rollback. Check the real GitHub endpoint and update dialog. Preserve the existing release and user's mapping.
4. Review changes, update roadmap/docs and publish the new Windows release. Record the limits of local testing; a subsequent update on the user's second computer remains a separate real-world check.

No broad UI rewrite, new visual generators, cloud storage, or Raspberry Pi implementation in this change.
