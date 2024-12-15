# OCEAN Dock

OCEAN Dock is the dock of Lingmo Desktop Environment.

A tutorial for build ocean-dock plugin: [plugins-developer-guide](plugins/plugin-guide/plugins-developer-guide.md)

### Dependencies

You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies

You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd ocean-dock
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
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

ocean-dock is licensed under [LGPL-3.0-or-later](LICENSE).