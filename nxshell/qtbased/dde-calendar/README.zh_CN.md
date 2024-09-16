# 日历

日历是一款查看日期、管理日程的小工具。

### 依赖

### 编译依赖

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

* debhelper-compat (=11)
* cmake
* deepin-gettext-tools
* libdtkgui-dev (>= 5.5.17~)
* libdtkwidget-dev (>= 5.5.17~)
* libgtest-dev
* libqt5svg5-dev
* pkg-config
* qtbase5-dev
* qttools5-dev
* qttools5-dev-tools

## 安装

### 构建过程

1. 确保已安装所有依赖库.

_不同发行版的软件包名称可能不同，如果您的发行版提供了日历，请检查发行版提供的打包脚本。_

如果你使用的是 [Deepin](https://distrowatch.com/table.php?distribution=deepin) 或者其它提供了日历的发行版:

``` shell
$ sudo apt-get build-dep dde-calendar
```

2. 构建:

```
$ cd dde-calendar
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. 安装:

```
$ sudo make install
```

可执行程序为 `/usr/bin/dde-calendar

## 使用

命令行执行 `dde-calendar`

## 文档

 - [Development Documentation](https://linuxdeepin.github.io/)
 - [用户文档](https://wikidev.uniontech.com/index.php?title=%E7%94%BB%E6%9D%BF)

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 开源许可证

日历是在 [GPL-3.0-or-later](LICENSE.txt) 下发布.