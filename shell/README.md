# Lingmo Aurora

<p align="center">
  <img src="aurora.png" alt="Aurora logo" width="180">
</p>

The Lingmo Aurora Wayland compositor, with window management, the desktop shell,
the lock screen and Xwayland in one process. Much of it is ported from
Chromium's Exo and Ash.

## Dependencies

lingmo-sdk, the lingmo-framework clients and libwayland 1.23 or newer.
At runtime it needs `adwaita-icon-theme` and `xwayland`. Tests also need
`libgtest-dev`.

## Build

```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

Pass `-DBUILD_TESTING=ON` to build the tests.

## License

GPL-3.0-or-later; see [LICENSE](LICENSE). Code ported from Chromium keeps its
original notice in [LICENSE.chromium](LICENSE.chromium), and code from
sommelier in [LICENSE.sommelier](LICENSE.sommelier).
