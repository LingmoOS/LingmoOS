## dde-appearance
dde-appearance 是一个用来设置deepin桌面主题和外观的程序。

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### Deepin

安装编译依赖

```shell
sudo apt-get build-dep dde-appearance
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
直接运行`/usr/bin/dde-appearance`, 它将提供`org.deepin.dde.Appearance1`服务。

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

dde-appearance 在 [GPL-3.0-or-later](LICENSE)下发布。
