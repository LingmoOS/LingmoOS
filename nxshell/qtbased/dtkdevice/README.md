# dtkdevice

------------
Development Tool Kit device module, including submodules:

* dtklsdevice
* dtkinputdevices

[中文版本](README.zh_CN.md)

## Features

1. Use `cmake` for project management.
2. Use `doxygen` for documentation.
3. Use `gtest` for unit testing.
4. Use `reuse` for open source protocol checking.
5. Support generating unit test coverage reports.
6. Both `g++` and `clang++` compilers are supported.
7. Compatible with `Qt6` and `Qt5`.
8. Support for compiling as a `debian` platform installer.

## Dependencies

### Build dependencies

* Qt >= 5.11
* cmake >= 3.13
* libdtkcore-dev >= 5.6.0.2
* doxygen
* pkg-config

## Build and install

### Get source

```bash
git clone https://github.com/linuxdeepin/dtkdevice
```

### Preparations(install dependencies)

```bash
sudo apt build-dep .
```

### Configure and build

1. Debug config

    ```bash
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=/usr && cmake --build build
    ```

    Notice, under debug config, we can run

    ```bash
    ./test-coverage.sh
    ```

    to acquire test coverage info.
2. Release config

    ```bash
    cmake -Bbuild -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr && cmake --build build
    ```

### Install

```bash
cd <dtkdevice>/build
sudo make install
```

### Make deb package

```bash
cd <dtkdevice>
dpkg-buildpackage -b -us -uc
```

## Catalog Specification

 **Catalog**           | **Description**
------------------|---------------------------------------------------------
 .github/         | github related files, generally used to store workflows
 .reuse/          | license declaration file, the project should use the reuse tool for license declaration
 .gitignore       | The git filter list, used to filter files that don't need to be pushed to the repository
 .clang-format    | Formatting declaration file, which is used for formatting code to maintain formatting uniformity
 README.md        | Project Overview Document
 LICENSE          | License agreement file, the file to github such repository use, the project should use reuse tool, but the agreement must be unified, generally LGPL-v3
 CMakeLists.txt   | Project files can be placed in the outermost
 debian/          | Required files for debian packaging
 docs/            | Store document-related content, currently used as a directory for doxygen-generated documents
 include/         | Public headers directory, all open headers should be placed in this directory
 LINCENSES/       | License agreement directory, where all license agreements for the project are stored
 misc/            | Store project-independent resources or configuration files, such as .service .conf .in, etc.
 src/             | Store source code files, such as: .h .cpp
 tests/           | Unit test related code storage directory
 examples/        | Examples storage directory

## Getting help

Any usage issues can ask for help via

* [Telegram group](https://t.me/deepin)
* [Matrix](https://matrix.to/#/#deepin-community:matrix.org)
* [IRC (libera.chat)](https://web.libera.chat/#deepin-community)
* [Forum](https://bbs.deepin.org)
* [WiKi](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en).

## License

Development Tool Kit is licensed under [LGPL-3.0-or-later](LICENSE).
