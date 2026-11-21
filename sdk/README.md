# Lingmo Toolkit

The UI toolkit of the Lingmo desktop: a Wayland client for
lingmo-aurora, drawing with Skia on GLES, with retained views, text
layout, controls and frame-paced animation.

## Dependencies

### Debian/Ubuntu

```shell
sudo apt install cmake ninja-build clang pkg-config python3 git \
    libwayland-dev wayland-protocols libegl-dev libgles-dev libxkbcommon-dev libsystemd-dev \
    libfontconfig-dev libfreetype-dev libharfbuzz-dev libicu-dev \
    libpng-dev libjpeg-dev zlib1g-dev libexpat1-dev \
    libwebp-dev libtiff-dev libheif-dev libjxl-dev
```

### Arch Linux

```shell
sudo pacman -S --needed base-devel cmake ninja clang pkgconf python git \
    wayland wayland-protocols libglvnd libxkbcommon systemd-libs fontconfig freetype2 harfbuzz icu \
    libpng libjpeg-turbo zlib expat libwebp libtiff libheif libjxl
```

## Build

Skia is fetched at a pinned revision and built as part of the project:

```shell
scripts/fetch-skia.sh
mkdir build
cd build
cmake ..
make
```

`examples/` has a demo window, `lingmo-toolkit-demo`, and
`lingmo-toolkit-gallery` with every control.

## License

GPL-3.0-or-later; see [LICENSE](LICENSE).
