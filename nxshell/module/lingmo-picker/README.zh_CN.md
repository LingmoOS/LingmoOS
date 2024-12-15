# Lingmo Picker

Lingmo Picker: 简单的取色器工具.

### 依赖

* libxcb-util0
* libdtkgui
* libdtkwidget
* qttools5
* libqt5svg5
* libxtst

### 编译依赖

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

* debhelper (>= 9)
* Qt5(>=5.6)
* pkg-config
* qt5-qmake
* libxcb-util0-dev
* libdtkgui-dev
* libdtkwidget-dev
* qttools5-dev-tools
* libqt5svg5-dev
* libxtst-dev

## 安装

### 构建过程

1. 确保已安装所有依赖库.

_不同发行版的软件包名称可能不同，如果您的发行版提供了lingmo-picker，请检查发行版提供的打包脚本。_

如果你使用的是 [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) 或者其它提供了lingmo-picker的发行版:

``` shell
$ apt build-dep lingmo-picker
```

2. 构建:

```
$ cd lingmo-picker
$ mkdir build
$ cd build
$ qmake ..
$ make
```

3. 安装:

```
$ sudo make install
```

## 使用

命令行执行 `lingmo-picker`

## 帮助

任何使用问题都可以通过以下方式寻求帮助

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](http://wiki.lingmo.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for users](http://wiki.lingmo.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.lingmo.org/index.php?title=Contribution_Guidelines_for_Developers).

## 开源许可证

Lingmo Picker 在 [GPL-3.0-or-later](LICENSE.txt) 下发布。

