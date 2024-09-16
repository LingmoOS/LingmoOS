# DDM

The `ddm` project is Display Manager base `SDDM` fork.

## Dependencies

Check `debian/control` for build-time and runtime dependencies, or use `cmake` to check the missing required dependencies.

## Building

Regular CMake building steps applies, in short:

```shell
$ cmake -Bbuild
$ cmake --build build
$ cmake --install build # only do this if you know what you are doing
```

A `debian` folder is provided to build the package under the *deepin* linux desktop distribution. To build the package, use the following command:

```shell
$ sudo apt build-dep . # install build dependencies
$ dpkg-buildpackage -uc -us -nc -b # build binary package(s)
```

## Getting Involved

- [Code contribution via GitHub](https://github.com/linuxdeepin/ddm/)
- [Submit bug or suggestions to GitHub Issues or GitHub Discussions](https://github.com/linuxdeepin/developer-center/issues/new/choose)

## License

**ddm** is licensed under [GPL-2.0](LICENSE).
