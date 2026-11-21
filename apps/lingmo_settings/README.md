# Settings

The System Settings application for Lingmo Desktop.

## Dependencies

Settings is built on [lingmo-toolkit](https://github.com/lingmoos/lingmo-toolkit)
and the clients of [lingmo-framework](https://github.com/lingmoos/lingmo-framework);
install those first.

Arch/Manjaro Dependencies:
```shell
sudo pacman -S cmake pkgconf icu libxcrypt fontconfig libxcursor libxkbcommon expat systemd-libs
```

Debian/Ubuntu Dependencies:
```shell
sudo apt install cmake pkg-config libicu-dev libcrypt-dev libfontconfig-dev libxcursor-dev libxkbcommon-dev libexpat1-dev libsystemd-dev
```

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## License

BSD-3-Clause; see [LICENSE](LICENSE).
