# Lingmo desktop schemas

gsettings项目

## 被依赖项目

* ocean-api
* ocean-daemon
* ocean-workspace
* lingmo-metacity
* lingmo-mutter
* startocean

### 依赖

请查看“debian/control”文件中提供的“Depends”。

### 编译依赖

请查看“debian/control”文件中提供的“Build-Depends”。

## 安装

### 构建过程

1. 确保已经安装了所有的编译依赖

2. 构建
```
$ cd lingmo-desktop-schemas
$ make
```

3. 安装
```
$ sudo make install
```

## 帮助

任何使用问题都可以通过以下方式寻求帮助：

* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [官方论坛](https://bbs.lingmo.org/) 
* [开发者中心](https://github.com/lingmoos/developer-center/issues) 
* [Wiki](https://wiki.lingmo.org/)
* [项目地址](https://github.com/lingmoos/lingmo-desktop-schemas) 

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

lingmo-face在 [GPL-3.0-or-later](LICENSE)下发布。
