# DDE Shell

dde-shell项目提供一套插件体系，将基于此插件体系开发的的插件整合到dde桌面。

## 依赖

查看 `debian/control` 文件来了解此项目的构建与运行时依赖，或者使用 `cmake` 检查缺失的必要组件。

## 构建

常规的 CMake 构建步骤，即：

```shell
$ cmake -Bbuild
$ cmake --build build
```

构建完毕后，将可在构建目录内获得名为 `dde-shell` 的二进制文件，可以执行`dde-shell -t`来运行内置的示例。此时你也可以通过如下方式进行安装：

```shell
$ cmake --install build # 当你知道这条命令的作用时再执行它
```

为在 *deepin* 桌面发行版进行此软件包的构建，我们还提供了一个 `debian` 目录。若要构建软件包，可参照下面的命令进行构建：

```shell
$ sudo apt build-dep . # 安装构建依赖
$ dpkg-buildpackage -uc -us -nc -b # 构建二进制软件包
```

## 参与贡献

- [通过 GitHub 发起代码贡献](https://github.com/linuxdeepin/dde-shell/)
- [通过 GitHub Issues 与 GitHub Discussions 汇报缺陷与反馈建议](https://github.com/linuxdeepin/developer-center/issues/new/choose)

## 许可协议

**dde-shell** 使用 [GPL-3.0-or-later](LICENSE) 许可协议进行发布。