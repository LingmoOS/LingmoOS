## OCEAN Control Center

OCEAN Control Center is the control panel of Lingmo Desktop Environment.
## Dependencies

### Build dependencies
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

### Runtime dependencies

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

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd ocean-control-center
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/ocean-control-center` after the installation is finished, and plugins will be placed into `${CMAKE_INSTALL_FULL_LIBDIR}/ocean-control-center/modules/`, usually is `/usr/lib/ocean-control-center/modules/`.

## Usage

Execute `ocean-control-center -h` to get more details.

Note: `--spec` can be used to debug plugins. The passed in value is the path where the so of plugin is in.

## Getting help

You can press `F1` to start [lingmo-manual](https://github.com/lingmoos/lingmo-manual) when you focus on OCEAN Control Center window.

You may also find these channels useful if you encounter any other issues:

* [Telegram group](https://t.me/lingmo)
* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#lingmo-community)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## Develop Plugins with cmake

```cmake

# just show the target link way

find_pacakge(DdeControlCenter REQUIRED)
find_pacakge(Dtk COMPONENTS Core Widget REQUIRED)
find_pacakge(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

add_library(oceanui_exampleplugin SHARED
    plugin.h
    plugin.cpp
)

target_link_libraries(oceanui_exampleplugin PRIVATE
    Dde::DCCWidget
    Dde::DCCInterface
    Dtk::Core
    Dtk::Widget
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

```

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

OCEAN Control Center is licensed under [GPL-3.0-or-later](LICENSE).
