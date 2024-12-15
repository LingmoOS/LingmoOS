# OCEAN Shell

The `ocean-shell` project provides a plugin system that integrates plugins developed based on this plugin system into OCEAN.

## Dependencies

Check `debian/control` for build-time and runtime dependencies, or use `cmake` to check the missing required dependencies.

## Building

Regular CMake building steps applies, in short:

```shell
$ cmake -Bbuild
$ cmake --build build
```

After building, a `ocean-shell` binary can be found inside the build folder, you can run it
by `ocean-shell -t` to start an example. And you can optionally install it by:

```shell
$ cmake --install build # only do this if you know what you are doing
```

A `debian` folder is provided to build the package under the *lingmo* linux desktop distribution. To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

## Getting Involved

- [Code contribution via GitHub](https://github.com/lingmoos/ocean-shell/)
- [Submit bug or suggestions to GitHub Issues or GitHub Discussions](https://github.com/lingmoos/developer-center/issues/new/choose)

## License

**ocean-shell** is licensed under [GPL-3.0-or-later](LICENSE).
