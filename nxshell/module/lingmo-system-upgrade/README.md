# Lingmo System Upgrade

## Background

This is a tool for system upgrade. Compared with the traditional apt upgrade method, we use the root directory replacement method.

## Dependencies

### Build dependencies 

* cmake

* golang-any

* golang-yaml.v2-dev

* golang-dbus-dev

* golang-github-lingmoos-go-lib-dev
* libdtkwidget-dev

* libdtkgui-dev

* qtbase5-dev

* qttools5-dev-tools



## Runtime dependencies

* [lingmo-upgrade-manager](https://github.com/lingmoos/lingmo-upgrade-manager)

* plymouth-label

* fuseiso

* dpkg-repack

* squashfs-tools

  

## Installation

1.Make sure you have installed all dependencies.

2.Build:

```
$ cd lingmo-system-upgrade
$ mkdir build
$ cd build
$ cmake ..
$ make
```

3.Install:

```
$ sudo make install
```



## Usage

After starting the upgrade tool, the UI interface will appear, and the upgrade can be completed according to the prompts.

## Maintainers

* [github](https://github.com/l631197874)



## License

Lingmo System Upgrade is licensed under [LGPL-3.0-or-later](LICENSE).

