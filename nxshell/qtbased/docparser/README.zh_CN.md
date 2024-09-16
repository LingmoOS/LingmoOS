### docparser

docparser 是一个从 [document2html](https://github.com/dmryutov/document2html) 移植过来的文件解析库.

### 依赖

### 编译依赖

_当前的开发分支为 **master**，编译依赖可能会在没有更新本说明的情况下发生变化，请参考`./debian/control`以获取构建依赖项列表_

* qtbase5-dev
*  qt5-qmake
*  pkg-config
*  libpoppler-cpp-dev

## 安装

### 构建过程

1. 确保已经安装所有依赖库。

   _不同发行版的软件包名称可能不同，如果您的发行版提供了 docparser，请检查发行版提供的打包脚本。_

如果你使用的是 [Deepin](https://distrowatch.com/table.php?distribution=deepin) 或其他提供了 `docparser` 的基于 Debian 的发行版：

``` shell
$ apt build-dep docparser
```

2. 构建:
```
$ cd docparser
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. 安装:
```
$ sudo make install
```

## Documentations


## 帮助

 - [官方论坛](https://bbs.deepin.org/) 
- [开发者中心](https://github.com/linuxdeepin/developer-center) 
- [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

 - [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 


## 开源许可证

docparser is 在 [LGPL-3.0-or-later](LICENSE.txt)

