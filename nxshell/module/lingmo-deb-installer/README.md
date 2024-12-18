# Lingmo deb installer

Lingmo deb installer helps users install and remove local packages. Lingmo deb installer is an easy-to-use deb package management tool with a simple interface for users to quickly install customized applications not included in App Store supporting bulk installation, version identification and auto completion.

### Dependencies

- libqapt3
- libqapt3-runtime
- lingmo-elf-verify
- lingmo-elf-sign-tool

### Build dependencies

- pkg-config
- cmake
- libqt5widgets5
- libqt5concurrent5
- libqapt-dev
- libdtkwidget-dev
- qtbase5-dev
- qttools5-dev-tools
- qttools5-dev
- qtchooser
- libgtest-dev
- lingmo-gettext-tools
- libpolkit-qt5-1-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies

```shell
sudo apt build-dep lingmo-deb-installer
```

2. Build

```shell
mkdir build
cd build
cmake ..
make
```

3. Install

```shell
sudo make install
```

## Documentations

Please see [INTRODUCTION.md](./INTRODUCTION.md)

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)

## License

Lingmo deb installer is licensed under [GPL-3.0-or-later](LICENSE)
