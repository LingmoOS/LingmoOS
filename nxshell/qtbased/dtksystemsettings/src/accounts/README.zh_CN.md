## 项目名称

dtkaccounts是对systemd-accounts接口进行封装。

## 编译依赖

### 在debian/uos上构建

软件包依赖:

- qtbase5-dev
- qttools5-dev-tools
- cmake
- doxygen

### 从源代码构建

```bash
sudo apt install qt5-default qttools5-dev doxygen cmake  
cmake -B build
cmake --build build/
cd build
sudo make install
```

# 文档

暂无

## 获取帮助

- 在[官方论坛](https://bbs.deepin.org/)上获取帮助或进行相关讨论
- 在[开发者中心](https://github.com/linuxdeepin/developer-center)上反馈BUG和提出建议
- [Wiki](https://wiki.deepin.org/)

## 参与其中

我们鼓励您报告问题并贡献更改

- [开发者贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (英文)

## 开源许可

本项目采用[LGPL-3.0-or-later](../LICENSE)授权。
