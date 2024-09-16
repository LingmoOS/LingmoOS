# dde-grand-search

全局搜索是一款自主研发的系统级基础搜索工具，支持对多种文件类型、应用与设置的搜索。

### 依赖

  * Qt
  * DTK
  * libdframeworkdbus2
  * libgsettings-qt1
  * libicu63
  * libpoppler-qt5-1 
  * libqt5concurrent5 
  * libtag1v5
  * deepin-desktop-schemas
  * dde-daemon
  * startdde
  * deepin-anything-server

### 编译依赖

_当前的开发分支为**master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

 * pkg-config
 * qt5-qmake
 * qtbase5-dev
 * libdtkwidget-dev
 * libdtkcore-dev
 * libdtkgui-dev
 * libdtkcore5-bin
 * libdframeworkdbus-dev
 * qttools5-dev-tools
 * cmake
 * dde-dock-dev
 * libglib2.0-dev
 * libpoppler-qt5-dev
 * libtag1-dev
 * libicu-dev
 * libgsettings-qt-dev
 * libpcre3-dev

## 安装

### 构建过程

1. 确保已经安装所有依赖库。

   _不同发行版的软件包名称可能不同，如果您的发行版提供了dde-grand-search，请检查发行版提供的打包脚本。_

如果你使用的是[Deepin](https://distrowatch.com/table.php?distribution=deepin)或其他提供了全局搜索的基于Debian的发行版：

``` shell
$ apt build-dep dde-grand-search
```

2. 构建:
```
$ cd dde-grand-search
$ mkdir Build
$ cd Build
$ cmake ..
$ make   
```

3. 安装:
```
$ sudo make install
```

可执行程序为 `/usr/bin/dde-grand-search` 和 `/usr/bin/dde-grand-search-daemon`。

## 文档

## 帮助

- [官方论坛](https://bbs.deepin.org/) 
- [开发者中心](https://github.com/linuxdeepin/developer-center) 
- [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证

dde-grand-search 在 [GPL-3.0-or-later](LICENSE.txt)下发布。
