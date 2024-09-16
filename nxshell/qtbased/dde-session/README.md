## dde-session
dde-session is used for launching DDE components systemd service project.
This project refers to a part of GNOME session documents and files.

## Dependencies
You can also check the "Depends" provided in the debian/control file.

### Build dependencies
You can also check the "Build-Depends" provided in the debian/control file.
### Runtime dependencies

- [startdde](https://github.com/linuxdeepin/startdde)
- systemd

## Installation

### Deepin

Install prerequisites

```shell
sudo apt-get build-dep dde-session
```

Build

```shell
mkdir build && cd build && cmake ../ -DCMAKE_INSTALL_PREFIX=/usr && make
```

If you have isolated testing build environment (say a docker container), you can install it directly.

```shell
sudo make install
```

## Usage

### Run with display manager

1. construct a session desktop in /usr/share/xsessions

```shell
cat /usr/share/xsessions/deepin.desktop
```

```text
[Desktop Entry]
Name=Deepin
Comment=Deepin Desktop Environment
Exec=/usr/bin/dde-session
```

2. Using DisplayManager like, gdm, kdm or lightdm to startup Deepin.

## Getting help

Any usage issues can ask for help via

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues)

## Getting involved

We encourage you to report issues and contribute changes

- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

dde-session is licensed under [GPLv3](LICENSE).
