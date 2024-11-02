# explor

explor is the default filemanager application of LINGMO Desktop Environment.

# About this project

- [Wiki on Gitee](https://gitee.com/lingmo/explor/wiki)

- [zh_CN](GUIDE_ZH_CN.md)

# Description

Peony uses the same underlying layer(glib/gvfs/gio) as old explor project(forked from caja), but uses Qt to refactor or improve anything else. Peony can be roughly divided into the following parts:

> 1. explor-qt-core: the abstraction of glib/gvfs/gio object or method from glib/c to qt/c++.

> 2. file-operation: file operation set based on core.

> 3. model: mapping core and operation to Qt's model/view framework.

> 4. extensions framework: migration of the Peony's extension framework.

> 5. ui: reconstructed based on the above parts and qt's ui framework.

# How to understand
Although Peony's documentation is limited and might be outdated, I still recommend you read them through. The documents in this project can be come together with doxygen, see [how to generate the document files](doxygen/README.md).

# Build and test

## Preparation

### **In Debian/Ustable and Ubuntu**

Peony has been uploaded to debian/unstable, and downstreams can get the source in archive by:

> apt source explor

To build explor in local, we should

> sudo apt build-dep explor

~~I recommend you use latest UbuntuLingmo(19.10 for now), and you can build pre-depends with such command:~~

> ~~sudo apt install libglib2.0-dev qt5-default libqt5x11extras5-dev libgsettings-qt-dev  libpoppler-qt5-dev  qttools5-dev-tools~~

~~NOTE: build-depends references the paragraph in debian/control.~~

~~and this is a recommend plugin:~~

> ~~sudo apt install qt5-gtk2-platformtheme~~

NOTE: actually Peony is designed to adapt to all different qt styles, but for now there are lots of things to be done. You can also experience it with other styles, such as breeze, oxygen, not only gtk.

This qpa plugin will let qt applications use system style in UbuntuLingmo.

### **Porting to other distros**

Peony provides some build options for trying to keep the build depencies simpler. For example, there are sevaral macros defined in [common.pri](common.pri). While building explor, it will attempt to find existed build depencies and set related macros for them. If a developer only wants to port explor without special depencies porivided by LINGMO/LINGMOSDK, or is porting whole LINGMO/LINGMOSDK, it's usually meaningful to help the developer know how explor handles the project building and implements function in different way.

It is a little chalange for porting explor to other distros. If you want to try, please make sure the distro's qt's version is higher than 5.6, and glib's version is higher than 2.48. We recommend to build explor with Qt5.12 or later, glib2.0 2.64 or later, KF5(window system/screens) 5.68 or later.

Note that although most of installtions of explor is implemented by qmake and make, I used some featrues provided by debhelper, which is the package tool in debian. That means it is better to know how to do those things without debhelper in other distros, too.

If there are some problems during porting explor to other distros, commit an issue here. I'm willing to help you.

## build from source and test

> git clone https://gitee.com/lingmo/explor.git

> cd explor && mkdir build && cd build

> qmake .. && make

> sudo make install

> /usr/bin/explor

NOTE: Peony & explor-qt-desktop is based on libexplor3, so you should put the libexplor3.so to the directory which is included in ld's config file.

# Examples
Another way to get familiar with the project is through [some examples which I provided](https://github.com/Yue-Lan/libexplor-qt-development-examples).

These examples are based on libexplor3 and display some basic usage of the Peony's api. It is simpler than the project but can help us understand how to use the libexplor3 for development.

# Contribution
I really welcome you to participate in this project. Before that, you'd better read the [contribution manual](CONTRIBUTING.md).

# About third-party open source codes
I used some third-party code and modified it to fit into my project.

The 3rd parties codes would been placed in project's ${top-src-dir}/3rd-parties directory.

Note that Peony/libexplor-qt might use different license (GPL/LGPL) with a 3rd parties' license. I will keep those 3rd parties codes' Copyrights and Licenses.
