# Lingmo OS

This is the source tree of the LingmoOS desktop environment.

### Introduction
![LingmoOS](desktop/screenshots/screenshot.png)
Lingmo OS is a Linux operating system based on the desktop environment. It is developed based on the open source Linux kernel and is compatible with the amd64 architecture. It has a beautiful desktop environment, software adapted to the desktop environment, and a unified UI style.

### UI, new desktop environment

Lingmo OS adopts a new design that is different from the traditional KDE, making the operation no longer cumbersome; the operation is more user-friendly, making the interaction and vision more dynamic.

# Preparations
## Build Host Requirements

 - Debian GNU/Linux 12(Bookworm), 13(Trixie) or later.
 - Disk free space > 50GB
 - Multi-core multi-threaded processor
 - 8 GB minimum running memory

## Tool preparation

### Clone This Code Warehouse.
   ```
   git clone https://github.com/LingmoOS/LingmoOS.git
   mkdir -p ~/bin
   cp -v LingmoOS/repo ~/bin/
   chmod a+x ~/bin/repo
   echo 'export PATH="$PATH:$HOME/bin"' >> ~/.bashrc
   source ~/.bashrc
```

# Initialize repo warehouse

## Create a directory for building:
```
mkdir -p workdir
cd workdir
```
## Init Repo

```
repo init -u https://github.com/LingmoOS/manifest.git
```

# Synchronization code
```
repo sync
```

# Build Desktop Packages
```
make config #Config
make pkg-base
make pkg-libs
make pkg-core
make pkg-fm
make pkg-shell
make base
```

# Build iso disk image file
```
make image
```
After completion, the generated iso file will be in the build/image directory

## License

This project has been licensed by GPLv3.
