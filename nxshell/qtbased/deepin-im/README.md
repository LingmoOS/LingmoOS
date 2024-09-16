## deepin-im
deepin-im is input method of deepin desktop environment.

## Dependencies
You can also check the "Depends" provided in the debian/control file.

### Build dependencies
You can also check the "Build-Depends" provided in the debian/control file.

## Installation

### Deepin

Install prerequisites

```shell
sudo apt-get build-dep deepin-im
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
run directly`/usr/bin/dim`.

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

deepin-im is licensed under [GPLv3](LICENSE).
