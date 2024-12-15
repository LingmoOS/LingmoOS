# lingmo-log-viewer
Lingmo log viewer is a fast and lightweight application for viewing system log.

## Dependencies
The GUI is based on [DTK](https://github.com/lingmoos/dtkwidget), Qt (Supported >= 5.12).

_The **master** branch is current development branch, build dependencies may changes without update README.md, refer to `./debian/control` for a working build depends list_

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

### Thridparty lib (source code)
- [DocxFactory](https://github.com/DocxFactory/DocxFactory)
- [libxlsxwriter](https://github.com/jmcnamara/libxlsxwriter)
- [md5](http://openwall.info/wiki/people/solar/software/public-domain-source-code/md5)
- [minizip](http://www.winimage.com/zLibDll/minizip.html)
- [tmpfileplus](https://www.di-mgt.com.au/c_function_to_create_temp_file.html)

## Installation

1. Make sure you have installed all dependencies.

_Package name may be different between distros, if lingmo-log-viewer is available from your distro, check the packaging script delivered from your distro is a better idea._

Assume you are using [Lingmo](https://distrowatch.com/table.php?distribution=lingmo) or other debian-based distro which got lingmo-log-viewer delivered:

``` shell
$ sudo apt-get build-dep lingmo-log-viewer
```

2. Build.
```
cd lingmo-log-viewer
mkdir build
cd build
cmake ..
make
```

3. Install.

```
$ sudo make install
```

## Usage
Caution: lingmo-log-viewer is depend on polkit, So if build from source code, 
you have to add policy to usr/share/polkit-1/actions/. 
add com.lingmo.pkexec.logViewerAuth.policy to /usr/bin.
If run in Qt Creator, you should set [Project][Run Settings] [Run] [Run configuration] to src from 2 projects authenticateProxy and src.

- sudo cp application/com.lingmo.pkexec.logViewerAuth.policy /usr/share/polkit-1/actions/
- sudo cp build-lingmo_log_viewer-unknown-Debug/logViewerAuth/logViewerAuth /usr/bin

- ./lingmo-log-viewer

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/lingmoos/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=lingmo)
* [Forum](https://bbs.lingmo.org)
* [WiKi](http://wiki.lingmo.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en). 

## License

Lingmo Log Viewer is licensed under [GPL-3.0-or-later](LICENSE.txt).


