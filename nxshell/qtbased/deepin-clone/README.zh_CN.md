# deepin-clone
备份/还原磁盘数据的工具


## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

```
sudo apt install deepin-fcitxconfigtool-plugin 
```

### 构建过程

```
sudo apt build-dep deepin-clone
cd ./deepin-clone
sudo dpkg-buildpackage -b
cd ..
sudo dpkg -i *.deb
```

## 帮助
任何使用问题都可以通过以下方式寻求帮助：
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 开源许可证
该项目在GPL-3.0-only下发布。

