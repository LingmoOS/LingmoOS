### Deepin movie

影院是深度技术开发的全功能视频播放器，支持以多种视频格式播放本地和流媒体。

### 依赖

### 编译依赖

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

- debhelper

* cmake
* pkg-config
* libdtkcore5-bin
* libdtkwidget-dev
* libmpv-dev
* libxcb1-dev
* libxcb-util0-dev
* libxcb-shape0-dev
* libxcb-ewmh-dev
* xcb-proto
* x11proto-record-dev
* libxtst-dev
* libavcodec-dev
* libavformat-dev
* libavutil-dev
* libpulse-dev
* libssl-dev
* libdvdnav-dev
* libgsettings-qt-dev
* ffmpeg module(s):
  - libffmpegthumbnailer-dev
* Qt5(>= 5.6) with modules:
  - qtbase5-dev
  - qtbase5-private-dev
  - libqt5x11extras5-dev
  - qt5-qmake
  - libqt5svg5-dev
  - qttools5-dev
  - qttools5-dev-tools
  - libqt5sql5-sqlite
  - qtmultimedia5-dev

## 安装 

### 构建过程

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if deepin-movie is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got deepin-movie delivered:

``` 
$ apt build-dep deepin-movie
```

2. Build:

```
$ cd deepin-movie-reborn
$ mkdir Build
$ cd Build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/deepin-movie`.

## 用途

Execute `deepin-movie`

## 文档

 - [Development Documentation](https://linuxdeepin.github.io/deepin-movie/)
 - [User Documentation](https://wikidev.uniontech.com/index.php?title=%E5%BD%B1%E9%99%A2) | [用户文档](https://wikidev.uniontech.com/index.php?title=%E5%BD%B1%E9%99%A2)

## 帮助

* [Official Forum](https://bbs.uniontech.com/)
 * [Developer Center](https://github.com/linuxdeepin/developer-center)
 * [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 * [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 * [Wiki](https://wikidev.uniontech.com/)

## 贡献指南

* We encourage you to report issues and contribute changes

   - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
   - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
   - [Translate for your language on Transifex](https://www.transifex.com/linuxdeepin/deepin-movie/)

## 开源许可证

Deepin Movie 根据 [GPL-3.0-or-later]（许可证）获得许可.
