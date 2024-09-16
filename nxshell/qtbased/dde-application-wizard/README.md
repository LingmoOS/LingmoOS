# DDE Application Wizard

Currently provide the d-bus interface to uninstall application (as a compat d-bus API). Intended to be a module to manage applications and system components on the current OS.

## Dependencies

Check `debian/control` for build-time and runtime dependencies, or use `cmake` to check the missing required dependencies.

## Building

Regular CMake building steps applies, in short:

```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

After building, a `dde-application-wizard-daemon-compat` binary can be found inside the build folder. And you can optionally install it by:

```shell
$ cmake --build . --target install # only do this if you know what you are doing
```

A `debian` folder is provided to build the package under the *deepin* linux desktop distribution. To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

## Getting Involved

- [Code contribution via GitHub](https://github.com/linuxdeepin/dde-application-wizard/)
- [Submit bug or suggestions to GitHub Issues or GitHub Discussions](https://github.com/linuxdeepin/developer-center/issues/new/choose)

## License

**dde-application-wizard** is licensed under [GPL-3.0-or-later](LICENSE).