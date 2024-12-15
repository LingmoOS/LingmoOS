### ocean-grand-search

Lingmo Grand Search is a basic search tool developed by Lingmo Technology,featured with searching including a series of files,applications or documents, etc.

### Dependencies

  * Qt
  * DTK
  * libdframeworkdbus2
  * libgsettings-qt1
  * libicu63
  * libpoppler-qt5-1 
  * libqt5concurrent5 
  * libtag1v5
  * lingmo-desktop-schemas
  * ocean-daemon
  * startocean
  * lingmo-anything-server

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
 * ocean-dock-dev
 * libglib2.0-dev
 * libpoppler-qt5-dev
 * libtag1-dev
 * libicu-dev
 * libgsettings-qt-dev
 * libpcre3-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if ocean-grand-search is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got ocean-grand-search delivered:

``` shell
$ apt build-dep ocean-grand-search
```

2. Build:
```
$ cd ocean-grand-search
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/ocean-grand-search` and `/usr/bin/ocean-grand-search-daemon` 

## Usage

Execute `ocean-grand-search-daemon` and `ocean-grand-search`

## Documentations


## Getting help

 - [Official Forum](https://bbs.lingmo.org/)
 - [Developer Center](https://github.com/lingmoos/developer-center)
 - [Wiki](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License

ocean-grand-search is licensed under [ GPL-3.0-only](LICENSE.txt)

