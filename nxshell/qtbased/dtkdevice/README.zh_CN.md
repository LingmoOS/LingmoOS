# dtkdevice

------------
DTK的device模块，该模块有以下子模块：

* dtklsdevice
* dtkinputdevices

## 特性

1. 使用`cmake`进行项目管理。
2. 使用`doxygen`进行文档编写。
3. 使用`gtest`进行单元测试。
4. 使用`reuse`进行开源协议检查。
5. 支持生成单元测试覆盖率报告。
6. 支持`g++`和`clang++`两种编译器。
7. 兼容`Qt6`与`Qt5`。
8. 支持编译为`debian`平台安装包。

## 依赖

### 编译依赖

* Qt >= 5.11
* cmake >= 3.13
* libdtkcore-dev >= 5.6.0.2
* doxygen
* pkg-config

## 编译安装

### 获取源码

```bash
git clone https://github.com/linuxdeepin/dtkdevice
```

### 准备工作（一键安装编译依赖，仅支持debian系系统）

```bash
sudo apt build-dep .
```

### 配置和编译

1. Debug配置：

    ```bash
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr && cmake --build build
    ```

    该配置下，会编译单元测试，用户可以运行

    ```bash
    ./test-coverage.sh
    ```

    获取单元测试报告
2. Release配置：

    ```bash
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr && cmake --build build
    ```

### 安装

```bash
cd <dtkdevice>/build
sudo make install
```

### 打deb包

```bash
cd <dtkdevice>
dpkg-buildpackage -b -us -uc
```

## 项目目录规范

 **目录**           | **描述**
------------------|---------------------------------------------------------
 .github/         | github相关文件，一般用来存储工作流
 .reuse/          | license声明文件，项目中应使用reuse工具进行license声明
 .gitignore       | git过滤列表，用来过滤不需要推送到仓库中的文件
 .clang-format    | 格式化声明文件，使用该文件进行格式化代码，以保持格式统一性
 README.md        | 项目概述文档
 LICENSE          | 许可协议文件，该文件给github这种仓库使用，项目应使用reuse工具，但协议必须统一，一般为LGPL-v3
 CMakeLists.txt   | 项目文件可放置在最外层
 debian/          | debian打包所需文件
 docs/            | 存放文档相关内容，目前作为doxygen生成文档目录
 include/         | 公共头文件目录，所有需要开放的头文件均应放置在此目录
 LINCENSES/       | 许可协议目录，存放该项目所有的许可协议
 misc/            | 存放项目独立的资源或配置文件，如.service .conf .in等
 src/             | 存放源码文件，如 .h .cpp
 tests/           | 单元测试相关代码存放目录
 examples/        | 示例存放目录

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## 参与贡献

我们鼓励您报告问题并作出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)

## 协议

DTK工具包遵循协议 [LGPL-3.0-or-later](LICENSE).
