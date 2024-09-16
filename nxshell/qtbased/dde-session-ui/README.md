## dde-session-ui
**Description**:This project include those sub-project:

- dde-shutdown
```
User interface of shutdown.
```
- dde-lock
```
User interface of lock screen.
```
- dde-lockservice
```
The back-end service of locking screen.
```
- lightdm-deepin-greeter
```
The user interface when you login in.
```
- dde-switchtogreeter
```
The tools to switch the user to login in.
```
- dde-lowpower
```
The user interface of reminding low power.
```
- dde-osd
```
User interface of on-screen display .
```
- dde-hotzone
```
User interface of setting hot zone.
```

## Dependencies
You can also check the "Depends" provided in the debian/control file.

## Installation
Those project's installation is all the same, so we just make a example.
A example to install lightdm-deepin-greeter:

- you need go to the directory of lightdm-deepin-greeter.pro, firstly;
- create a directory, go to the directory, and run `qmake .. && make`, secondly;
- run `sudo make install`, finally.
- if you want to make your installation work, you may be need to change some configure file
make sure that the lightdm-deepin-greeter is running, or may be it'will run the old one.
In the deepin operation system, you need to copy the file lightdm-deepin-greeter (which you get by run `make`) to the directory "/usr/local/bin".

## Usage
Those tools you get by the sub-project are system tools, you may use lightdm-deepin-greeter when you login, and you may use dde-shutdown
when you choose to shutdown you pc, and so on.

## Getting help
* [github](https://github.com/linuxdeepin/dde-session-ui)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues) 

## Getting involved
We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License
DDE session ui is licensed under [GPL-3.0-or-later](LICENSE).
