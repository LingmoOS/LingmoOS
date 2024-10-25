# Deb Installer

LingmoOS的软件包安装程序。

## 依赖包

Debian/Ubuntu 依赖包:
```shell
sudo apt install cmake debhelper qtbase5-dev qtdeclarative5-dev qtquickcontrols2-5-dev libqapt-dev libapt-pkg-dev
```

## 构建

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
sudo make install
```

## License

This project has been licensed by GPLv3.
