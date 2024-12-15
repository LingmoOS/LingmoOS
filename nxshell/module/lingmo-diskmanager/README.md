# diskManager
Disk Utility is a useful tool to help you better manage your disks through disk partitioning, disk data administration, and health management.

## Dependencies
* smartmontools
* exfat-fuse
* Qt5 (>= 5.3)
* Qt5-X11extras

### Build dependencies

* cmake
* qt5
* dtk


## Installation
sudo apt-get install lingmo-diskmanager

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd lingmo-diskmanager
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/lingmo-diskmanager`.

## Getting help

Any usage issues can ask for help via
* [WiKi](https://wiki.lingmo.org)
* [Forum](https://bbs.lingmo.org)
* [Developer Center](https://github.com/lingmoos/developer-center)

## Getting involved

We encourage you to report issues and contribute changes

* [**Contribution guide for developers**](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en)


## License
This project is licensed under  GPL-3.0-only.
