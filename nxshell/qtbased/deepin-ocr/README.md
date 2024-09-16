# Deepin OCR

Deepin OCR provides the base character recognition ability on Deepin.

## Dependencies

### Build dependencies

See debian/control

### Build from source code

1. Make sure you have installed all dependencies.
````
sudo apt build-dep deepin-ocr
````
If you need to use the designer plugin, you should also install:
````
sudo apt install qttools5-dev
````
2. Build:

````
$ mkdir build
$ cd build
$ cmake ..
$ make
````

3. Install:

````
$ sudo make install
````

## Getting help

Any usage issues can ask for help via

* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC channel](https://webchat.freenode.net/?channels=deepin)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)

## License

Deepin OCR is licensed under [GPL-3.0-or-later](LICENSE).
