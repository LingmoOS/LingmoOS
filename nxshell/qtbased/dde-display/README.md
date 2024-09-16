## DDE Display

DDE Display is the dde screen management and display service for the dde desktop environment.

## Dependencies
You can also check the "Depends" provided in the debian/control file.

### Build dependencies
You can also check the "Build-Depends" provided in the debian/control file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd dde-display
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/dde-display` after the installation is finished.

## Usage

Execute `dde-display-console -h` to get more details.

## Getting help

You can press `F1` to start [deepin-manual](https://github.com/linuxdeepin/deepin-manual) when you focus on DDE Display window.

You may also find these channels useful if you encounter any other issues:

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

DDE Display is licensed under [LGPL-3.0-or-later](LICENSE).
