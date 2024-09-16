# dtkdeclarative

dtkdeclarative is a widget development toolkit based on QtQuick/QtQml, which is a brand new substitute for dtkwidget. dtkdeclarative is developed based on qtdeclarative. It covers all existing QML widgets and adds plenty of DTK friendly visual effects and color schemes. Compared to dtkwidget. It has:

1. **A primitive Qt and Qml code style**
2. **Adapted APIs with traditional Qml**
3. **Simple and quick development interfaces**
4. **Unified widget theme style**
5. **Abundant effects and colors**

[中文介绍](README.zh_CN.md)

## Document

[dtkdeclarative 中文文档](https://linuxdeepin.github.io/dtkdeclarative/index.html)


## Dependencies

+ qml-module-qtquick-shapes
+ qml-module-qtquick-layouts

## Build dependencies

+ qtdeclarative5-dev
+ qtbase5-dev-tools,
+ qtquickcontrols2-5-dev
+ libdtkgui-dev
+ libdtkcore-dev
+ qtdeclarative5-private-dev
+ qtbase5-private-dev

### Build from source

1. Please make sure that all dependencies are installed
2. Get source and build

```shell
$ git clone https://github.com/linuxdeepin/dtkdeclarative.git
$ cmake -Bbuild -GNinja
$ cmake --build build
```

3. Install

```shell
$ sudo cmake --install build
```

## Getting help

+ Please feel free to report issues if you encounter any problem
+ [Deepin Community](https://bbs.deepin.org/) for generic discussion and help.

## Getting Involved

Any usage issues can ask for help via

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)
 If you find a definite bug, you can raise it by way of an issue and we will fix it as soon as possible.
 If you have some development skills, you are welcome to submit a PR.

## License

dtkdeclarative is licensed under the [LGPL-3.0-or-later](LICENSE)

## TODO

1. Add plugins.qmltypes, support code completion
2. Support Vulkan
