# Deepin Image Viewer

Deepin Image Viewer is an image viewing tool with fashion interface and smooth performance  developed by Deepin Technology.

### Dependencies

### Runtime dependencies

* [deepin-tool-kit](https://github.com/linuxdeepin/deepin-tool-kit)
* [deepin-shortcut-viewer](https://github.com/linuxdeepin/deepin-shortcut-viewer)
* [deepin-manual](https://github.com/linuxdeepin/deepin-manual)
* libexif
* freeimage
* libraw
* Qt5 (>= 5.6)
  * Qt5-DBus
  * Qt5-Svg
  * Qt5-X11extras

## Installation

sudo apt install cmake qtbase5-dev pkg-config libexif-dev libqt5svg5-dev libqt5x11extras5-dev libsqlite3-dev qttools5-dev-tools qttools5-dev libxcb-util0-dev libstartup-notification0-dev libraw-dev libfreeimage-dev libqt5opengl5-dev qtbase5-private-dev qtmultimedia5-dev x11proto-xext-dev libmtdev-dev libegl1-mesa-dev libudev-dev libfontconfig1-dev libfreetype6-dev libglib2.0-dev libxrender-dev libdtkwidget-dev libdtkwidget5-bin libdtkcore5-bin libimageeditor-dev

### Build from source code

1. Make sure you have installed all dependencies.
```
$ sudo apt build-dep .
```

2. Build:
```
$ cd deepin-image-viewer
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```
## Usage

Execute `deepin-image-viewer`

## Documentations

When install complete, the executable binary file is placed into `/usr/bin/deepin-image-viewer`.

## Getting help

Any usage issues can ask for help via

* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

Deepin Image Viewer is licensed under [GPL-3.0-or-later](LICENSE.txt).
