# DDE Application Manager

## WORK IN PROGRESS

**This branch is used to totally refactor.**

[Refactor plan](./docs/TODO.md).

---

DDE Application Manager is the app manager of Deepin Desktop Environment.

### Dependencies

You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies

You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```shell
$ cd dde-application-manager
$ cmake -B build
$ cmake --build build -j`nproc`
```
3. Install

```
sudo cmake --install build
```

## Getting help

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

dde-application-manager is licensed under [GPL-3.0-or-later](LICENSE).
