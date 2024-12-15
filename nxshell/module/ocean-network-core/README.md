## OCEAN Network Core

OCEAN network library framework

## Dependencies
You can also check the "Depends" provided in the debian/control file.

### Build dependencies
You can also check the "Build-Depends" provided in the debian/control file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd ocean-network-core
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/ocean-network-dialog` after the installation is finished, and plugins will be placed into related module plugin directory.

## Usage

Execute `ocean-network-dialog -h` to get more details.

## Getting help

You can press `F1` to start [lingmo-manual](https://github.com/lingmoos/lingmo-manual) when you focus on OCEAN Control Center network module.

You may also find these channels useful if you encounter any other issues:

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=lingmo)
* [Official Forum](https://bbs.lingmo.org/)
* [Wiki](https://wiki.lingmo.org/)
* [Developer Center](https://github.com/lingmoos/ocean-network-core) 

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

OCEAN Network Core is licensed under [LGPL-3.0-or-later](LICENSE).
