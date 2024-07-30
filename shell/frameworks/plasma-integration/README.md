# Plasma Integration

Integrates Qt applications with the KDE workspace by providing a QPlatformTheme. Applications do not need to link to this directly.

This plugin provides [the KdePlatformTheme](/qt6/src/platformtheme/kdeplatformtheme.h). It handles integrating Qt applications, such as displaying native KDE file dialogs. The theme also controls other miscellaneous integrations like setting the default Qt Quick Controls style and fonts.

## Building

The easiest way to make changes and test Plasma Integration during development is to [build it with kdesrc-build](https://community.kde.org/Get_Involved/development/Build_software_with_kdesrc-build).

When building Plasma Integration manually, keep in mind that the Qt5 and Qt6 versions will be built by default. To control which versions are built, use the `BUILD_QT5` and `BUILD_QT6` CMake variables.
