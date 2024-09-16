# Deepin Editor

Deepin Editor is a desktop text editor that supports common text editing features.

## Dependencies

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

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

## Installation

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if deepin-editor is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Deepin](https://distrowatch.com/table.php?distribution=deepin) or other debian-based distro which got deepin-editor delivered:

``` shell
$ sudo apt-get build-dep deepin-edtor
```

2. Build editor daemon

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

3. Build editor

```
cd deepin-editor
mkdir build
cd build
cmake ..
make
./deepin-editor
```

4. Install editor:

```
$ sudo make install
```

## Config file

configure save at: ~/.config/deepin/deepin-editor/config.conf

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). 

## License

Deepin Editor is licensed under [GPL-3.0-or-later](LICENSE.txt).
