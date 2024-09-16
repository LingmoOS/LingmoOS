# Deepin Editor

文本编辑器是一个简单的文本编辑工具。您可以用它书写简单的文本文档，也可以使用它的高级特性，让它成为一个代码编辑工具，支持代码语法高亮。

## 依赖

_**master**分支是当前开发分支，编译依赖可能在未更新README.md文件的情况下变更，请参考./debian/control文件获取有效的编译依赖列表_

* libqt5widgets5 
* libdtkcore-dev 
* libdtkwidget-dev 
* qt5-default 
* libpolkit-qt5-1-dev 
* libkf5syntaxhighlighting-dev 
* libkf5codecs-dev 
* qttools5-dev-tools 
* qtbase5-private-dev 
* libxcb-util0-dev 
* libdtkwm-dev 
* libxtst-dev

## 安装

1. 确保已安装所有依赖库.


_不同发行版的软件包名称可能不同，如果您的发行版提供了deepin-editor，请检查发行版提供的打包脚本。_

如果你使用的是 [Deepin](https://distrowatch.com/table.php?distribution=deepin) 或者其它提供了deepin-editor的发行版:

``` shell
$ sudo apt-get build-dep deepin-edtor
```

2. 构建 editor daemon

```
sudo cp com.deepin.editor.policy /usr/share/polkit-1/actions
sudo cp com.deepin.editor.conf /usr/share/dbus-1/system.d/
sudo cp com.deepin.editor.daemon.service /usr/share/dbus-1/system-services

cd ./daemon
mkdir build
cd build
qmake ..
make
sudo ./deepin-editor-daemon
```

3. 构建 editor

```
cd deepin-editor
mkdir build
cd build
cmake ..
make
./deepin-editor
```

4. 安装 editor:

```
$ sudo make install
```

## 配置文件

配置文件位于: ~/.config/deepin/deepin-editor/config.conf

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

Deepin Editor 在 [GPL-3.0-or-later](LICENSE.txt) 下发布
