# Deepin Album

Deepin  Album is a photo management tool with fashionable appearance and smooth performance, which supports viewing and managing multiple image formats. You can arrange your photos by date timeline, add them to your collection, or categorize them into different albums to keep them in order.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* pkg-config
* libexif-dev
* libxcb-util0-dev
* libstartup-notification0-dev
* libraw-dev
* libfreeimage-dev
* x11proto-xext-dev
* libmtdev-dev
* libegl1-mesa-dev
* libudev-dev
* libfontconfig1-dev
* libfreetype6-dev
* libglib2.0-dev
* libxrender-dev
* libdtkwidget-dev
* libdtkwidget5-bin
* libdtkcore5-bin
* Qt5(>=5.6) with modules:
  * qt5-default
  * libgio-qt-dev
  * libudisks2-qt5-dev
  * deepin-gettext-tools
  * qt5-qmake
  * qtbase5-dev
  * libqt5svg5-dev
  * libqt5x11extras5-dev
  * qttools5-dev-tools
  * libqt5opengl5-dev
  * qtbase5-private-dev
  * qtmultimedia5-dev



## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if deepin-album is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got deepin-album delivered:

``` shell
$ apt build-dep deepin-album
```

2. Build:

```
$ cd deepin-album
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:

```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/deepin-album`

## Usage

Execute `deepin-album`

## Documentations

 - [User Documentation](https://wikidev.uniontech.com/index.php?title=%E7%9B%B8%E5%86%8C)

## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en)
 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

deepin-album is licensed under [GPL-3.0-or-later](LICENSE.txt)