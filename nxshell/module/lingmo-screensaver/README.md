### lingmo-screensaver

Lingmo Screensaver is a screensaver service developed by Lingmo Technology.

### Dependencies

  * Qt
  * xscreensaver
  * xscreensaver-data
  * startocean
  * qml-module-qt-labs-platform
  *  libdframeworkdbus

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

 * pkg-config
 * qt5-qmake
 * qtbase5-dev
 * qtdeclarative5-dev
 * libqt5x11extras5-dev
 * libdtkwidget-dev
 * libx11-dev
 * libxss-dev
 * libxext-dev
 * qttools5-dev-tools
 * qml-module-qt-labs-platform
 *  libdframeworkdbus-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if lingmo-screensaver is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got lingmo-screensaver delivered:

``` shell
$ apt build-dep lingmo-screensaver
```

2. Build:
```
$ cd lingmo-screensaver
$ mkdir Build
$ cd Build
$ qmake ../lingmo-screensaver.pro
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `lingmo-screensaver` .

## Usage

Execute `lingmo-screensaver` 

## Documentations


## Getting help

 - [Official Forum](https://bbs.lingmo.org/)
 - [Developer Center](https://github.com/lingmoos/developer-center)
 - [Wiki](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License

lingmo-screensaver is licensed under [ GPL-3.0-or-later](LICENSE.txt)

