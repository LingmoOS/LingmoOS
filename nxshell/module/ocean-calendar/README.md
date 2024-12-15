# ocean-calendar

Calendar is a small management tool for personal life that combines time and events and integrates the function of memorizing and scheduling.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* debhelper-compat (=11)
* cmake
* lingmo-gettext-tools
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

_Package name may be different between distros, if ocean-calendar is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got ocean-calendar delivered:

``` shell
$ sudo apt-get build-dep ocean-calendar
```

2. Build:

```
$ cd ocean-calendar
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. Install:

```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/ocean-calendar

## Usage

Execute `ocean-calendar`

## Documentations

 - [Development Documentation](https://lingmoos.github.io/)

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). 

## License

ocean-calendar is licensed under [GPL-3.0-or-later](LICENSE.txt).