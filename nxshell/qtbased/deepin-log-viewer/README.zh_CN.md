# deepin-log-viewer

日志收集工具是一款提供系统日志查看的软件工具。

## 依赖
界面基于 [DTK](https://github.com/linuxdeepin/dtkwidget), Qt (Supported >= 5.12).

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

* debhelper (>= 11)
* pkg-config
* qt5-qmake
* qt5-default
* libdtkwidget-dev
* libdtkgui-dev
* qtbase5-private-dev
* qttools5-dev-tools
* libqt5svg5-dev
* libsystemd-dev
* libicu-dev

### 三方库 (源码)
- [DocxFactory](https://github.com/DocxFactory/DocxFactory)
- [libxlsxwriter](https://github.com/jmcnamara/libxlsxwriter)
- [md5](http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5)
- [minizip](http://www.winimage.com/zLibDll/minizip.html)
- [tmpfileplus](https://www.di-mgt.com.au/c_function_to_create_temp_file.html)

## 安装

1. 确保已安装所有依赖库.

_不同发行版的软件包名称可能不同，如果您的发行版提供了deepin-log-viewer，请检查发行版提供的打包脚本。_

如果你使用的是 [Deepin](https://distrowatch.com/table.php?distribution=deepin) 或者其它提供了deepin-log-viewer的发行版:

``` shell
$ sudo apt-get build-dep deepin-log-viewer
```

2. 构建.
```
cd deepin-log-viewer
mkdir build
cd build
cmake ..
make
```

3. 安装.

```
$ sudo make install
```

## 使用
注意: deepin-log-viewer 依赖 polkit, 如果你从源码构建, 需要添加 policy 文件到 usr/share/polkit-1/actions/. 
添加 com.deepin.pkexec.logViewerAuth.policy 文件到 /usr/bin.
如果在 Qt Creator 中运行，你应该将 [Project][Run Settings] [Run] [Run configuration] 设置为 2 个项目 authenticateProxy 和 src 中的 src。

- sudo cp application/com.deepin.pkexec.logViewerAuth.policy /usr/share/polkit-1/actions/
- sudo cp build-deepin_log_viewer-unknown-Debug/logViewerAuth/logViewerAuth /usr/bin

- ./deepin-log-viewer

## 帮助

任何使用问题都可以通过以下方式寻求帮助:

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改

* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers)


## 开源许可证

Deepin Log Viewer 在 [GPL-3.0-or-later](LICENSE.txt) 下发布.


