# dde-qt-dbus-factory

`dde-qt-dbus-factory` 是用来统一存放 `dde` 使用到的自动生成的 `Qt DBus` 代码的仓库，这样 `dde` 开发者就不用在自己的项目中重复创建同样的 `DBus` 代码，或者从其它项目中拷贝文件到自己的项目里面。

生成代码的时候请使用 `qdbusxml2cpp-fix` 这个工具而不是 `Qt` 官方提供的 `qdbusxml2cpp`，这个工具的源码包含在本项目的 `tools` 目录下，之后也会提供相关包直接安装。

具体生成的规则如下：

- 生成代码用的 `xml` 文件要存放到项目的 `xml` 目录下，这样以后需要更新接口的时候，代码审核可以清晰看到接口的变动；
- 文件名统一为相应 `DBus Interface` 的全小写并用 `_` 替换 `.`；
- 类名统一为 `DBus Interface` 的最后一个单词，使用过程中使用命名空间引用（或者自己起别名）。

举个简单的例子：

比如我需要生成 `org.freedesktop.Notifications` 的 `DBus` 代码，那么先要生成 `org.freedesktop.Notifications.xml` 文件，把接口相应的描述信息写入丢到 `xml` 目录下，然后使用命令：
```bash
qdbusxml2cpp -c Notifications -p org_freedesktop_notifications xml/org.freedesktop.Notifications.xml
```
生成相应的代码即可。

**注：** 生成过程中遇到的复杂类型需要自定义类型的，统一放在相应的生成文件内以自包容。

## 依赖
请查看“debian/control”文件中提供的“Depends”。

### 编译依赖
请查看“debian/control”文件中提供的“Build-Depends”。

### 构建过程

```bash
mkdir build
cd build
cmake ..
make
```

## 帮助
任何使用问题都可以通过以下方式寻求帮助：

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [官方论坛](https://bbs.deepin.org)
* [开发者中心](https://github.com/linuxdeepin/developer-center/issues) 

## 贡献指南

我们鼓励您报告问题并做出更改

- [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) 

## 开源许可证
dde-qt-dbus-factory 在 [GPL-3.0-or-later](LICENSE) 下发布。
