# Deepin Service Manager

------------
Manage DBus service on Deepin.

## Dependencies

### Build dependencies

* Qt >= 5.11
* cmake >= 3.13
* systemd >= 241.46

## Build and install

### Build from source code

```bash
git clone https://github.com/linuxdeepin/deepin-service-manager.git
cd deepin-service-manager
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

### Install

```bash
sudo cmake --build build --target install
```

## Getting help

Any usage issues can ask for help via

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

Deepin Service Manager is licensed under [LGPL-3.0-or-later](LICENSE).
