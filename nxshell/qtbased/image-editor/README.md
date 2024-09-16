# image-editor

Image editor is a public library for deepin-image-viewer and deepin-album  developed by Deepin Technology.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* Qt (>=5.11)
* debhelper (>= 11),
* cmake,
* qtbase5-dev,
* pkg-config,
* libexif-dev,
* libqt5svg5-dev,
* libqt5x11extras5-dev,
* libsqlite3-dev,
* qttools5-dev-tools,
* qttools5-dev,
* libxcb-util0-dev,
* libstartup-notification0-dev,
* libraw-dev,
* libfreeimage-dev,
* libqt5opengl5-dev,
* qtbase5-private-dev,
* qtmultimedia5-dev,
* x11proto-xext-dev,
* libmtdev-dev,
* libegl1-mesa-dev,
* libudev-dev,
* libfontconfig1-dev,
* libfreetype6-dev,
* libglib2.0-dev,
* libxrender-dev,
* libdtkwidget-dev,
* libdtkwidget5-bin,
* libdtkcore5-bin,
* libgio-qt-dev,
* libudisks2-qt5-dev,
* libopencv-dev,
* libmediainfo-dev,
* libgstreamer1.0-dev,
* libtiff-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if image-editor is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got image-editor delivered:

``` shell
$ sudo apt-get build-dep image-editor
```
or
``` shell
$ sudo apt install cmake qtbase5-dev pkg-config libexif-dev libqt5svg5-dev libqt5x11extras5-dev libsqlite3-dev qttools5-dev-tools qttools5-dev libxcb-util0-dev libstartup-notification0-dev libraw-dev libfreeimage-dev libqt5opengl5-dev qtbase5-private-dev qtmultimedia5-dev x11proto-xext-dev libmtdev-dev libegl1-mesa-dev libudev-dev libfontconfig1-dev libfreetype6-dev libglib2.0-dev libxrender-dev libdtkwidget-dev libdtkwidget5-bin libdtkcore5-bin libgio-qt-dev libudisks2-qt5-dev libmediainfo-dev libopencv-dev libgstreamer1.0-dev
```

2. Build:

```
$ cd draw
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. Install:

```
$ sudo make install
```

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). 

## License
Image editor is licensed under [GPL-3.0-or-later](LICENSE.txt)