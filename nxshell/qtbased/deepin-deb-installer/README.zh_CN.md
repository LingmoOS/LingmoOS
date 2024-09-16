# 深度软件包安装器

深度软件包安装器帮助用户安装和卸载本地的软件包，它是一个有着简单界面且易用的包管理工具，它可以让用户安装没有在应用商店中的自己的软件包，同时它还支持批量安装、版本识别以及自动解决依赖。

### 安装依赖包

- libqapt3
- libqapt3-runtime
- deepin-elf-sign-tool

### 构建依赖包

- pkg-config
- cmake
- libqt5widgets5
- libqt5concurrent5
- libqapt-dev
- libdtkwidget-dev
- qtbase5-dev
- qttools5-dev-tools
- qttools5-dev
- qtchooser
- libgtest-dev
- deepin-gettext-tools
- libpolkit-qt5-1-dev

## 编译安装

### 从源码构建

1. 确保已经安装了所有的构建依赖

```shell
sudo apt build-dep deepin-deb-installer
```

2. 执行构建

```shell
mkdir build
cd build
cmake ..
make
```

3. 安装

```shell
sudo make install
```

## 文档

部分开发文档请参考 [INTRODUCTION.md](./INTRODUCTION.md)

## 获得帮助

任何使用问题都可以通过以下方式寻求帮助

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 代码贡献

我们鼓励报告问题并做出更改，相关内容请参考一下文档

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers). (中文)

## License

深度软件包安装器的开源协议是 [GPL-3.0-or-later](LICENSE)

