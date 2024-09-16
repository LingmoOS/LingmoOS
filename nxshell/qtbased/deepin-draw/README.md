# deepin-draw

Draw is a lightweight drawing tool for users to freely draw and simply edit images developed by Deepin Technology.

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

_Package name may be different between distros, if deepin-draw is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got deepin-draw delivered:

``` shell
$ sudo apt-get build-dep deepin-draw
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

The executable binary file could be found at `/usr/bin/deepin-draw

## Usage

Execute `deepin-draw`

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

deepin-draw is licensed under [GPL-3.0-or-later](LICENSE.txt).
