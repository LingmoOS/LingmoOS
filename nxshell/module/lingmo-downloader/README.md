# Download Manager

Download Manager is Lingmo Desktop Environment download manager.

## Dependencies

### Build dependencies

* cmake
* qt5
* aria2


### Runtime dependencies

* Qt5 (>= 5.3)
* Qt5-X11extras


## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd lingmo-downloader
$ mkdir Build
$ cd Build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/lingmo-downloadmanager`.

## Getting help

Any usage issues can ask for help via
* [WiKi](https://wiki.lingmo.org)
* [Forum](https://bbs.lingmo.org)
* [Developer Center](https://github.com/lingmoos/developer-center/issues)
* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)


## License
This project is licensed under GPL-3.0-or-later.