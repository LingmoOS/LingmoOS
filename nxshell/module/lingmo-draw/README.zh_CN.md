# lingmo-draw

画板是一款轻量级的绘图工具，支持在画板上自由的绘图和简单的图片编辑。

### 依赖

### 编译依赖

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

* Qt (>=5.6),
* debhelper (>=9)
* cmake
* qtbase5-dev
* pkg-config
* libexif-dev
* libqt5svg5-dev
* libraw-dev
* libfreeimage-dev
* libqt5opengl5-dev
* qttools5-dev
* qttools5-dev-tools
* libdtkgui-dev
* libdtkwidget-dev
* libxcb-util0-dev
* libdtkcore5-bin
* libgtest-dev
* libgmock-dev

## 安装

### 构建过程

1. 确保已安装所有依赖库.

_不同发行版的软件包名称可能不同，如果您的发行版提供了lingmo-draw，请检查发行版提供的打包脚本。_

如果你使用的是 [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) 或者其它提供了lingmo-draw的发行版:

``` shell
$ sudo apt-get build-dep lingmo-draw
```

2. 构建:

```
$ cd draw
$ mkdir Build
$ cd Build
$ cmake ../
$ make
```

3. 安装:

```
$ sudo make install
```

可执行程序为 `/usr/bin/lingmo-draw

## 使用

命令行执行 `lingmo-draw`

## 文档

 - [Development Documentation](https://lingmoos.github.io/)
 - [用户文档](https://wikidev.uniontech.com/index.php?title=%E7%94%BB%E6%9D%BF)

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 开源许可证

lingmo-draw 在 [GPL-3.0-or-later](LICENSE.txt) 下发布.
