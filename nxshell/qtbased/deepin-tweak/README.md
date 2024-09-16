# Deepin Tweak

Deepin Tweak is an advanced setting tool built on dtkdeclarative. Deepin Tweak only provides limited built-in functions, most of which need to be provided by other developers in the community according to the requirements of plug-in development.

[中文文档](README.zh_CN.md)

## Dependencies

* dtkdeclarative

## Build dependencies

* cmake
* libdtkdeclarative-dev
* qtdeclarative5-dev
* qtbase5-dev-tools
* qtquickcontrols2-5-dev
* libdtkgui-dev
* libdtkcore-dev
* libgsettings-qt-dev

## Installation

### Build from source

1. Please make sure that all dependencies are installed
2. Get source and build

```shell
$ git clone https://github.com/linuxdeepin/deepin-tweak.git
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

3. Install

```shell
$ sudo cmake --install build
```

## Getting help

* Please feel free to report issues if you encounter any problem
* [Deepin Community](https://bbs.deepin.org/) for generic discussion and help.

## Getting Involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en)(English)

## License

deepin-tweak is licensed under the [LGPL-3.0-or-later](LICENSE)
