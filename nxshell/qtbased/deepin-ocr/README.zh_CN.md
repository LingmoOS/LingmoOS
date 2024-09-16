# Deepin OCR

Deepint Ocr 提供基础文字识别功能.

您应该首先阅读 [Deepin应用程序规范](\ref doc/Specification).

## 依赖

### 编译依赖

编译依赖可以通过查看 debian/control 获取

## 安装

### 从源代码构建

1. 确保已经安装了所有的编译依赖.
````bash
sudo apt build-dep deepin-ocr
````
如果需要使用qtcreator的`设计`功能，需要安装以下依赖:
```bash
sudo apt install qttools5-dev
```
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

## 用法

命令行执行 `deepin-ocr`

## 文档

 - 无

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

Deepin OCR 遵循协议 [GPL-3.0-or-later](LICENSE).
