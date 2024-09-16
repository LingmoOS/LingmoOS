# 深度相册

深度相册是一款外观时尚、性能流畅的照片和视频管理工具，支持查看、管理多种图片和视频格式。您可以按日期时间线排列照片和视频，还可以将照片和视频添加到我的收藏，或者归类到不同的相册中，将照片和视频管理得井井有条。

### 依赖

### 编译依赖

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

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


## 安装

### 构建过程

1. 确保已安装所有依赖库.

_不同发行版的软件包名称可能不同，如果您的发行版提供了deepin-album，请检查发行版提供的打包脚本。_

如果你使用的是 [Deepin](https://distrowatch.com/table.php?distribution=deepin) 或者其它提供了deepin-album的发行版:

``` shell
$ apt build-dep deepin-album
```

2. 构建:

```
$ cd deepin-album
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. 安装:

```
$ sudo make install
```

可执行文件位于 `/usr/bin/deepin-album`

## 使用

命令行启动 `deepin-album`

## 文档

 - [用户文档](https://wikidev.uniontech.com/index.php?title=%E7%9B%B8%E5%86%8C)

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 开源许可证

deepin-album 在 [GPL-3.0-or-later](LICENSE.txt) 下发布。