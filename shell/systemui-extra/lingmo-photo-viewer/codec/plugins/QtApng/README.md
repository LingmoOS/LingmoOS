# QtApng
apng image plugin for Qt to support animated PNGs

[![CI build](https://github.com/Skycoder42/QtApng/workflows/CI%20build/badge.svg)](https://github.com/Skycoder42/QtApng/actions?query=workflow%3A%22CI+build%22)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/c2a4d6813a07497784580e79475a314e)](https://www.codacy.com/app/Skycoder42/QtApng)
[![AUR](https://img.shields.io/aur/version/qt5-apng-plugin.svg)](https://aur.archlinux.org/pkgbase/qt5-apng-plugin/)

## Features
Enable the usage of apng images with Qt. The plugin adds the apng format as a new format for any Qt application, and thus supports loading of apng images via `QMovie`, `AnimatedImage` and other types.

## Requirements
To build the plugin, libpng with the apng patch applied is required. By default the libpng provided by `pkg-config` is used for unix-like systems. If the library is not available, the project will compile a static library of libpng and embed it into the plugin. You can force this behaviour by running qmake with `CONFIG += libpng_static`

### Embedded libpng/zlib
The project comes with a version of zlib and libpng (with the apng patch). They can be found in the `src/3rdparty` subfolder. They are automatically compiled into static libraries and used to link the apng plugin when neccessary. Please note that both libraries are compiled without any optimizations for architecture etc. If you wish to have those features, you can replace those two by your own versions.

 Project	| Version	| License																	| Project page
------------|-----------|---------------------------------------------------------------------------|--------------
 zlib		| 1.2.11	| [zlib-license](https://www.zlib.net/zlib_license.html)					| https://www.zlib.net/
 libpng		| 1.6.37	| [libpng-license](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt)	| http://www.libpng.org/pub/png/libpng.html
 apng patch	| 1.6.37	| [libpng-license](http://www.libpng.org/pub/png/src/libpng-LICENSE.txt)	| https://sourceforge.net/projects/libpng-apng/

## Download/Installation
1. Package Managers: The library is available via:
	- **Arch-Linux:** AUR-Repository: [`qt5-apng-plugin`](https://aur.archlinux.org/pkgbase/qt5-apng-plugin/)
2. Simply add my repository to your Qt MaintenanceTool (Image-based How-To here: [Add custom repository](https://github.com/Skycoder42/QtModules/blob/master/README.md#add-my-repositories-to-qt-maintenancetool)):
	1. Start the MaintenanceTool from the commandline using `/path/to/MaintenanceTool --addTempRepository <url>` with one of the following urls (GUI-Method is currently broken, see [QTIFW-1156](https://bugreports.qt.io/browse/QTIFW-1156)) - This must be done *every time* you start the tool:
		- On Linux: https://install.skycoder42.de/qtmodules/linux_x64
		- On Windows: https://install.skycoder42.de/qtmodules/windows_x86
		- On Mac: https://install.skycoder42.de/qtmodules/mac_x64
	2. A new entry appears under all supported Qt Versions (e.g. `Qt > Qt 5.11 > Skycoder42 Qt modules`)
	3. You can install either all of my modules, or select the one you need: `Qt Apng Image Plugin`
	4. Continue the setup and thats it! you can now use the module for all of your installed Kits for that Qt
3. Download the compiled modules from the release page. **Note:** You will have to add the correct ones yourself and may need to adjust some paths to fit your installation! In addition to that, you will have to download the modules this one depends on as well. See Section "Requirements" below.
4. Build it yourself! **Note:** This requires all build an runtime dependencies to be available (See Section "Requirements" below). If you don't have/need cmake, you can ignore the related warnings. To automatically build and install to your Qt installation, run:
	- Download the sources. Either use `git clone` or download from the releases. If you choose the second option, you have to manually create a folder named `.git` in the projects root directory, otherwise the build will fail.
	- `qmake`
	- `make` (If you want the tests/examples/etc. run `make all`)
	- `make install`

## Usage
Simply use the default Qt classes like `QImageReader`, `QMovie` etc. and open the apng files just like you would open normal images/animations (like gif files)

**Format Detection:**
Since the png format is already used by Qt, `*.png` files will **not** use the plugin. To load a png as animated, you can either rename the file to `*.apng`, or set the format explicitly

```cpp
QMovie movie("path/to/image.png", "apng");
```
