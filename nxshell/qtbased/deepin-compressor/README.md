## deepin-compressor
Archive Manager is a fast and lightweight application for creating and extracting archives.

归档管理器是一款提供对文件解压、压缩常用功能的软件工具。

#### Core
The GUI is based on [DTK](https://github.com/linuxdeepin/dtkwidget), Qt (Supported >= 5.12).

Build-Depends:
debhelper (>= 11), cmake, libgsettings-qt-dev, libarchive-dev, libsecret-1-dev, libpoppler-cpp-dev, libudisks2-qt5-dev, libdisomaster-dev, libkf5codecs-dev, libzip-dev, qttools5-dev-tools, deepin-gettext-tools, qtbase5-dev, qtchooser (>= 55-gc9562a1-1~), qt5-qmake, libdtkwidget-dev,libqt5svg5-dev, libqt5x11extras5-dev, libkf5archive-dev, libminizip-dev, qttools5-dev, libgtest-dev

Execute Depends:
${shlibs:Depends}, ${misc:Depends}, p7zip-full, deepin-shortcut-viewer, unar[i386 amd64], rar[i386 amd64], unrar[i386 amd64]

### Third Party Code
[KDE ark](https://github.com/kde/ark)

#### Installation
sudo apt-get install deepin-compressor

#### Build
- mkdir build
- cd build
- cmake ..
- make
- make install

Caution:

## [Wiki](http://gitlab01.archermind.com/amt_SY/compressor/wikis/home)

## License
deepin-compressor is licensed under [GPL-3.0-or-later](LICENSE).
