## Deepin Tool Kit Core

Deepin Tool Kit Core(DtkCore) 是所有C++/Qt开发人员在Deepin上工作的基础开发工具.

您应该首先阅读 <a href=docs/Specification.md>Deepin应用程序规范</a>.

## 文档

中文文档：[dtkcore文档](https://linuxdeepin.github.io/dtkcore/index.html)

## 依赖

### 编译依赖

* Qt >= 5.10

## 编译选项

| **编译选项**           | **含义**      | **默认状态**      |
|--------------------|-------------|---------------|
| BUILD_DOCS         | 编译文档        | ON            |
| BUILD_TESTING      | 编译测试        | Debug模式下默认为ON |
| BUILD_EXAMPLES     | 编译示例        | ON            |
| BUILD_WITH_SYSTEMD | 支持Systmed功能 | OFF           |
| BUILD_THEME        | 为文档添加主题     | OFF           |

## 安装

### 从源代码构建

1. 确保已经安装了所有的编译依赖.

2. 构建:

```bash
mkdir build
cd build
cmake ..
make
```

3. 安装:

```bash
sudo make install
```

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Telegram 群组](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

DTK工具包遵循协议 [LGPL-3.0-or-later](LICENSE).
