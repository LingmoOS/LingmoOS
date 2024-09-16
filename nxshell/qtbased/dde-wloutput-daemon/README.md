## Dde Wloutput Daemon

Dde Wloutput Daemon is a daemon for display settings in the DDE KWayland desktop environment

## Dependencies
You can also check the "Depends" provided in the debian/control file.

### Build dependencies
You can also check the "Build-Depends" provided in the debian/control file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd dde-wloutput-daemon
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `/usr/bin/dde-wloutput-daemon` after the installation is finished.

## Usage

Execute `/usr/bin/dde-wloutput-daemon`.

## Getting help

You may find these channels useful if you encounter any other issues:

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=deepin)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)
* [Developer Center](https://github.com/linuxdeepin/dde-wloutput-daemon) 

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

Dde Wloutput Daemon is licensed under [GPL-3.0-or-later](LICENSE).
