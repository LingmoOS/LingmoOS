# Download Manager

下载管理器是深度桌面环境下载管理器。

## 依赖

### 编译依赖

* cmake
* qt5
* aria2


### 运行依赖

* Qt5 (>= 5.3)
* Qt5-X11extras


## 安装

### 从源代码构建

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd deepin-downloader
$ mkdir Build
$ cd Build
$ cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr ..
$ make
```

3. Install:
```
$ sudo make install
```

安装完成后，可执行二进制文件放入 `/usr/bin/deepin-downloadmanager`.

## 获得帮助

任何使用问题都可以通过以下方式寻求帮助
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)


## 开源许可证
该项目在GPL-3.0-or-later下发布。