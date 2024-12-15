## OCEAN Display
OCEAN Display 是ocean 桌面环境的显示管理模块，提供显示服务。

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### 构建过程

1. 确保已经安装了所有的编译依赖

2. 构建
```
$ cd ocean-display
$ mkdir Build
$ cd Build
$ cmake ..
$ make
```

3. 安装
```
$ sudo make install
```

安装完成后可执行二进制文件在/usr/bin/ocean-display。

## 用法

执行 `ocean-dispaly-console -h` 以获取更多详细信息。

## 获得帮助

当你正在使用 OCEAN 控制中心显示模块时，您可以按 `F1` 启动 [lingmo-manual](https://github.com/lingmoos/lingmo-manual)。

如果您遇到任何其他问题，您可能还会发现这些渠道很有用：

* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [官方论坛](https://bbs.lingmo.org/)
* [Wiki](https://wiki.lingmo.org/)
* [项目地址](https://github.com/lingmoos/ocean-control-center)
* [开发者中心](https://github.com/lingmoos/developer-center/issues) 

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

OCEAN Display在 [LGPL-3.0-or-later](LICENSE)下发布。
