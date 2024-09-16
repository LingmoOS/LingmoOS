# dde-calendar

Calendar is a small management tool for personal life that combines time and events and integrates the function of memorizing and scheduling.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* debhelper-compat (=11)
* cmake
* deepin-gettext-tools
* libdtkgui-dev (>= 5.5.17~)
* libdtkwidget-dev (>= 5.5.17~)
* libgtest-dev
* libqt5svg5-dev
* pkg-config
* qtbase5-dev
* qttools5-dev
* qttools5-dev-tools

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if dde-calendar is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got dde-calendar delivered:

``` shell
$ sudo apt-get build-dep dde-calendar
```

2. Build:

```
$ cd dde-calendar
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. Install:

```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/dde-calendar

## Usage

Execute `dde-calendar`

## Documentations

 - [Development Documentation](https://linuxdeepin.github.io/)
 - [User Documentation](https://wikidev.uniontech.com/index.php?title=%E7%94%BB%E6%9D%BF) 

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). 

## License

dde-calendar is licensed under [GPL-3.0-or-later](LICENSE.txt).