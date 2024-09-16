# gio-qt: Gio 的 Qt 封装库

## 描述

这是一个 Gio 的封装库（或者说它是一个主要专注于 [GIO](https://gitlab.gnome.org/GNOME/glibmm/tree/master/gio) 模块的 glib/glibmm 封装库）。这个库被设计为无异常的，避免 Qt 应用程序开发人员直接访问 glib/glibmm（这样他们就可以以 Qt 的方式使用 Gio）。

## 构建
### 编译依赖

 - cmake `>= 3.12.4`
 - pkg-config (查找和使用giomm模块)
 - Qt5Core `>= 5.6.3`
 - Qt5Test (如果你想构建单元测试)
 - giomm-2.4
 - doxygen (构建文档)

如果您使用的是 cmake 版本较低的旧系统来尝试构建此库，请关闭 `BUILD_DOCS` 选项。

### 构建安装

``` shell
$ cd /path/to/your/source/code
$ mkdir build
$ cd build
$ cmake ../
$ make
$ sudo make install
```

默认情况下，它将安装到 `/usr/local/lib`。安装后，您可能还需要执行 `sudo ldconfig /usr/local/lib`。

## 帮助

[官方论坛](https://bbs.deepin.org/) 

[开发者中心](https://github.com/linuxdeepin/developer-center) 

 [Wiki](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

[开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## License

gio-qt 在 [LGPL-3.0-or-later](LICENSE) 下发布
