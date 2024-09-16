# deepin-voice-note

语音笔记是一个简单的备忘录软件，带有文本和语音记录。您可以将录音保存为MP3格式或文本。

## 编译依赖

Build-Depends:debhelper (>= 11),cmake,qttools5-dev-tools,libutf8proc-dev,qtbase5-dev,pkg-config,libsqlite3-dev,qtbase5-private-dev,libglib2.0-dev,libdtkwidget-dev,libdtkwidget5-bin,libdtkcore5-bin,libdframeworkdbus-dev,libgstreamer1.0-dev,libvlc-dev,libvlccore-dev,libgtest-dev

### 构建过程
- mkdir build
- cd build
- cmake ..
- make
- sudo make install

## 配置文件

* Voice Notes Settings's configure save at:<br/>
~/.config/deepin/deepin-voice-note/config.conf

## 帮助

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](http://wiki.deepin.org/)

## 贡献指南

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源许可证

Deepin Voice Notes 根据[GPL-3.0-or-later](许可证).
