# Insider Program

System integration for Insider Program, mainly provides plugin for Control Center to provide a GUI to toggle features or components which are already in "Tech Preview" stage.

## Dependencies

Check `debian/control` for build-time and runtime dependencies, or use `cmake` to check the missing required dependencies.

## Building

Regular CMake building steps applies, in short:

```shell
$ mkdir build && cd build
$ cmake ..
$ cmake --build .
```

After building, a `libdcc-insider-plugin.so` binary can be found inside the build folder. And you can optionally install it by:

```shell
$ cmake --build . --target install # only do this if you know what you are doing
```

This plugin can also be tested without installing by using the `--spec` argument of `dde-control-center`, you might need to add a soft link of `libdcc-update-plugin.so` from the system to the build folder since this is a "sub-page" plugin that will be shown as a part of the Control Center's "Update" module.

A `debian` folder is provided to build the package under the *deepin* linux desktop distribution. To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

## Getting Involved

- [Code contribution via GitHub](https://github.com/linuxdeepin/dcc-insider-plugin/)
- [Submit bug or suggestions to GitHub Issues or GitHub Discussions](https://github.com/linuxdeepin/developer-center/issues/new/choose)

## License

**dcc-insider-plugin** is licensed under [GPL-3.0-or-later](LICENSE).

