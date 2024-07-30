# LingmoUI

QtQuick plugins to build user interfaces following the [KDE Human Interface Guidelines](https://develop.kde.org/hig/).

## Introduction

LingmoUI is a set of [QtQuick](https://doc.qt.io/qt-6/qtquick-index.html) components for building adaptable user interfaces based on [QtQuick Controls 2](https://doc.qt.io/qt-6/qtquickcontrols-index.html). LingmoUI makes it easy to create applications that look and feel great on [Lingmo Mobile](https://lingmo-mobile.org/), Desktop Linux, Android, MacOS, and Windows.

The API can be found in the [KDE API Reference website](https://api.kde.org/frameworks/lingmoui/html/index.html) and a LingmoUI tutorial is available in the [KDE Developer Platform](https://develop.kde.org/docs/getting-started/lingmoui/).

We also provide [LingmoUI Gallery](https://invent.kde.org/sdk/lingmoui-gallery) to showcase most LingmoUI components.

## Building LingmoUI

After installing `extra-cmake-modules` (ECM) and the necessary Qt6 development libraries, run:

```bash
git clone https://invent.kde.org/frameworks/lingmoui.git
cd lingmoui
cmake -B build/ -DCMAKE_INSTALL_PREFIX=/path/where/lingmoui/will/be/installed
cmake --build build/
cmake --install build/
```

If you compiled and installed ECM yourself, you will need to add it to your PATH to compile LingmoUI with it, as ECM does not provide its own `prefix.sh` file:

```bash
PATH=/path/to/the/ecm/installation/usr/ cmake -B build/ -DCMAKE_INSTALL_PREFIX=/path/where/lingmoui/will/be/installed
cmake --build build/
cmake --install build/
```

Alternatively, we recommend you use [kdesrc-build](https://community.kde.org/Get_Involved/development#Setting_up_the_development_environment) to build extra-cmake-modules and LingmoUI together.

The provided LingmoUI example can be built and run with:

```bash
cmake -B build/ -DBUILD_EXAMPLES=ON
cmake --build build/
./build/bin/applicationitemapp
```

And the remaining examples containing only single QML files in the `examples/` folder can be viewed using `qml <filename.qml>` or `qmlscene <filename.qml>`.

# Using a self-compiled LingmoUI in your application

To compile your application and link a self-compiled build of LingmoUI to it, run:

```bash
source path/to/lingmoui/build/prefix.sh
```

And then compile your application as usual.

# Build your Android application and ship it with LingmoUI

1) Build LingmoUI

You will need to compile Qt for Android or use the [Qt Installer](https://www.qt.io/download-open-source) to install it, in addition to the Android SDK and NDK. After that, run:

```bash
cmake -B build/ \
    -DCMAKE_TOOLCHAIN_FILE=/usr/share/ECM/toolchain/Android.cmake \
    -DCMAKE_PREFIX_PATH=/path/to/Qt5.15.9/5.15/android_armv7/ \
    -DCMAKE_INSTALL_PREFIX=/path/where/lingmoui/will/be/installed/ \
    -DECM_DIR=/usr/share/ECM/cmake

cmake --build build/
cmake --install build/
```

2) Build your application

This guide assumes that you build your application with CMake and use [Extra CMake Modules (ECM)](https://api.kde.org/ecm/) from KDE frameworks.

Replace `$yourapp` with the actual name of your application:

```bash
cmake -B build/ \
    -DCMAKE_TOOLCHAIN_FILE=/usr/share/ECM/toolchain/Android.cmake \
    -DQTANDROID_EXPORTED_TARGET=$yourapp \
    -DANDROID_APK_DIR=../path/to/yourapp/ \
    -DCMAKE_PREFIX_PATH=/path/to/Qt5.15.9/5.15/android_armv7/ \
    -DCMAKE_INSTALL_PREFIX=/path/where/yourapp/will/be/installed/

cmake --build build/
cmake --install build/
cmake --build build/ --target create-apk-$yourapp
```

Note: `-DCMAKE_INSTALL_PREFIX` directory should be the same as where LingmoUI was installed, since you need to create an apk package that contains both the LingmoUI build and the build of your application.
