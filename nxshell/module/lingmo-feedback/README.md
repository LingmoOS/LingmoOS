# Lingmo Feedback

**Description**: Easy to use issue or advice report tool for Lingmo.

Lingmo Feedback will collect as much related information as possible
according to the user specified category, include machine model,
kernel version, syslog, configurations, etc. Then upload the report to
Lingmo's issue tracking site and that's it.

Lingmo Feedback also provide a command line tool named
`lingmo-feedback-cli`, which in fact used as the information
collecting back-end. For advanced usage, run it under the terminal.

## Dependencies

### Build dependencies

- [ocean-go-dbus-factory](https://github.com/lingmoos/dbus-factory)
- [lingmo-gettext-tools](https://github.com/lingmoos/lingmo-gettext-tools)
- golang
- [golang-dlib](https://github.com/lingmoos/go-lib)
- qt5-default
- qt5-qmake

### Runtime dependencies

- Qt5Quick
- Qt5webkit
- ocean-qml-dbus-factory
- [lingmo-qml-widgets](https://github.com/lingmoos/lingmo-qml-widgets)
- gzip
- lsb-release
- net-tools
- pciutils
- pulseaudio
- qml-module-qtquick-dialogs
- rfkill
- usbutils
- x11
- xorg-xrandr
- xtst

### Optional Dependencies

- bluezutils
- boot-info-script
- dmidecode
- dpkg
- efibootmgr
- lshw
- modem-manager
- network-manager
- pacman
- systemd
- upstart
- wireless-tools

## Installation

### Debian 8.0 (jessie)

Install prerequisites
```
$ sudo apt-get install ocean-go-dbus-factory \
                       lingmo-gettext-tools \
                       golang \
                       golang-dlib-dev \
                       qt5-default \
                       qt5-qmake \
                       qtdeclarative5-dev \
                       libqt5webkit5-dev \
                       ocean-qml-dbus-factory \
                       lingmo-qml-widgets \
                       efibootmgr \
                       gzip \
                       lshw \
                       qml-module-qtquick-dialogs \
                       rfkill \
                       wireless-tools \
                       libx11-dev \
                       libxtst-dev
```

Build
```
$ make GOPATH=/usr/share/gocode
```

Or, build through gccgo
```
$ make GOPATH=/usr/share/gocode USE_GCCGO=1
```

If you have isolated testing build environment (say a docker container), you can install it directly
```
$ sudo make install
```

Or, generate package files and install Lingmo Feedback with it
```
$ debuild -uc -us ...
$ sudo dpkg -i ../lingmo-feedback-*deb
```

## Usage

Run Lingmo Feedback with the command below
```
$ lingmo-feedback &
```

Collect all system information manually and save to archive file in
current directory
```
sudo lingmo-feedback-cli
sudo lingmo-feedback-cli all
```

Collect information without personal data
```
sudo lingmo-feedback-cli --username $USER --privacy-mode
```

Only collect network related information
```
sudo lingmo-feedback-cli network
```

Print current network information

```
sudo lingmo-feedback-cli --dump network
```

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](http://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for users](http://wiki.lingmo.org/index.php?title=Contribution_Guidelines_for_Users)
* [Contribution guide for developers](http://wiki.lingmo.org/index.php?title=Contribution_Guidelines_for_Developers).

## License

Lingmo Feedback is licensed under [GPLv3](LICENSE).
