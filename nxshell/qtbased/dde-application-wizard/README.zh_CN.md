# DDE Application Wizard

此项目当前提供了一个（兼容性质的）d-bus API 供进行应用程序的卸载。此项目旨在在后续为当前系统提供应用程序与其相关组件的管理与卸载功能。

## 依赖

查看 `debian/control` 文件来了解此项目的构建与运行时依赖，或者使用 `cmake` 检查缺失的必要组件。

## 构建

常规的 CMake 构建步骤，即：

```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

构建完毕后，将可在构建目录内获得名为 `dde-application-wizard-daemon-compat` 的二进制文件。此时你也可以通过如下方式进行安装：

```shell
$ cmake --build . --target install # 当你知道这条命令的作用时再执行它
```

为在 *deepin* 桌面发行版进行此软件包的构建，我们还提供了一个 `debian` 目录。若要构建软件包，可参照下面的命令进行构建：

```shell
$ sudo apt build-dep . # 安装构建依赖
$ dpkg-buildpackage -uc -us -nc -b # 构建二进制软件包
```

## 参与贡献

- [通过 GitHub 发起代码贡献](https://github.com/linuxdeepin/dde-launchpad/)
- [通过 GitHub Issues 与 GitHub Discussions 汇报缺陷与反馈建议](https://github.com/linuxdeepin/developer-center/issues/new/choose)

## 许可协议

**dde-application-wizard** 使用 [GPL-3.0-or-later](LICENSE) 许可协议进行发布。