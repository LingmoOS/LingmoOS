## OCEAN Daemon

OCEAN Daemon is a daemon for handling  the lingmo session settings

### Dependencies


### Build dependencies

* [ocean-api](https://github.com/lingmoos/ocean-api)
* [startocean](https://github.com/lingmoos/startocean)
* libudev
* fontconfig
* libbamf3
* pam
* libnl-3.0
* libnl-genl-3.0
* librsvg
* libfprint

### Runtime dependencies

* upower
* udisks2
* systemd
* pulseaudio
* network-manager
* policykit-1-gnome
* grub-themes-lingmo
* gnome-keyring
* lingmo-notifications
* xserver-xorg-input-wacom
* libinput
* xdotool
* fontconfig
* pam
* libnl-3.0
* libnl-genl-3.0
* libfprint
* dnsmasq (for hotspot)

### Optional Dependencies

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

## Installation


Install prerequisites

```shell
$ go get github.com/axgle/mahonia
$ go get github.com/msteinert/pam
```

Build:
```
$ make GOPATH=/usr/share/gocode
```

Or, build through gccgo
```
$ make GOPATH=/usr/share/gocode USE_GCCGO=1
```

Install:
```
sudo make install
```

## Usage

### ocean-system-daemon

`ocean-system-daemon` primarily provide account services, need to run as root.

### ocean-session-daemon

#### Flags:

```
memprof      : Write memory profile to specific file
cpuprof      : Write cpu profile to specific file, can not use memprof and
               cpuprof together
-i --Ignore  : Ignore missing modules, --no-ignore to revert it, default is true
-v --verbose : Show much more message, the shorthand for --loglevel debug,
               if specificed, loglevel is ignored
-l --loglevel: Set log level, possible value is error/warn/info/debug/no
```

#### Commands:

```
list   : List all the modules or the dependencies of one module.
auto   : Automatically get enabled and disabled modules from settings.
enable : Enable modules and their dependencies, ignore settings.
disable: Disable modules, ignore settings.
```

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org/)
* [WiKi](https://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes.

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

OCEAN Daemon is licensed under [GPL-3.0-or-later](LICENSE).
