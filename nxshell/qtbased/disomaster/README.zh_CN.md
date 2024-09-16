# DISOMaster

DISOMaster提供基本的光盘驱动器操作和磁盘上的文件系统操作。

### 依赖

  * Qt 5.7
  * libisoburn 1.2.6
  * 支持C++11的新C++编译器

### 编译依赖

  * debhelper (>= 9)
  * pkg-config
  * qtbase5-dev
  * qt5-qmake
  * libisoburn-dev

## 安装

### 构建过程

1. 确保已经安装所有依赖库。

``` shell
$ apt build-dep disomaster 
```

2. 构建:
```
$ cd disomaster
$ mkdir Build
$ cd Build
$ qmake ../disomaster.pro
$ make
```

3. 安装:
```
$ sudo make install
```

可执行程序为 `disomaster` 。

## 文档

## 帮助

- [官方论坛](https://bbs.deepin.org/) 
- [开发者中心](https://github.com/linuxdeepin/developer-center) 
- [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证

disomaster 在 [GPL-3.0-or-later](LICENSE.txt)下发布。