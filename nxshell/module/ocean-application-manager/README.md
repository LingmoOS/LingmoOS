# OCEAN Application Manager

## WORK IN PROGRESS

**This branch is used to totally refactor.**

[Refactor plan](./docs/TODO.md).

---

OCEAN Application Manager is the app manager of Lingmo Desktop Environment.

### Dependencies

You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies

You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```shell
$ cd ocean-application-manager
$ cmake -B build
$ cmake --build build -j`nproc`
```
3. Install

```
sudo cmake --install build
```

## Getting help

* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [WiKi](https://wiki.lingmo.org)
* [Forum](https://bbs.lingmo.org)
* [Developer Center](https://github.com/lingmoos/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

ocean-application-manager is licensed under [GPL-3.0-or-later](LICENSE).
