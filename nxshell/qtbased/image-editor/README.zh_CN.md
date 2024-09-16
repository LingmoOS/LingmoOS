# image-editor

Image editor 是深度开发的用于  deepin-image-viewer 和 deepin-album 的图像处理公共库。

### 依赖

### 编译依赖

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

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

## 安装

### 构建过程

1. 确保已安装所有依赖库.

_不同发行版的软件包名称可能不同，如果您的发行版提供了image-editor，请检查发行版提供的打包脚本。_

如果你使用的是 [Deepin](https://distrowatch.com/table.php?distribution=deepin) 或者其它提供了image-editor的发行版:

``` shell
$ sudo apt-get build-dep image-editor
```
或者手动安装
``` shell
$ sudo apt install cmake qtbase5-dev pkg-config libexif-dev libqt5svg5-dev libqt5x11extras5-dev libsqlite3-dev qttools5-dev-tools qttools5-dev libxcb-util0-dev libstartup-notification0-dev libraw-dev libfreeimage-dev libqt5opengl5-dev qtbase5-private-dev qtmultimedia5-dev x11proto-xext-dev libmtdev-dev libegl1-mesa-dev libudev-dev libfontconfig1-dev libfreetype6-dev libglib2.0-dev libxrender-dev libdtkwidget-dev libdtkwidget5-bin libdtkcore5-bin libgio-qt-dev libudisks2-qt5-dev libmediainfo-dev libopencv-dev libgstreamer1.0-dev
```

2. 构建:

```
$ cd draw
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. 安装:

```
$ sudo make install
```

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). 

## 开源许可证
Image editor 在 [GPL-3.0-or-later](LICENSE.txt) 下发布。