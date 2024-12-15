# OCEAN Wallpapers

wallpapers for Debian OCEAN

## What's the difference with lingmo-wallpapers

lingmo-wallpapers contains many non free copyright pictures.

Debian won't accept these pictures to the repo of Debian main area,

so this project is created to replace deeppin-walllapers on Debian.

## Dependencies
You can also check the "Depends" provided in the `debian/control` file.

### Build dependencies
You can also check the "Build-Depends" provided in the `debian/control` file.

## Installation

### Build from source code

1. Make sure you have installed all dependencies.
```bash
sudo apt build-dep ocean-wallpapers
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
## Getting help

Any usage issues can ask for help via

* [Matrix](https://matrix.to/#/#lingmo-community:matrix.org)
* [WiKi](https://wiki.lingmo.org)
* [Forum](https://bbs.lingmo.org)
* [Developer Center](https://github.com/lingmoos/developer-center/issues) 

## Getting involved

We encourage you to report issues and contribute changes

- [**Contribution guide for developers**](https://github.com/lingmoos/developer-center/wiki/Contribution-Guidelines-for-Developers-en) 

## License
ocean-wallpapers is licensed under [MIT](LICENSE).