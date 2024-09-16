# DISOMaster

DISOMaster provides basic optical drive operation and on-disc filesystem manipulation.

### Dependencies

  * Qt 5.7
  * libisoburn 1.2.6
  * Reasonable new C++ compiler with C++11 support

### Build dependencies

  * debhelper (>= 9)
  * pkg-config
  * qtbase5-dev
  * qt5-qmake
  * libisoburn-dev

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

``` shell
$ apt build-dep disomaster
```

2. Build:
```
$ cd disomaster
$ mkdir Build
$ cd Build
$ qmake ../disomaster.pro
$ make
```

3. Install:
```
$ sudo make install
```

The executable binary file could be found at `disomaster` .


## Getting help

 - [Official Forum](https://bbs.deepin.org/) for generic discussion and help.
 - [Developer Center](https://github.com/linuxdeepin/developer-center) for BUG report and suggestions.
 - [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

 - [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License

disomaster is licensed under [ GPL-3.0-or-later](LICENSE.txt)
