## DDE Daemon

DDE Daemon是一个用于处理深度会话设置的守护进程

### 依赖


### 编译依赖

* [dde-api](https://github.com/linuxdeepin/dde-api)
* [startdde](https://github.com/linuxdeepin/startdde)
* libudev
* fontconfig
* libbamf3
* pam
* libnl-3.0
* libnl-genl-3.0
* librsvg
* libfprint

### 运行依赖

* upower
* udisks2
* systemd
* pulseaudio
* network-manager
* policykit-1-gnome
* grub-themes-deepin
* gnome-keyring
* deepin-notifications
* xserver-xorg-input-wacom
* libinput
* xdotool
* fontconfig
* pam
* libnl-3.0
* libnl-genl-3.0
* libfprint
* dnsmasq (for hotspot)

### 可选依赖项

* network-manager-vpnc-gnome
* network-manager-pptp-gnome
* network-manager-l2tp-gnome
* network-manager-strongswan-gnome
* network-manager-openvpn-gnome
* network-manager-openconnect-gnome
* iso-codes
* mobile-broadband-provider-info
* xserver-xorg-input-synaptics (provide mode features, such as disable touchpad when typing ...)
* [miraclecast](https://github.com/derekdai/miraclecast) (provide WIFI Direct)
* bluez
* fprintd

## 安装

dde-daemon需要预安装以下包

```shell
$ go get github.com/axgle/mahonia
$ go get github.com/msteinert/pam
```

构建:
```
$ make GOPATH=/usr/share/gocode
```

通过gccgo构建
```
$ make GOPATH=/usr/share/gocode USE_GCCGO=1
```

安装:
```
sudo make install
```

## 使用方法

### dde-system-daemon

`dde-system-daemon`主要提供账号服务，需要以root身份运行。

### dde-session-daemon

#### 标识:

```
memprof      : Write memory profile to specific file
cpuprof      : Write cpu profile to specific file, can not use memprof and
               cpuprof together
-i --Ignore  : Ignore missing modules, --no-ignore to revert it, default is true
-v --verbose : Show much more message, the shorthand for --loglevel debug,
               if specificed, loglevel is ignored
-l --loglevel: Set log level, possible value is error/warn/info/debug/no
```

#### 命令:

```
list   : List all the modules or the dependencies of one module.
auto   : Automatically get enabled and disabled modules from settings.
enable : Enable modules and their dependencies, ignore settings.
disable: Disable modules, ignore settings.
```

## 获得帮助

如果您遇到任何其他问题，您可能还会发现这些渠道很有用：

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org/)
* [WiKi](https://wiki.deepin.org/)

## 贡献指南

我们鼓励您报告问题并做出更改。

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## 开源协议

dde-daemon项目在[GPL-3.0-or-later](LICENSE)开源协议下发布。
