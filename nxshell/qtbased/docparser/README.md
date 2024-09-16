### docparser

docparser is a document parser library.It is ported from [document2html](https://github.com/dmryutov/document2html).

### Dependencies

### Build dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

* qtbase5-dev
*  qt5-qmake
*  pkg-config
*  libpoppler-cpp-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if docpraser is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got docparser delivered:

``` shell
$ apt build-dep docparser
```

2. Build:
```
$ cd docparser
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

## Documentations


## Getting help

 - [Official Forum](https://bbs.deepin.org/)
 - [Developer Center](https://github.com/linuxdeepin/developer-center)
 - [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
 - [IRC Channel](https://webchat.freenode.net/?channels=deepin)
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)
 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)
 - [Translate for your language on Transifex](https://www.transifex.com/linuxdeepin/deepin-file-manager/)


## License

docparser is licensed under [LGPL-3.0-or-later](LICENSE.txt)

