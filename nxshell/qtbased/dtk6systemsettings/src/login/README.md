## dtklogin

Development Tool Kit Login module, used for login user, session and seat etc.

[中文文档](./README.zh_CN.md)

## Dependencies

`systemd: >=241.36-1`  
Testing environment for this project：`systemd 241.36-1` and `systemd 250.2.2-4`

### Build dependencies

Project compile-time dependencies: qtbase5-dev,doxygen,cmake,gcc,g++,qttools5-dev

```bash
sudo apt install qtbase5-dev qttools5-dev doxygen cmake 
```

## Installation

### Build from source code

```bash
sudo apt install qt5-default qttools5-dev doxygen cmake  
cmake -B build
cmake --build build/
cd build
sudo make install
```

## Getting help

- [Official Forum](https://bbs.deepin.org/) for generic discussion and help.
- [Developer Center](https://github.com/linuxdeepin/developer-center) for BUG report and suggestions.
- [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

- [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)

## License

This project is licensed under [LGPL-3.0-or-later](../LICENSE).
