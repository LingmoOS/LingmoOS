## lingmo-gomoku

Gomoku is an easy and funny chess game that is suitable for all ages.

五子棋是一款容易上手，老少皆宜，而且趣味横生，引人入胜的棋类小游戏。

### Core

The GUI is based on DTK, Qt (Supported >= 5.12).

Build-Depends: debhelper (>= 11),cmake, pkg-config, qtbase5-dev, qtchooser (>= 55-gc9562a1-1~),qt5-qmake, libdtkwidget-dev, qttools5-dev-tools, libqt5svg5-dev, qttools5-dev,libgtest-dev, libgmock-dev

## Installation

sudo apt-get install lingmo-gomoku

### Build

``` shell
mkdir build
cd build
cmake ..
make
make install
```

## Getting help

- [Official Forum](https://bbs.lingmo.org/) for generic discussion and help.
- [Developer Center](https://github.com/lingmoos/developer-center) for BUG report and suggestions.
- [Wiki](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

- [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
- [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
- [Translate for your language on Transifex](https://www.transifex.com/lingmoos/lingmo-gomoku/)

## License

Lingmo Gomoku is licensed under [GPL-3.0-or-later](LICENSE)