## dde-session-ui
**描述**：本项目包括以下子项目：

- dde-shutdown
```
关机界面
```
- dde-lock
```
锁屏界面
```
- dde-lockservice
```
锁屏后台服务
```
- lightdm-deepin-greeter
```
登录时的界面
```
- dde-switchtogreeter
```
切换用户登录的工具
```
- dde-lowpower
```
低电量提醒界面
```
- dde-osd
```
屏幕叠加显示的界面
```
- dde-hotzone
```
设置热区的界面
```

## 依赖
请查看“debian/control”文件中提供的“Depends”

## 安装
那些项目的安装都是一样的，所以我们只是举个例子。
安装 lightdm-deepin-greeter 的示例：

- 首先需要进入lightdm-deepin-greeter.pro目录；
- 创建一个目录，进入该目录，然后运行`qmake .. && make`；
- 最后运行`sudo make install`。
- 如果你想让你的安装工作，你可能需要改变一些配置文件
确保 lightdm-deepin-greeter 正在运行，或者它可能会运行旧的。
在深度操作系统中，您需要将文件lightdm-deepin-greeter（通过运行`make`获得）复制到目录“/usr/local/bin”。

## 使用
子项目得到的工具都是系统工具，登录的时候可以使用lightdm-deepin-greeter，也可以使用dde-shutdown
当你选择关闭你的电脑时，等等。

## 获得帮助
* [github](https://github.com/linuxdeepin/dde-session-ui)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [官方论坛](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)
* [项目地址](https://github.com/linuxdeepin/dde-session-ui) 
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues)

## 贡献指南

我们鼓励您报告问题并做出更改

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License
dde-session-ui 在 [GPL-3.0-or-later](LICENSE) 下发布。
