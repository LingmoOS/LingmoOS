# Lingmo Service Manager

------------
Lingmo 平台上的服务管理程序。

## 依赖

### 编译依赖

* Qt >= 5.11
* cmake >= 3.13
* systemd >= 241.46

## 构建安装

### 从源代码构建

```bash
git clone https://github.com/lingmoos/lingmo-service-manager.git
cd lingmo-service-manager
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build
```

### 安装

```bash
sudo cmake --build build --target install
```

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Telegram group](https://t.me/lingmo)
* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](https://wiki.lingmo.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

Lingmo 服务管理遵循协议 [LGPL-3.0-or-later](LICENSE).

