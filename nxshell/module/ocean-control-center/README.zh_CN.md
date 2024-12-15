## OCEAN 控制中心

OCEAN 控制中心控制整个OCEAN桌面环境

## 依赖

### 构建依赖

* pkg-config
* cmake
* Qt5(>= 5.6) with modules:
  - qt5-default
  - qt5declarative5-dev
  - qt5svg5-dev
  - qttools5-dev-tools
  - qtdeclarative5-dev
* Lingmo-tool-kit(>=2.0) with modules:
  - dtkcore-dev
  - dtkwidget-dev
  - dtkcore-bin
* geoip-dev
* ocean-qt-dbus-factory
* gsettings-qt-dev
* kf5networkmanagerqt-dev
* xrandr-dev
* mtdev-dev
* fontconfig1-dev
* freetype6-dev
* egl1-mesa-dev
* xcb-ewmh-dev

### 运行时依赖

* [lingmo-tool-kit](https://github.com/lingmoos/lingmo-tool-kit)
* [ocean-api](https://github.com/lingmoos/ocean-api)
* [ocean-daemon](https://github.com/lingmoos/ocean-daemon)
* [startocean](https://github.com/lingmoos/startocean)
* [ocean-qt-dbus-factory](https://github.com/lingmoos/ocean-qt-dbus-factory)
* [qt5integration](https://github.com/lingmoos/qt5integration)
* geoip-database
* Qt5(>= 5.6)
  * Qt5-DBus
  * Qt5-Multimedia
  * Qt5-MultimediaWidgets
  * Qt5-Svg

## 安装

### 源码安装

1. Make sure you have installed all dependencies.

2. Build:
```bash
cd ocean-control-center
cmake -B build
cmake --build build 
```

3. Install:
```bash
sudo make install
```
注意，不推荐，应该先打包然后安装。

运行文件在 `/usr/bin/ocean-control-center` , 插件在 `${CMAKE_INSTALL_FULL_LIBDIR}/ocean-control-center/modules/`下，`CMAKE_INSTALL_FULL_LIBDIR`请查阅GNUInstallDir,根据发行版有所不同 , 如arch下这个目录在 `/usr/lib/ocean-control-center/modules/`.

## 使用方式

Execute `ocean-control-center -h` to get more details.

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Telegram 群组](https://t.me/lingmo)
* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#lingmo-community)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)



## 加入我们

请到developer-center提交issue或者参与讨论。当然也可以直接参与代码维护提交代码和修复。我们会很开心获得您的帮助和贡献

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

OCEAN Control Center 遵循协议 [GPL-3.0-or-later](LICENSE).
