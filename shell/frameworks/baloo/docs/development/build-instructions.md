# Build Instructions (Linux) {#build-instructions}

## Prerequisites

* A C++ compiler
* Qt 5.x
* KDE Frameworks - I18n, Config, IdleTime, Auth, Crash, Solid, KIO, DBusAddons, KFileMetaData

On Arch Linux, install the following libraries:

```bash
$ sudo pacman -S ki18n kconfig kidletime kauth kcrash solid kio kdbusaddons kfilemetadata
```

FIXME: What about basic requirements such as gcc and cmake?

## Getting the code

```bash
$ git clone git://anongit.kde.org/baloo
```

## Setting up the development environment

Baloo installs a number of executables and libraries. While these can be installed in `/usr/`, that requires root access and typically interfers with the distro packages. Instead we prefer installing Baloo locally, and updating the required environment variables. This isolates environment also protects us from mistakes.

Python development typically also uses an isolated environment called a virtual environment. For now we can just use their tool.

```bash
$ sudo pacman -S python-virtualenv
$ mkdir ~/baloo-env
$ cd ~/baloo-env
$ git clone git://anongit.kde.org/baloo
$ mkdir build
$ cd build
$ cmake baloo -DCMAKE_INSTALL_PREFIX=~/baloo-env
$ make -j4
$ make install
```

This builds and installs Baloo in the virtual environment.

This is stupid, the LD_LIBRARY_PATH is not set by virtualenv. We need a better tool. Write my own?

 Setting up the environment
 Compilation and installation
 Running automated tests
