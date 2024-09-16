# 系统升级工具

## 背景

这是一个用于系统升级的工具。与传统的apt升级方法相比，我们使用根目录替换方法。

## 依赖

### 编译依赖

* cmake

* golang-any

* golang-yaml.v2-dev

* golang-dbus-dev

* golang-github-linuxdeepin-go-lib-dev
* libdtkwidget-dev

* libdtkgui-dev

* qtbase5-dev

* qttools5-dev-tools



## 运行 依赖

* [deepin-upgrade-manager](https://github.com/linuxdeepin/deepin-upgrade-manager)

* plymouth-label

* fuseiso

* dpkg-repack

* squashfs-tools

  

## 安装

1.安装编译依赖包

2.编译:

```
$ cd deepin-system-upgrade
$ mkdir build
$ cd build
$ cmake ..
$ make
```

3.安装:

```
$ sudo make install
```



## 用法

启动升级工具后，将出现UI界面，可以根据提示完成升级。

## 维护

* [github](https://github.com/l631197874)


## 许可协议

Deepin系统升级 在 [LGPL-3.0-or-later](LICENSE) 下发布。