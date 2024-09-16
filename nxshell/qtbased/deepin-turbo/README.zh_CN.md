# deepin-turbo

deepin-turbo 是一个源自Applauncherd的深度项目。

Applauncherd是一个守护进程，通过预加载动态链接库和缓存内容，帮助更快地启动应用程序。它还节省了内存，因为所有启动的应用程序都共享某些资源。

我们在这个项目中构建了一个名为dtkwidget booster的booster类型，以支持更快地启动基于dtk的应用程序。

## 构建过程

```bash
    mkdir build
    cd build
    cmake ..
    make -j`nproc`
```

## 贡献者

为MapPlauncerd做出贡献的人：

Robin Burchell
John Brooks
Thomas Perl

为meegotouch applauncherd做出贡献的人：

Olli Leppänen
Jussi Lind
Juha Lintula
Pertti Kellomäki
Antti Kervinen
Nimika Keshri
Alexey Shilov
Oskari Timperi

代码的某些部分基于maemo启动器：

Guillem Jover
Michael Natterer

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
deepin-turbo 在 [GPL-3.0-or-later](LICENSE) 下发布。
