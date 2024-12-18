## Dde Wloutput Daemon
Dde Wloutput Daemon 是OCEAN KWayland桌面环境下用来进行显示设置的守护程序

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### 构建过程

1. 确保已经安装了所有的编译依赖

2. 构建
```
$ cd ocean-wloutput-daemon
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. 安装
```
$ sudo make install
```

安装完成后可执行二进制文件在/usr/bin/ocean-wloutput-daemon。

## 用法

执行 `/usr/bin/ocean-wloutput-daemon`。

## 获得帮助

如果您遇到任何其他问题，您可能会发现这些渠道很有用：

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=lingmo)
* [官方论坛](https://bbs.lingmo.org/)
* [Wiki](https://wiki.lingmo.org/)
* [项目地址](https://github.com/lingmoos/ocean-network-core) 

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

Dde Wloutput Daemon在 [GPL-3.0-or-later](LICENSE)下发布。
