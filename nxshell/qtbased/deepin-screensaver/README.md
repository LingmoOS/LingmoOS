### deepin-screensaver

Deepin Screensaver is a screensaver service developed by Deepin Technology.

### Dependencies

  * Qt
  * xscreensaver
  * xscreensaver-data
  * startdde
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

_Package name may be different between distros, if deepin-screensaver is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got deepin-screensaver delivered:

``` shell
$ apt build-dep deepin-screensaver
```

2. Build:
```
$ cd deepin-screensaver
$ mkdir Build
$ cd Build
$ qmake ../deepin-screensaver.pro
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `deepin-screensaver` .

## Usage

Execute `deepin-screensaver` 

## Documentations


## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License

deepin-screensaver is licensed under [ GPL-3.0-or-later](LICENSE.txt)

