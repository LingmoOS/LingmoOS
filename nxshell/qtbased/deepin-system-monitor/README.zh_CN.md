# Deepin System Monitor

深度系统监视器：更用户友好的系统监视器。

Thanks a lot for [Lily Rivers](https://github.com/VioletDarkKitty/system-monitor)'s working, this project used a lot of his code.

## 依赖关系

* sudo apt install libpcap-dev libncurses5-dev libxtst-dev libxcb-util0-dev

## 安装

* mkdir build
* cd build
* qmake ..
* make
* sudo setcap cap_kill,cap_net_raw,cap_dac_read_search,cap_sys_ptrace+ep ./deepin-system-monitor

## 用法

* ./deepin-system-monitor

## 配置文件

* ~/.config/deepin/deepin-system-monitor/config.conf

## 帮助

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## 参与贡献

We encourage you to report issues and contribute changes

* [Contribution guide for users](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Developers).

## 证书

This project is licensed under GPLv3 or any later version.
