# KIconThemes

Icon GUI utilities

## Introduction

This library contains classes to improve the handling of icons
in applications using the KDE Frameworks. Provided are:

- KIconDialog: Dialog class to let the user select an icon
    from the list of installed icons.
- KIconButton: Custom button class that displays an icon.
    When clicking it, the user can change it using the icon dialog.
- KIconEffect: Applies various colorization effects to icons,
    which can be used to create selected/disabled icon images.

Other classes in this repository are used internally by the icon
theme configuration dialogs, and should not be used by applications.

## Icon Theme Deployment

On Linux/BSD, it is expected that the main icon themes (hicolor, oxygen, breeze)
are installed by the distribution. The platform theme plugin reads the icon
theme name from KConfig, and redirects QIcon::fromTheme calls to KIconEngine/KIconLoader,
which brings some benefits over Qt's internal icon loading, such as a cache shared
between processes.

On other platforms such as MS Windows and Mac OS, icon themes are not a regular part of the system,
though they are available through package management systems (like MacPorts, Fink and Cygwin).

When icon themes are installed in a suitable (system) location, applications can be built and
deployed as on Linux/BSD provided that the location is registered by calling QIcon::setThemeSearchPath().
This can be done early in the main() function, or in a plugin that Qt loads automatically when
the application starts up (a platform theme plugin, for instance).
The more usual deployment strategy on those platforms consists of creating standalone applications
where each has its own copy of all dependencies and required resources.

Icon theme resources for standalone applications can be created and deployed as follows:
- breeze-icons and other icon themes, when configured with -DBINARY\_ICONS\_RESOURCE=ON, install .rcc files (binary resources, loadable by Qt)
- the installation process should copy one of these under the name "icontheme.rcc", in
    a directory found by [QStandardPaths::AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum).
    For instance on Windows, icontheme.rcc is usually installed in APPROOT/data/,
    while on Mac OS it is installed in the Resources directory inside the application bundle.
- as long as the application links to KIconThemes (even if it doesn't use any of its API),
    the icontheme.rcc file will be found on startup, loaded, and set as the default icon theme.

A related note of caution: These systems use embedded application icons regardless
of how icon themes are provided, and allow users to change that icon. Software should
thus ideally refrain from calling QGuiApplication::setWindowIcon() or at least pass
QGuiApplication::windowIcon() as the fallback argument when using QIcon::fromTheme().

