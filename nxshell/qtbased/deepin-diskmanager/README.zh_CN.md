# 磁盘管理器
磁盘管理器是一款管理磁盘的工具，可帮助用户进行磁盘分区管理、磁盘数据管理及磁盘健康管理。

## 依赖
* smartmontools
* exfat-fuse
* Qt5 (>= 5.3)
* Qt5-X11extras

### 编译依赖
* cmake
* qt5
* dtk


## 安装
sudo apt-get install deepin-diskmanager

### 构建过程

1. 确保您安装了所有的依赖包

2. 编译:
```
$ cd deepin-diskmanager
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. 安装:
```
$ sudo make install
```

安装完成后，可执行二进制文件将放入`/usr/bin/deepin-diskmanager`.


## 帮助

任何使用问题都可以通过以下方式寻求帮助：
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center)


## 开源许可证
This project is licensed under  GPL-3.0-only.

