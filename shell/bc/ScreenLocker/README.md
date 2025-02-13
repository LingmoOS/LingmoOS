# Screen locker

## Third Party Code

kcheckpass

## Dependencies

### Debian/Ubuntu

```
sudo apt install libpam0g-dev libx11-dev -y
```

## Build

```shell
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..
make
```

## Install

```shell
sudo make install
```

## License

This project has been licensed by GPLv3.
