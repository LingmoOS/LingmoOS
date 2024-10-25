# LingmoOS video player

Open source video player built using Qt/QML and libmpv.

### Third-party code

[haruna](https://github.com/g-fb/haruna)

## Dependencies

```shell
sudo apt install extra-cmake-modules qtbase5-dev qtdeclarative5-dev qtquickcontrols2-5-dev libmpv-dev
```

# Build

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

# License

This project has been licensed by GPLv3.
