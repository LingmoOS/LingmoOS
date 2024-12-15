## Lingmo Icon Theme

Lingmo Icon Theme 提供各种Lingmo风格的基础图标主题.

您应该首先阅读 [Lingmo应用程序规范](\ref doc/Specification).

## 依赖

### 编译依赖

* Qt >= 5.6

## 安装

### 从源代码构建

1. 确保已经安装了所有的编译依赖.
````bash
sudo apt build-dep lingmo-icon-theme
````
如果需要使用qtcreator的`设计`功能，需要安装以下依赖:
```bash
sudo apt install qttools5-dev
```
2. 构建:

```bash
make
```

3. 安装:

```bash
sudo make install
```

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

DTK工具包遵循协议 [GPLv3](LICENSE).
