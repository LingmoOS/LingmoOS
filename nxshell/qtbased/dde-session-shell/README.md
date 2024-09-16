# DDE Session Shell

DDE session shell provides two applications: dde-lock and lightdm-deepin-greeter. dde-lock provides screen lock function, and lightdm-deepin-greeter provides login function.

## Dependencies
You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies
You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.
```bash
sudo apt build-dep dde-session-shell
```

2. Build
```bash
mkdir build
cd build
cmake ..
make
```

3. Install
```bash
sudo make install
```

4. debian/rules

```makefile
override_dh_auto_configure:
	dh_auto_configure -- \
		-DWAIT_DEEPIN_ACCOUNTS_SERVICE=1
```
lightdm-deepin-greeter strongly relies on the com.deepin.daemon.Accounts service(created by dde-system-daemon). We use WAIT_DEEPIN_ACCOUNTS_SERVICE to control whether to wait for the service to start when the program starts. If dde-system-daemon will not be started in the desktop environment, please set this value to 0, otherwise the deepin-lightdm-greeter will get stuck.

## Getting help

Any usage issues can ask for help via

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center]((https://github.com/linuxdeepin/developer-center/issues))

## Getting involved

We encourage you to report issues and contribute changes

- [**Contribution guide for developers**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License
DDE session shell is licensed under [GPL-3.0-or-later](LICENSE).
