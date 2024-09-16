# 预览计划

提供了预览计划的系统集成，主要提供一个用于供用户开启与关闭预览特性/组件的图形界面的控制中心插件。

## 依赖

查看 `debian/control` 文件来了解此项目的构建与运行时依赖，或者使用 `cmake` 检查缺失的必要组件。

## 构建

常规的 CMake 构建步骤，即：

```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

构建完毕后，将可在构建目录内获得名为 `libdcc-insider-plugin.so` 的二进制文件。此时你也可以通过如下方式进行安装：

```shell
$ cmake --build . --target install # 当你知道这条命令的作用时再执行它
```

此插件也可在不安装的情况下使用控制中心的 `--spec` 参数来进行测试，注意的是，由于插件是一个位于“更新”组件下的二级菜单插件，故你需要在生成的插件同目录添加一个 `libdcc-update-plugin.so` 的软链接。

为在 *deepin* 桌面发行版进行此软件包的构建，我们还提供了一个 `debian` 目录。若要构建软件包，可参照下面的命令进行构建：

```shell
$ sudo apt build-dep . # 安装构建依赖
$ dpkg-buildpackage -uc -us -nc -b # 构建二进制软件包
```

## 参与贡献

- [通过 GitHub 发起代码贡献](https://github.com/linuxdeepin/dcc-insider-plugin/)
- [通过 GitHub Issues 与 GitHub Discussions 汇报缺陷与反馈建议](https://github.com/linuxdeepin/developer-center/issues/new/choose)

## 许可协议

**dcc-insider-plugin** 使用 [GPL-3.0-or-later](LICENSE) 许可协议进行发布。
