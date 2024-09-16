# Deepin System Monitor

Deepin system monitor: a more user-friendly system monitor.

Thanks a lot for [Lily Rivers](https://github.com/VioletDarkKitty/system-monitor)'s working, this project used a lot of his code.

## Dependencies

* sudo apt install libpcap-dev libncurses5-dev  libxtst-dev libxcb-util0-dev

## Installation

* mkdir build
* cd build
* qmake ..
* make
* sudo setcap cap_kill,cap_net_raw,cap_dac_read_search,cap_sys_ptrace+ep ./deepin-system-monitor

## Usage

* ./deepin-system-monitor

## Config file

* ~/.config/deepin/deepin-system-monitor/config.conf

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for users](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.deepin.org/index.php?title=Contribution_Guidelines_for_Developers).

## License

This project is licensed under GPLv3 or any later version.
