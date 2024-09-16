# DDE Launcher

深度桌面环境的启动器

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### 构建过程

1. 确保已经安装了所有的编译依赖
```bash
sudo apt build-dep dde-launcher
```

2. 构建
```bash
mkdir build
cd build
cmake ..
make
```

3. 安装
```bash
sudo make install
```
## 帮助
任何使用问题都可以通过以下方式寻求帮助：

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues) 

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证
dde-launcher 在 [GPL-3.0-or-later](LICENSE) 下发布。
