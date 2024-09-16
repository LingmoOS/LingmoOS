# dtkbluetooth

开发工具包: Bluetooth模块，使用Qt对BlueZ DBus API进行封装。

[英文文档](./README.md)

## 依赖

### 运行时依赖

`BlueZ >= 5.50.33`

### 编译依赖

- Qt >= 5.11
- libdtkcore-dev >= 5.6.2 (or build from github source code),
- CMake >= 3.13
- qttools5-dev-tools,
- doxygen,
- qtconnectivity5-dev

## 编译安装

### 从源码编译

```bash
git clone https://github.com/linuxdeepin/dtkbluetooth.git
cd dtkbluetooth
cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

### 安装

```bash
sudo cmake --build build --target install
```

## 获取帮助

- 在[官方论坛](https://bbs.deepin.org/)上获取帮助或进行相关讨论
- 在[开发者中心](https://github.com/linuxdeepin/developer-center)上反馈BUG和提出建议
- [Wiki](https://wiki.deepin.org/)

## 参与其中

我们鼓励您报告问题并贡献更改

- [开发者贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (英文)

## 开源许可

本项目采用[LGPL-3.0-or-later](../LICENSE)授权。
