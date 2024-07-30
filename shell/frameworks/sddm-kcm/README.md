# sddm-kcm - Login Screen (SDDM) System Settings Module

`sddm-kcm` is a KConfig Module (KCM) that integrates itself into KDE's System Settings and serves the purpose of configuring the Simple Desktop Display Manager (SDDM) - the recommended display manager for KDE Plasma.

The main features are the selection of an SDDM theme and setting/modifying its wallpaper. The KCM also supports KDE's Get Hot New Stuff (GHNS) feature, which  means it is possible to install (and remove) user-created SDDM themes hosted on [the KDE Store](https://store.kde.org/browse/cat/101/).

The remainder of the options is geared towards affecting the login screen's behavior, such as enabling or disabling auto-login or setting the default session.

Certain visual and behavioral discrepancies between the login screen and the desktop — which may present themselves due to the login screen's inability to access user files — can be alleviated with the use of the settings synchronization feature. The feature currently allows users to sync the Plasma color scheme, cursor theme, font, font rendering, NumLock preference, Plasma theme, and scaling DPI.

# Installation

Most of the time your distribution should already include this settings module. If it doesn't, the module should exist as an installable standalone `sddm-kcm` package.

## Dependencies
* Qt5
* CMake (with extra modules)
* SDDM
* KDE Frameworks 5

## Compiling from source
Installing system-wide:

```
git clone https://anongit.kde.org/sddm-kcm.git
cd sddm-kcm
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

# Support
## Reporting bugs
Bugs should be reported to the KDE bug tracking system:

* https://bugs.kde.org/ (product `systemsettings`, component `sddm-kcm`)

The following community page can help explain the bug reporting process:

* https://community.kde.org/Get_Involved/Issue_Reporting

## Contributing
Developers are appreciative of potential contributions and will gladly help out with them. For contributors new to KDE there are guidelines which may be of assistance:

* https://community.kde.org/Get_Involved/development

If there is something you would like to discuss, you can either use [the main KDE developer mailing list](plasma-devel@kde.org) or contact the community via instant messaging. The most appropriate groups would be #plasma or #kde-vdg.

## End user
Some distributions use their own SDDM themes. If it's evident that a particular issue with the login screen stems from the theme please contact the support channels of your Linux distribution for user support.
