# DDE Polkit Agent

DDE Polkit Agent is the [polkit](https://www.freedesktop.org/software/polkit/docs/latest/polkit.8.html) agent used in Deepin Desktop Environment.

## Dependencies
You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies
You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.
```bash
sudo apt build-dep dde-polkit-agent
```

2. Build
```bash
mkdir build
cd build
qmake ..
make
```

3. Install
```bash
sudo make install
```

## Usage

DDE Polkit Agent works as an standard polkit agent, see https://www.freedesktop.org/software/polkit/docs/latest/polkit.8.html for 
more about how to use polkit.

## Getting help

Any usage issues can ask for help via

* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [WiKi](https://wiki.deepin.org)
* [Forum](https://bbs.deepin.org)
* [Developer Center](https://github.com/linuxdeepin/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

- [**Contribution guide for developers**](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License
DDE Polkit Agent is licensed under [GPL-3.0-or-later](LICENSE).
