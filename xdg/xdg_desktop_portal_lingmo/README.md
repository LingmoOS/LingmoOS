# xdg-desktop-portal-lingmo

The `xdg-desktop-portal` backend for Lingmo: Settings, FileChooser, Inhibit
and ScreenCast.

## Dependencies

It is built on the base library of
[lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit); install it
first.

```sh
sudo apt install cmake ninja-build pkg-config libsystemd-dev libpipewire-0.3-dev \
    libwayland-dev wayland-protocols libgbm-dev libdrm-dev
```

At runtime it needs `xdg-desktop-portal`, `xdg-desktop-portal-gtk`, PipeWire,
lingmo-services and the File Manager daemon (file dialogs). Tests also need
`dbus-run-session` and GoogleTest (`libgtest-dev`).

## Build

```sh
cmake -S . -B build -G Ninja -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
sudo cmake --install build
```

## Tests

```sh
ctest --test-dir build --output-on-failure
```

## License

GPL-3.0-or-later; see [LICENSE](LICENSE).
