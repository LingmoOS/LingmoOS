# Lingmo Photos

Lingmo Photos is a native C++20 photo library application for Lingmo OS.

The first foundation release separates the layers into:

- `PhotoStore`: SQLite persistence for indexed images, favorites and trash.
- `LibraryScanner`: asynchronous local filesystem discovery and indexing.
- SDK UI: `SidebarList`, `Grid`, `ScrollView`, `ImageView` and `AboutDialog`.
- `GlobalMenu`: D-Bus menu registration for the Aurora global status bar.

The scanner indexes supported image files under `~/Pictures` and renders them with the SDK thumbnail cache. The next milestones add EXIF extraction, folder and album models, the full viewer, non-destructive edit transactions and export jobs without moving decoding or indexing onto the UI thread.
