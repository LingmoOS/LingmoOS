KDE Wacom support
=================

This module implements a GUI for the Wacom Linux Drivers and extends it
with profile support to handle different button / pen layouts per profile.

For hardware support have a look at https://linuxwacom.github.io/

Configuration can be located in System Settings → Input devices → Tablets

Easy profile switching is supported via hotkeys or a small plasma applet in tray notification area.
In addition, a daemon running in the background helps with hotplug support.

This project only works on X11. For Plasma Wayland there is a separate KCM to configure graphics tablets in [plasma-desktop](https://invent.kde.org/plasma/plasma-desktop/-/tree/master/kcms/tablet).

Hardware support
----------------

All tablets can be set up as long as they are found with the wacom kernel module.

Check with

    $ xsetwacom list devices

if your device is correctly recognized first. If it doesn't show up there, we can't configure it.

Installation
------------

Wacom support in KDE is a separate component and might be not installed by default.
Package usually goes by name `wacomtablet` or `kcm-wacomtablet`.

More or less full list of distributions including the package should be avaliable at:

* https://repology.org/metapackage/kcm-wacomtablet/versions

Starting the module
------------------

Background daemon should be started automatically each time you log in.

You can enable or manually start the service via System Settings → Startup and Shutdown → Background services.

Adding missing tablet devices
-----------------------------

If your device is not recognized by this program, but you can set it up via the xsetwacom driver,
run Wacom Tablet Finder (`kde_wacom_tabletfinder`), describe your device and click `Save`, and restart Wacom tablet service (see above).

Please send your device description (file `~/.config/tabletdblocalrc`) to the bugtracker, so it will be added in future versions.

Reporting bugs
--------------

Please report bugs here: https://bugs.kde.org/enter_bug.cgi?product=wacomtablet

Building & manual installation
==============================

This is *not* a recommended way to use this application. Installing it manually creates files untracked by your package manager.
This can cause everyone problems later on. Please contact your distribution's maintainers to package it instead.

Run-time dependencies:
* X11 wacom tablet driver (xserver-xorg-input-wacom), including the xsetwacom tool, version 0.20 or higher
* libwacom. Highly recommended to have version 0.29 or higher for support of consumer-grade tablets with quirky buttons.

Build dependencies on Debian/Ubuntu:
* g++
* cmake
* extra-cmake-modules
* gettext
* libqt5x11extras5-dev
* qtdeclarative5-dev
* libkf5coreaddons-dev
* libkf5i18n-dev
* libkf5dbusaddons-dev
* libkf5globalaccel-dev
* libkf5config-dev
* libkf5xmlgui-dev
* libkf5notifications-dev
* plasma-framework-dev
* kdoctools-dev
* libxi-dev
* libxcb-xinput-dev
* libwacom-dev

You can install them by running:

    $ apt install g++ cmake extra-cmake-modules gettext libqt5x11extras5-dev qtdeclarative5-dev libkf5coreaddons-dev libkf5i18n-dev libkf5dbusaddons-dev libkf5globalaccel-dev libkf5config-dev libkf5xmlgui-dev libkf5notifications-dev plasma-framework-dev kdoctools-dev libxi-dev libwacom-dev

Building from source
--------------------

    $ mkdir build
    $ cd build
    $ cmake ../ -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE=Release -DKDE_INSTALL_USE_QT_SYS_PATHS=ON
    $ make

Manual installation (this *will* litter in your system):

    $ make install

Running without installing
--------------------------

This is not intended for daily use, but for people who want to test developer builds.

Running KCM component:

    $ QT_PLUGIN_PATH=src/kcmodule/ kcmshell5 wacomtablet

Running KDED component:

    $ ln -s kf5 src/
    $ kquitapp5 kded5 && QT_PLUGIN_PATH=$PWD kded5

Running tablet finder:

    $ src/tabletfinder/kde_wacom_tabletfinder

Running unit tests
==================

First, configure the build to include tests by enabling BUILD_TESTING:

    $ mkdir build
    $ cd build
    $ cmake ../ -DBUILD_TESTING=ON
    $ make

Then, run the tests with:

    $ ctest

or a single one via

    $ cd autotests/common/property
    $ ./Test.Common.Property

you can find the test results in `Testing/Temporary/LastTest.log`

Releasing new version
=====================

Checklist for developers:

* Bump main package version in: `CMakeLists.txt`
* Bump applet version in: `src/applet/package/metadata.desktop`
* Follow https://community.kde.org/ReleasingSoftware
