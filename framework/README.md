# Lingmo Framework

Shared system APIs for Lingmo components and applications, on the
[lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit) base
library:

- `clients/`: clients of system services (accounts, appearance, audio,
  BlueZ, display, NetworkManager, power profiles, time and date, UPower), one
  library each, in the `LingmoClients` package.
- `files/`: file listing, MIME types, operations, trash, volumes and
  thumbnails, in the `LingmoFiles` package.

## Dependencies

### Debian/Ubuntu

```shell
sudo apt install cmake pkg-config build-essential \
    libicu-dev libpng-dev \
    libpipewire-0.3-dev libspa-0.2-dev libgtest-dev
```

### Arch Linux

```shell
sudo pacman -S --needed base-devel cmake pkgconf \
    icu libpng pipewire gtest
```

and [lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit).

## Build and Install

```shell
mkdir build
cd build
cmake ..
make
sudo make install
```

## License

BSD-3-Clause; see [LICENSE](LICENSE).
