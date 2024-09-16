# Deepin Calculator

Deepin calculator is an easy to use calculator for ordinary users.

## Dependencies

* sudo apt install libdtkwidget-dev libqt5svg5-dev qttools5-dev-tools cmake

## Installation

* mkdir build
* cd build
* cmake .. -DBUILD_TESTING=OFF
* make

## Test

* sudo apt install libgtest-dev googletest libgmock-dev
* mkdir build
* camke ..
* cd tests
* make
* ./deepin-calculator-test
## Usage

* ./deepin-calculator

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

Deepin Calculator is licensed under [GPL-3.0-or-later](LICENSE)
