# lingmo-draw

Draw is a lightweight drawing tool for users to freely draw and simply edit images developed by Lingmo Technology.

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* Qt (>=5.6),
* debhelper (>=9)
* cmake
* qtbase5-dev
* pkg-config
* libexif-dev
* libqt5svg5-dev
* libraw-dev
* libfreeimage-dev
* libqt5opengl5-dev
* qttools5-dev
* qttools5-dev-tools
* libdtkgui-dev
* libdtkwidget-dev
* libxcb-util0-dev
* libdtkcore5-bin
* libgtest-dev
* libgmock-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if lingmo-draw is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got lingmo-draw delivered:

``` shell
$ sudo apt-get build-dep lingmo-draw
```

2. Build:

```
$ cd draw
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. Install:

```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/lingmo-draw

## Usage

Execute `lingmo-draw`

## Documentations

 - [Development Documentation](https://lingmoos.github.io/)
 - [User Documentation](https://wikidev.uniontech.com/index.php?title=%E7%94%BB%E6%9D%BF) 

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

lingmo-draw is licensed under [GPL-3.0-or-later](LICENSE.txt).
