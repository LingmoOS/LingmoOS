# dtknetworkmanager

Development Tool Kit NetworkManger Module, Qt wrapper for NetworkManager DBus API.

[中文文档](./README.zh_CN.md)

## Dependencies

### Runtime Dependency

`NetWorkManager >= 1.14.6`

### Build Dependencies

- Qt >= 5.11
- libdtkcore-dev >= 5.6.2 (or build from github source code),
- CMake >= 3.13
- qttools5-dev-tools,
- doxygen,
- libnm-dev >= 1.14.6,
- libglib2.0-dev

## Build And Install

### Build From Source Code

```bash
git clone https://github.com/linuxdeepin/dtknetworkmanager.git
cd dtknetworkmanager
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

### Install

```bash
sudo cmake --build build --target install
```

## Getting help

Any usage issues can ask for help via

- [Telegram group](https://t.me/deepin)
- [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
- [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
- [Forum](https://bbs.deepin.org)
- [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes:

- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

Development Tool Kit is licensed under [LGPL-3.0-or-later](LICENSE).
