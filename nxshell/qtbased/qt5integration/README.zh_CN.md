## qt5integration

qt5integration 是用于深度桌面环境的Qt平台主题集成插件。它包括多个Qt5插件使DDE对Qt5应用程序更加友好。

## 依赖

### 编译依赖

* gtk+-2.0
* mtdev
* [dtk](https://github.com/linuxdeepin/deepin-tool-kit)
* [libqtxdg](https://github.com/lxde/libqtxdg)
* Qt5 (>= 5.6)
  * Qt5-Core
  * Qt5-Gui
  * Qt5-Widgets
  * Qt5-DBus
  * Qt5-Multimedia
  * Qt5-MultimediaWidgets
  * Qt5-Svg
  * Qt5-OpenGL
  * Qt5-X11extras
  * Qt5-Core-Private
  * Qt5-Widgets-Private

### Runtime dependencies

* DDE文件管理器（建议使Qt5文件对话框更优雅）

## 安装

### 从源代码构建

1. 确保已经安装了所有的编译依赖.

2. 构建:

```bash
mkdir build
cd build
qmake ..
make
```

3. 安装:

```bash
sudo make install
```

## 使用

待补充.

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Telegram 群组](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

qt5integration遵循协议 [LGPL-3.0-or-later](LICENSE).
