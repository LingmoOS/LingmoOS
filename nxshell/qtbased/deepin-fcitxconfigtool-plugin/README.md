# deepin-fcitxconfigtool-plugin 

The input method management plug-in of DDE control center has been adapted based on fcitx4 input method framework.

## Dependencies
You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies
You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

```
sudo apt install deepin-fcitxconfigtool-plugin 
```

### Build from source code
```
sudo apt build-dep deepin-fcitxconfigtool-plugin 
cd ./deepin-fcitxconfigtool-plugin 
sudo dpkg-buildpackage -b
cd ..
sudo dpkg -i *.deb
```

## Getting help

Any usage issues can ask for help via
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)


## Getting involved

We encourage you to report issues and contribute changes
- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)


## License

This project is licensed under GPL-3.0-or-later.
