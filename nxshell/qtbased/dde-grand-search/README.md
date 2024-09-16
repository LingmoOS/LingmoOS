### dde-grand-search

Deepin Grand Search is a basic search tool developed by Deepin Technology,featured with searching including a series of files,applications or documents, etc.

### Dependencies

  * Qt
  * DTK
  * libdframeworkdbus2
  * libgsettings-qt1
  * libicu63
  * libpoppler-qt5-1 
  * libqt5concurrent5 
  * libtag1v5
  * deepin-desktop-schemas
  * dde-daemon
  * startdde
  * deepin-anything-server

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

 * pkg-config
 * qt5-qmake
 * qtbase5-dev
 * libdtkwidget-dev
 * libdtkcore-dev
 * libdtkgui-dev
 * libdtkcore5-bin
 * libdframeworkdbus-dev
 * qttools5-dev-tools
 * cmake
 * dde-dock-dev
 * libglib2.0-dev
 * libpoppler-qt5-dev
 * libtag1-dev
 * libicu-dev
 * libgsettings-qt-dev
 * libpcre3-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if dde-grand-search is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got dde-grand-search delivered:

``` shell
$ apt build-dep dde-grand-search
```

2. Build:
```
$ cd dde-grand-search
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/dde-grand-search` and `/usr/bin/dde-grand-search-daemon` 

## Usage

Execute `dde-grand-search-daemon` and `dde-grand-search`

## Documentations


## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License

dde-grand-search is licensed under [ GPL-3.0-only](LICENSE.txt)

