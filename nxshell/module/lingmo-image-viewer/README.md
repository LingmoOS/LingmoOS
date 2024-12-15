# Lingmo Image Viewer

Lingmo Image Viewer is an image viewing tool with fashion interface and smooth performance  developed by Lingmo Technology.

### Dependencies

### Runtime dependencies

* [lingmo-tool-kit](https://github.com/lingmoos/lingmo-tool-kit)
* [lingmo-shortcut-viewer](https://github.com/lingmoos/lingmo-shortcut-viewer)
* [lingmo-manual](https://github.com/lingmoos/lingmo-manual)
* libexif
* libraw
* Qt5 (>= 5.6)
  * Qt5-DBus
  * Qt5-Svg
  * Qt5-X11extras

## Installation

sudo apt install cmake qtbase5-dev pkg-config libexif-dev libqt5svg5-dev libqt5x11extras5-dev libsqlite3-dev qttools5-dev-tools qttools5-dev libxcb-util0-dev libstartup-notification0-dev libraw-dev libqt5opengl5-dev qtbase5-private-dev qtmultimedia5-dev x11proto-xext-dev libmtdev-dev libegl1-mesa-dev libudev-dev libfontconfig1-dev libfreetype6-dev libglib2.0-dev libxrender-dev libdtkwidget-dev libdtkwidget5-bin libdtkcore5-bin libimageeditor-dev

### Build from source code

1. Make sure you have installed all dependencies.
```
$ sudo apt build-dep .
```

2. Build:
```
$ cd lingmo-image-viewer
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

Execute `lingmo-image-viewer`

## Documentations

When install complete, the executable binary file is placed into `/usr/bin/lingmo-image-viewer`.

## Getting help

Any usage issues can ask for help via

* [Official Forum](https://bbs.lingmo.org/)
* [Wiki](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

Lingmo Image Viewer is licensed under [GPL-3.0-or-later](LICENSE.txt).
