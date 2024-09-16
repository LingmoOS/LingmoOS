## dde-session
dde-session 是一个用于启动 DDE 组件 systemd 服务的项目。
该项目涉及了一部分 GNOME 的会话文档和文件。

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

### 运行依赖
- startdde [https://github.com/linuxdeepin/startdde] (https://github.com/linuxdeepin/startdde)
- systemd

## 安装

### Deepin

安装编译依赖

```shell
sudo apt-get build-dep dde-session
```

构建

```shell
mkdir build && cd build && cmake ../ -DCMAKE_INSTALL_PREFIX=/usr && make
```

如果你有独立的测试构建环境（比如 docker 容器），你可以直接安装它。

```shell
sudo make install
```

## 用法

### 使用显示管理器运行

1. 加载 /usr/share/xsessions下的文件构建一个桌面会话

```shell
cat /usr/share/xsessions/deepin.desktop
```

```text
[Desktop Entry]
Name=Deepin
Comment=Deepin Desktop Environment
Exec=/usr/bin/dde-session
```

2. 使用 DisplayManager，如 gdm、kdm 或 lightdm 来启动 Deepin。


## 获得帮助

您遇到的任务问题都可以通过以下方式寻求帮助

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [官方论坛](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)
* [项目地址](https://github.com/linuxdeepin/dde-session)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues)

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

dde-session 在 [LGPL-3.0-or-later](LICENSE)下发布。
