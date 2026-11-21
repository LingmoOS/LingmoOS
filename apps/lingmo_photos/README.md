# Lingmo Photos

Lingmo Photos is a native C++20 photo library application for Lingmo OS.

The first foundation release separates the layers into:

- `PhotoStore`: SQLite persistence for indexed images, favorites and trash.
- `LibraryScanner`: asynchronous local filesystem discovery and indexing.
- SDK UI: `SidebarList`, `Grid`, `ScrollView`, `ImageView` and `AboutDialog`.
- `GlobalMenu`: D-Bus menu registration for the Aurora global status bar.

The scanner indexes supported image files under `~/Pictures` and renders them with the SDK thumbnail cache. The next milestones add EXIF extraction, folder and album models, the full viewer, non-destructive edit transactions and export jobs without moving decoding or indexing onto the UI thread.

## Languages

The application uses Lingmo's gettext integration. Chinese (`zh_CN`) is
included in `po/zh_CN.po`; add new catalogs beside it and run the
`update-translations` and `check-translations` CMake targets when extending
the UI.
