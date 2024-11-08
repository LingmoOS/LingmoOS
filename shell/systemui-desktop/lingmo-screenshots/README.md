# Screenshot

Screenshot tool for LingmoOS.

## Dependencies

Arch / Manjaro Dependencies:

```shell
sudo pacman -S extra-cmake-modules qt5-base qt5-quickcontrols2
```

Debian / Ubuntu Dependencies:
```shell
sudo apt install cmake qtbase5-dev qtdeclarative5-dev qtquickcontrols2-5-dev qttools5-dev qttools5-dev-tools qml-module-qtquick-controls2 qml-module-qtquick2 qml-module-qtquick-layouts qml-module-qt-labs-platform qml-module-qt-labs-settings qml-module-qtqml qml-module-qtquick-window2 qml-module-qtquick-shapes qml-module-qtquick-dialogs qml-module-qtquick-particles2
```

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## License

This project has been licensed by GPLv3.
