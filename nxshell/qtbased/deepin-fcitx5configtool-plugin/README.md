# deepin-fcitx5configtool-plugin 

The input method management plug-in of DDE control center has been adapted based on fcitx5 input method framework.

#### Installation

```
sudo apt install deepin-fcitx5configtool-plugin 
```

#### Build dependencies

```
sudo apt build-dep deepin-fcitx5configtool-plugin 
```

```
sudo apt install  debhelper qt5-qmake libqt5widgets5 libqt5network5 libdtkwidget-dev libdtkgui-dev libdtkgui5 libdtkgui5-bin libdtkcore5-bin libdtkcore-dev qttools5-dev qttools5-dev-tools pkg-config cmake fcitx-bin fcitx-libs-dev libfcitx-qt5-1 libfcitx-qt5-data libfcitx-qt5-dev iso-codes libdbus-glib-1-dev libgtk-3-dev libgtk2.0-dev libunique-dev libdframeworkdbus-dev libgtest-dev libgmock-dev dde-control-center-dev extra-cmake-modules libkf5i18n-dev
```

#### Build from source

git clone https://github.com/linuxdeepin/deepin-fcitx5configtool-plugin.git

sudo apt build-dep deepin-fcitx5configtool-plugin 

cd ./deepin-fcitx5configtool-plugin 

sudo dpkg-buildpackage -b

cd ..

sudo dpkg -i *.deb

#### Contributors

1.  cssplay
2.  liuwenhao
3.  chenshijie
4.  zhaoyue


## License

This project is released under GPL-3.0-or-later
