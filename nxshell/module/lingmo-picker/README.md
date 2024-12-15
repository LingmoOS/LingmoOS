# Lingmo Picker

Lingmo Picker: Simplest color picker.

## Dependencies

* libxcb-util0
* libdtkgui
* libdtkwidget
* qttools5
* libqt5svg5
* libxtst

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* debhelper (>= 9)
* Qt5(>=5.6)
* pkg-config
* qt5-qmake
* libxcb-util0-dev
* libdtkgui-dev
* libdtkwidget-dev
* qttools5-dev-tools
* libqt5svg5-dev
* libxtst-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if lingmo-picker is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got lingmo-picker delivered:

``` shell
$ apt build-dep lingmo-album
```

2. Build:

```
$ cd lingmo-picker
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

Execute `lingmo-picker`

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](http://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for users](http://wiki.lingmo.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.lingmo.org/index.php?title=Contribution_Guidelines_for_Developers).

## License

Lingmo Picker is licensed under [GPL-3.0-or-later](LICENSE.txt).

