# File Manager

Lingmo File Manager, simple to use, beautiful, and retain the classic PC interactive design.

## Dependencies

File Manager is built on [lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit) and
the files library of [lingmo-framework](https://github.com/lingmoos/lingmo-framework);
install them first. The mount service needs FUSE, libssh, libsmbclient, libmtp and
libudev, and zlib, bzip2, liblzma and LZFSE for disk images; archives need libarchive
and libzip. GoogleTest for the tests.

Debian/Ubuntu:

```shell
sudo apt install build-essential cmake pkg-config libicu-dev libarchive-dev libzip-dev \
    libcurl4-openssl-dev libfuse3-dev libssh-dev libsmbclient-dev libmtp-dev libudev-dev \
    zlib1g-dev libbz2-dev liblzma-dev liblzfse-dev libgtest-dev
```

## Build

```shell
cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

## Desktop widgets

The desktop holds clock, calendar, system, battery, now-playing, notes and
weather widgets beside its icons. Right-click an empty spot and choose **Edit
Widgets…** to open the gallery, then pick a card. Drag a card to move it (it
snaps to the grid and stays in the work area), or right-click it to remove it
or change its size. Dragging a card brings it above the others, and that order
is saved. The layout is saved per output in `filemanager.conf`; Settings →
Desktop Widgets opens the same gallery through
`org.lingmo.FileManager.EditWidgets`.

Each card gets a stable instance id in that layout. A notes card keeps its text
in `~/.local/share/lingmo/filemanager/widget-notes`, one file per id, so two
notes stay separate. The weather card reads its city from Weather/City in
`filemanager.conf` (set it on the same Settings page) and asks Open-Meteo for
the forecast; the now-playing card follows MPRIS players and shows their
`mpris:artUrl` cover when it is a local image.

The cards ask the compositor for a backdrop blur behind their silhouettes
(`lingmo_surface_blur_v1`) and fall back to an opaque body when the compositor
does not have it.

## Tests

```shell
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## License

BSD-3-Clause; see [LICENSE](LICENSE).

## Desktop widgets

The searchable widget centre, desktop and Notification Center layouts, appearance settings and verification steps are described in [Desktop widgets](docs/desktop-widgets.md).
