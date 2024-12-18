# qt5integration

qt5integration is the Qt platform theme integration plugins for Lingmo Desktop Environment. It includes multiple Qt5 plugins
to make make OCEAN more Qt5 Apps friendly.

## Dependencies

### Build dependencies

* gtk+-2.0
* mtdev
* [dtk](https://github.com/lingmoos/lingmo-tool-kit)
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

* OCEAN File Manager (suggestion, to make Qt5 file dialogs more elegant)

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd qt5integration
$ mkdir build
$ cd build
$ qmake ..
$ make
```

3. Install:
```
$ sudo make install
```

## Usage

To be done.

## Getting help

You may also find these channels useful if you encounter any other issues:

* [Telegram group](https://t.me/lingmo)
* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#lingmo-community)
* [Official Forum](https://bbs.lingmo.org/)
* [Wiki](http://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

qt5integration is licensed under [LGPL-3.0-or-later](LICENSE).
