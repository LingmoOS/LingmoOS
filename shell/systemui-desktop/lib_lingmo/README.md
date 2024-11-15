# System Library

## Dependencies

```shell
sudo pacman -S gcc cmake qt5-base qt5-quickcontrols2 networkmanager-qt modemmanager-qt
```

Note that when using apt, the names of certain packages are different:
```shell
sudo apt install qtbase5-dev qtquickcontrols2-5-dev modemmanager-qt-dev libqt5sensors5-dev libkf5networkmanagerqt-dev libkf5screen-dev libkf5bluezqt-dev libkf5kio-dev cmake qtdeclarative5-dev libcanberra-dev libpulse-dev libcanberra-pulse extra-cmake-modules qttools5-dev qttools5-dev-tools
```

## Build and Install

```
mkdir build
cd build
cmake ..
make
sudo make install
```

## License

This project has been licensed by GPLv3.
