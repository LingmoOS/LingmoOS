# Launcher

LingmoOS's full-screen application launcher.

[![Build](https://github.com/LingmoOS/launcher/actions/workflows/build.yml/badge.svg)](https://github.com/LingmoOS/launcher/actions/workflows/build.yml)

## Dependencies
Arch Linux/Manjaro Dependencies
```shell
sudo pacman -S gcc cmake qt5-base qt5-quickcontrols2 kwindowsystem
```
Debian/Ubuntu Dependencies
```shell
sudo apt install gcc cmake qtbase5-dev qml-module-qtquick-controls2 qml-module-org-kde-kwindowsystem qtdeclarative5-dev qtquickcontrols2-5-dev qttools5-dev libkf5windowsystem-dev
```

## Build and Install

```
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## License

This project has been licensed by GPLv3.
