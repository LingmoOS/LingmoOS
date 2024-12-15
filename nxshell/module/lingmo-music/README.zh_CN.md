### lingmo-music

Lingmo music  is a local music player with beautiful design and simple functions  developed by Lingmo Technology.

### 依赖

### 构建依赖

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* pkg-config
* libtag1-dev
* libicu-dev
* libgsettings-qt-dev
* libdbusextended-qt5-dev
* libkf5codecs-dev
* libavutil-dev
* libavcodec-dev
* libavformat-dev
* libdtkcore-dev
* libdtkgui-dev
* libudisks2-qt5-dev 
* libvlc-dev
* libvlccore-dev
* Qt5(>= 5.6) with modules:
  - qt5-qmake
  - libqt5svg5-dev
  - qttools5-dev-tools
  - qtmultimedia5-dev
  - libkf5codecs-dev
* Lingmo-tool-kit(>=2.0) with modules:
  - libdtkwidget-dev

## 安装

### 构建源码

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if lingmo-music is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got lingmo-music delivered:

``` shell
$ apt build-dep lingmo-music
```

2. Build:

```
$ cd lingmo-music
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:

```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/lingmo-music`

## 用途

Execute `lingmo-music`

## 帮助

 - [Official Forum](https://bbs.lingmo.org/)
 - [Developer Center](https://github.com/lingmoos/developer-center)
 - [Gitter](https://gitter.im/orgs/lingmoos/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=lingmo)
 - [Wiki](https://wiki.lingmo.org/)

## 参与贡献

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
 - [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 协议

lingmo-music 根据 [GPL-3.0-or-later]（许可证）获得许可.
