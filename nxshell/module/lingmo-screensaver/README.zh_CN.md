# lingmo-screensaver

深度屏保是一款自主研发的屏保服务。

### 依赖

  * Qt
  * xscreensaver
  * xscreensaver-data
  * startocean
  * qml-module-qt-labs-platform
  *  libdframeworkdbus

### 编译依赖

_当前的开发分支为**master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

 * pkg-config
 * qt5-qmake
 * qtbase5-dev
 * qtdeclarative5-dev
 * libqt5x11extras5-dev
 * libx11-dev
 * libxss-dev
 * libxext-dev
 * qttools5-dev-tools
 * qml-module-qt-labs-platform
 *  libdframeworkdbus-dev

## 安装

### 构建过程

1. 确保已经安装所有依赖库。

   _不同发行版的软件包名称可能不同，如果您的发行版提供了lingmo-screensaver ，请检查发行版提供的打包脚本。_

如果你使用的是[Lingmo](https://distrowatch.com/table.php?distribution=lingmo)或其他提供了全局搜索的基于Debian的发行版：

``` shell
$ apt build-dep lingmo-screensaver 
```

2. 构建:
```
$ cd lingmo-screensaver
$ mkdir Build
$ cd Build
$ qmake ../lingmo-screensaver.pro
$ make
```

3. 安装:
```
$ sudo make install
```

可执行程序为 `lingmo-screensaver` 。

## 文档

## 帮助

- [官方论坛](https://bbs.lingmo.org/) 
- [开发者中心](https://github.com/lingmoos/developer-center) 
- [Wiki](https://wiki.lingmo.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证

lingmo-screensaver 在 [GPL-3.0-or-later](LICENSE.txt)下发布。

