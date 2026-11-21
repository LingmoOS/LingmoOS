# Core

The Lingmo session, system services and their helpers.

## Dependencies

lingmo-toolkit, the display client of lingmo-framework, libsystemd, ICU,
Fontconfig, Wayland development libraries and wayland-protocols (>= 1.39).
The screen recorder also needs FFmpeg, libva, libdrm, GBM and PipeWire; the
tests need GoogleTest.

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
```

## Install

```shell
sudo make install
```

Install `lingmo-aurora` and `lingmo-filemanager` as well, then select
`Lingmo (Wayland)` in the display manager.

## License

GPL-3.0-or-later; see [LICENSE](LICENSE).
