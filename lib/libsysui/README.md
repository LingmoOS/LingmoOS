# Lingmo UI

LingmoUI is a GUI library based on QQC (Qt Quick Controls) and Qt 6, every Lingmo application uses it.

## Features

* Light and Dark Mode
* Borderless window (Wayland & XCB Window move & resize)
* Blurred window
* Window shadow
* Desktop-level menu
* QQC Style
* ...

## Structures

- `Compatible`: This folder stores the old version of LingmoUI 1.0. (Will be removed in the future)
- `LingmoStyle`: This folder stores QMLs for Qt Quick Controls 2 style.
- `src`: This folder stores the source code of LingmoUI.
  - `Controls`: This folder stores the main QMLs of LingmoUI.
  - `Image`: This folder stores the images of LingmoUI.
  - `Font`: This folder stores the fonts of LingmoUI.
  - `GraphicalEffects`: This folder stores the graphical effects implementation of LingmoUI.
- `example`: This folder stores the example app using LingmoUI.

## Documentation

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/LingmoOS/LingmoUI) (Auto generated docs, thanks for DeepWiki!)

API documentation: [Click here](https://lingmoos.github.io/LingmoUI/)

## Dependencies

### Debian

```bash
sudo apt install devscripts equivs
sudo mk-build-deps -i -t "apt --yes" -r
```

### Windows

1. Install Qt 6.7 or higher, and set the environment variables.
2. Install ECM (Extra CMake Modules) from [here](https://invent.kde.org/frameworks/extra-cmake-modules). Build and install it to a proper directory. Set the environment variables CMAKE_PREFIX_PATH for ECM.
3. Clone this repository.
4. Open the Qt Creator and open the `CMakeLists.txt` file.
5. Build and run the example app.

## Build
Before build, make sure you have necessary Qt environment.

### Using Command Line
```bash
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

### Using Qt Creator

Open the `CMakeLists.txt` file in Qt Creator and build the project.

## Packaging

### Debian/Ubuntu

Install compile dependencies:

```bash
$ sudo apt install equivs devscripts --no-install-recommends
$ sudo mk-build-deps -i -t "apt --yes" -r
```

Start packing

```bash
$ dpkg-buildpackage -b -uc -us -tc
```

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for more information.

For maintainers, use `conventional-changelog -p angular -i CHANGELOG.md -w` to generate the changelog.

## License

LingmoUI is licensed under GPLv3.
