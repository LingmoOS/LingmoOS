# Deepin Calculator

深度计算器是一个普通用户易于使用的计算器。

## 依赖关系

* sudo apt install libdtkwidget-dev libqt5svg5-dev qttools5-dev-tools cmake

## 安装

* mkdir build
* cd build
* cmake .. -DBUILD_TESTING=OFF
* make

## 测验

* sudo apt install libgtest-dev googletest libgmock-dev
* mkdir build
* camke ..
* cd tests
* make
* ./deepin-calculator-test

## 用法

* ./deepin-calculator

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并作出更改

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

Deepin Calculator 根据 [GPL-3.0-or-later](LICENSE) 或任何更高版本获得许可.

