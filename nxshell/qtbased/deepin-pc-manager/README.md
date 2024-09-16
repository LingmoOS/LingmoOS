<!--
SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.

SPDX-License-Identifier: GPL-3.0-or-later
-->

# Deepin PC Manager
Deepin PC Manager is an application tool designed to help users quickly manage, maintain, and optimize computer systems.

## Dependencies
* libdtkwidget-dev
* libdframeworkdbus-dev
* libdtkgui-dev
* qtbase5-dev
* libgsettings-qt-dev
* qttools5-dev-tools
* libqt5x11extras5-dev
* libprocps-dev
* libxext-dev
* libncurses-dev
* libdtkcore5-bin
* libqt5multimedia5
* libqt5svg5


### Build dependencies

* cmake
* qt5
* dtk


## Installation
sudo apt-get install deepin-pc-manager

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd deepin-pc-manager
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/deepin-pc-manager`.

## Getting help

Any usage issues can ask for help via
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/developer-center)

## Getting involved

We encourage you to report issues and contribute changes

* [**Contribution guide for developers**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en)


## License
This project is licensed under  GPL-3.0-only.