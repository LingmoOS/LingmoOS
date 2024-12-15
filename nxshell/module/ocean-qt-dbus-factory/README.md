# ocean-qt-dbus-factory

`ocean-qt-dbus-factory` is used to uniformly store the automatically generated `Qt DBus` code used by `ocean`, so that `ocean` developers do not have to create the same `DBus` code repeatedly in their own projects, or copy files from other projects into their own projects.

## Dependencies
You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies
You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.
```bash
sudo apt build-dep ocean-qt-dbus-factory
```

2. Build
```bash
mkdir build
cd build
cmake ..
make
```

3. Install
```bash
sudo make install
```
## Getting help

Any usage issues can ask for help via

* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [WiKi](https://wiki.lingmo.org)
* [Forum](https://bbs.lingmo.org)
* [Developer Center](https://github.com/lingmoos/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

- [**Contribution guide for developers**](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License
ocean-qt-dbus-factory is licensed under [GPL-3.0-or-later](LICENSE).
