# DDE Application Manager

当前分支用于完全重构。

DDE Application Manager 是深度桌面环境的应用程序管理器。

### 依赖

请查看“debian/control”文件中提供的“Depends”。

### 编译依赖

请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### 构建过程

1. 确保已经安装了所有的编译依赖。


2. 构建:

```shell
$ cd dde-application-manager
$ cmake -B build
$ cmake --build build -j`nproc`
```
3. Install

```
sudo cmake --install build
```

## 帮助

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues) 

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

dde-application-manager 在 [GPL-3.0-or-later](LICENSE)下发布。
