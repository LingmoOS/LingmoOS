# qt5-lingmo-platformtheme

The LINGMO platform theme for qt5 QPA.

## Wiki on Gitee
[Wiki](https://gitee.com/lingmo/qt5-lingmo-platformtheme/wikis/Home)

# Introduction
[zh_CN](https://gitee.com/lingmo/qt5-lingmo-platformtheme/wikis/%E9%A1%B9%E7%9B%AE%E7%AE%80%E4%BB%8B)

[安装和测试](https://gitee.com/lingmo/qt5-lingmo-platformtheme/wikis/%E5%AE%89%E8%A3%85%26%E6%B5%8B%E8%AF%95%E6%B5%81%E7%A8%8B)

## Document
See [doxygen/README.md](doxygen/README.md).

## Description
In the early development of the LINGMO 3.0, we used qt5-gtk2-platformtheme to ensure the unity of the LINGMO desktop style. However, it has many limitations for our new desgin.

This project is intend to provide a common resolution of desktop application development with qt5 in LINGMO3.0. We hope provide a platform theme to unify and beautify all qt applications according to the design of LINGMO3.0, not only our own applications. We are also committed to building our applications that can adapt to different styles. This project is first step to archive those objectives.

qt5-lingmo-platformtheme's route brings us closer to the upstream communities. It is not a division, but a desire for fusion and individuality in a compatible way.

## Build and Test
### Build Depends (reference debian/contorl)
- pkg-config
- qt5-default
- qtbase5-private-dev
- libkf5windowsystem-dev
- libgsettings-qt-dev
- libglib2.0-dev

### Testing
To test the project, you should first install it into system and make sure that the current qpa platform is lingmo.
You can export the QT_QPA_PLATFORMTHEME in terminal.

> export QT_QPA_PLATFORMTHEME=lingmo

One more important job,

> sudo glib-compile-schemas /usr/share/glib-2.0/schemas

That will let the gsettings used by qt5-lingmo-platformtheme worked.

Then you can run the test in project, or run any qt5 program for testing with lingmo platformtheme.

### ToDoList
- change style's details through configuration file
- animations
