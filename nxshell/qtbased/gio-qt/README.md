# gio-qt: Qt wrapper library of Gio

## Description

This is a Qt wrapper library for Gio (or say it's a glib/glibmm wrapper mainly focused on [GIO](https://gitlab.gnome.org/GNOME/glibmm/tree/master/gio) module). This library is designed to be exception-free and avoid Qt application developer do direct access to glib/glibmm (so they can use Gio in a more Qt way).

## Build
### Build depts:

 - cmake `>= 3.12.4`
 - pkg-config (to find and use giomm module)
 - Qt5Core `>= 5.6.3`
 - Qt5Test (if you would like to build tests)
 - giomm-2.4
 - doxygen (to build documentations)

If you are using an older system which are using a lower cmake version and attempt to build this library, please turn off the `BUILD_DOCS` option.

### Build & Install

``` shell
$ cd /path/to/your/source/code
$ mkdir build
$ cd build
$ cmake ../
$ make
$ sudo make install
```

It will installed to `/usr/local/lib` by default. You may also need to do a `sudo ldconfig /usr/local/lib` after install.

## Getting help

[Official Forum](https://bbs.deepin.org/) for generic discussion and help.

[Developer Center](https://github.com/linuxdeepin/developer-center) for BUG report and suggestions.

[Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

[Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) (English)

## License

 gio-qt is licensed under [LGPL-3.0-or-later](LICENSE)
