# Lingmo Photos

Lingmo Photos is a native C++20 photo library application for Lingmo OS.

The app separates the layers into:

- `PhotoStore`: SQLite persistence for indexed images, favorites, trash and albums.
- `LibraryScanner`: asynchronous local filesystem discovery and indexing.
- SDK UI: `SidebarList`, `Grid`, `ScrollView`, `ImageView` and `AboutDialog`.
- `GlobalMenu`: D-Bus menu registration for the Aurora global status bar.

The scanner indexes supported image files under `~/Pictures` and renders them
with the SDK thumbnail cache. Browse the timeline, favorites, trash, folders
and user albums; right-click a photo to add it to an album, or remove it from
the album currently open. Albums can be created, renamed and deleted. The
viewer opens image files launched from the desktop, hides library navigation
while viewing, and keeps navigation, zoom, slideshow and edit actions in the
top toolbar. Edits are exported as a separate copy. EXIF extraction remains a
future milestone; image decoding and indexing stay off the UI thread.

## Languages

The application uses Lingmo's gettext integration. Chinese (`zh_CN`) is
included in `po/zh_CN.po`; add new catalogs beside it and run the
`update-translations` and `check-translations` CMake targets when extending
the UI.
