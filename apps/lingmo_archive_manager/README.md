# Lingmo Archive Manager

A C++20 Linux archive manager built with Lingmo SDK and libarchive.

## Features

- Browse ZIP, TAR and common compressed TAR variants, 7z and RAR as supported by the installed libarchive. Single gzip/xz/bzip2/zstd streams are shown as one file.
- Navigate folders, search within the current subtree, select multiple entries, and extract all or selected subtrees.
- Create ZIP, TAR, TAR+gzip and TAR+xz archives from files and folders.
- Check archive payloads by reading every entry; available checksums depend on the archive format. This is not cryptographic authenticity verification.
- Run reads, compression, extraction and checking in background workers with progress and cancellation.
- Accept local files via command line, desktop MIME associations, file dialogs or drag-and-drop.
- Enter a session-only archive password. Decryption support depends on libarchive; unsupported encrypted formats fail explicitly. Encrypted archive creation is not included.

## Design

Follows [the project application design standard](../docs/application-design.md): left glass sidebar, opaque right content, centered title, global menus first. The compact title-bar menu remains until the global registrar acknowledges registration and returns if the service disappears. Native SDK controls are used throughout; lists are virtualized. The sidebar highlights the active workspace and groups archive metadata separately. Extract All is the primary content-header action; selected extraction appears when files are selected. Password input expands on demand or when encryption requires it. File and folder glyphs, aligned columns and a separate status footer keep the browser scannable.

The left sidebar is true compositor glass when available. Headless screenshots show the solid fallback, not actual desktop blur. The global menu exporter is adapted from Calculator, preserving the original attribution from File Manager. No changes to SDK, Framework, Core or the compositor are needed for this first version.

## Usage

~~~sh
lingmo-archive-manager documents.zip
lingmo-archive-manager --create document.pdf pictures/
lingmo-archive-manager -- --name-starting-with-dash.zip
~~~

One archive opens in the browser. Multiple arguments or a folder enter the creation workspace. The existing application instance receives subsequent requests; new requests are refused while an operation is running.

Click a row to toggle its selection. Double-click a folder or press Enter on its focused row to enter it. Space toggles the focused row. Search is an ASCII case-insensitive substring match within the current subtree; other UTF-8 characters match exactly. Entries are not executed or opened directly from inside an archive.

In the creation workspace, choose files/folders and an output format, then Create. Existing archives are never overwritten. Adding/removing source rows changes the new-archive selection only; editing an existing archive in place is not implemented.

Keyboard shortcuts: Ctrl+O open, Ctrl+N new, Ctrl+E extract all, Ctrl+T check integrity, Ctrl+A select all, Ctrl+F search, Ctrl+Q quit, Escape cancel/clear selection, Backspace parent folder outside text fields. File dialogs require a working desktop FileChooser portal.

## File handling

Extraction stages into a new private directory inside the chosen destination and commits it only after success. A new name is allocated if the output folder already exists. Cancellation or failure removes the staged output. Existing files are not merged or overwritten. Extracted files use owner-only permissions, retaining the executable bit where applicable; ownership and special permission bits are not restored.

Absolute paths, parent traversal, backslashes/drive-style paths, symbolic links, hard links and special file entries are refused for extraction. Unsafe entries can be listed as Restricted. Path components are opened relative to directory descriptors using no-follow flags. Creating an archive rejects links/special source files and a destination nested inside a selected source directory. The source files are never removed.

Default limits: 100,000 headers and 20 GiB uncompressed payload per operation. Actual decoded bytes are bounded while reading and extracting. Selected extraction still validates the selected entries. Multipart archives, encrypted archive creation, in-place archive edits and automatic execution are outside this version's scope.

## Build and test

Dependencies: CMake >= 3.22, a C++20 compiler, pkg-config, libarchive-dev, LingmoSDK development files, gettext and dbus-run-session for the menu test. xdg-utils is used to show the completed output directory.

~~~sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build -j2
ctest --test-dir build --output-on-failure
sudo cmake --install build
~~~

For a custom SDK prefix, pass -DCMAKE_PREFIX_PATH=/path/to/prefix. Core-only tests can use -DARCHIVE_MANAGER_BUILD_UI=OFF. The local build-deps/verify.sh helper reuses this workspace's previously extracted Calculator SDK package; it is ignored build tooling, not a required application dependency.

Generate light/dark, 100%/150% headless captures:

~~~sh
build/tests/archive-manager-capture captures
~~~

Tests cover real archive creation/extraction, Unicode paths, selection, traversal/link rejection, overwrite protection, limits, cancellation, SDK interactions, glass alpha boundaries, and global menu registration/reconnection. Actual portal interaction and focused global-menu display still require a live Lingmo session.

## Validation (2026-11-06)

The WSL Debian build passed all five CTest groups: translation catalogs, archive core, SDK UI, global-menu protocol and full window/global-menu integration. Core tests passed ZIP/TAR/TAR.GZ/TAR.XZ roundtrips, Unicode paths, selected extraction, encrypted ZIP password handling, duplicate/hardlink/traversal rejection, truncation, limits and mid-operation cancellation cleanup. SDK tests exercised background create/open/extract/verify and the left-glass/right-solid alpha boundary. The window/global-menu integration test confirms fallback hiding after registration, fallback restoration when the registrar disappears, re-registration, page preservation, and triggering New Archive from the exported menu. Light/dark captures were inspected. A staged install verified the executable, desktop file (including unique keys), icon and Chinese catalog.

RAR/7z compatibility depends on libarchive and was not covered by a fixture in this run. Real Lingmo global status bar display, portal dialogs, drag-and-drop between applications and desktop glass blur still need live-session acceptance testing. No system installation or publishing was performed.

GPL-3.0-or-later. The bundled application icon is reused from the workspace's Lingmo icon theme; see debian/copyright.
