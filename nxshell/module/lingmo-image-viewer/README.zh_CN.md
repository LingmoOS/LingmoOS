# Lingmo Image Viewer

看图是一款外观时尚、性能流畅的图片查看工具。

### 依赖

### 运行时依赖

* [lingmo-tool-kit](https://github.com/lingmoos/lingmo-tool-kit)
* [lingmo-shortcut-viewer](https://github.com/lingmoos/lingmo-shortcut-viewer)
* [lingmo-manual](https://github.com/lingmoos/lingmo-manual)
* libexif
* libraw
* Qt5 (>= 5.6)
  * Qt5-DBus
  * Qt5-Svg
  * Qt5-X11extras

## 安装

sudo apt install cmake qtbase5-dev pkg-config libexif-dev libqt5svg5-dev libqt5x11extras5-dev libsqlite3-dev qttools5-dev-tools qttools5-dev libxcb-util0-dev libstartup-notification0-dev libraw-dev libqt5opengl5-dev qtbase5-private-dev qtmultimedia5-dev x11proto-xext-dev libmtdev-dev libegl1-mesa-dev libudev-dev libfontconfig1-dev libfreetype6-dev libglib2.0-dev libxrender-dev libdtkwidget-dev libdtkwidget5-bin libdtkcore5-bin libimageeditor-dev

### 从源代码构建

1. 确保已经安装了所有的编译依赖.
```
$ sudo apt build-dep .
```

2. 构建:
```
$ cd lingmo-image-viewer
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. 安装:
```
$ sudo make install
```
## 使用

执行 `lingmo-image-viewer`

## 文档


安装完成后，可执行文件路径为`/usr/bin/lingmo-image-viewer`.

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Official Forum](https://bbs.lingmo.org/)
* [Wiki](https://wiki.lingmo.org/)

## 参与贡献

我们鼓励您报告问题并作出更改：

* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 协议

Lingmo Image Viewer 遵循协议 [GPL-3.0-or-later](LICENSE.txt).
