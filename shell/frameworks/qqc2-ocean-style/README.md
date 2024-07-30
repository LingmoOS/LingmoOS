# QQC2 Ocean Style

This is a style for Qt Quick Controls (also known as QQC2 in Qt5) which implements the KDE Visual Design Group's vision for Ocean in pure Qt Quick and LingmoUI. 

This library has no public API, applications should not (and cannot) use it directly. Instead, developers should add this library as a dependency of their apps.

## Usage

The name of the style is `org.kde.ocean`.

On Lingmo Mobile, this style should be used automatically. Ensure that your application does not override the style by accident by checking if `QT_QUICK_CONTROLS_STYLE` is set:

```c++
#include <QQuickStyle>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Default to org.kde.desktop style unless the user forces another style
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }

    QQmlApplicationEngine engine;
    ...
}
```

## Differences from QQC2 Desktop Style

Unlike [QQC2 Desktop Style](https://invent.kde.org/frameworks/qqc2-desktop-style), it does not depend on Qt Widgets and the system QStyle. This means you can use the lighter `QGuiApplication` instead of `QApplication`, and your application does not need to link to QtWidgets.

The performance, loading times and RAM usage should be generally competitive with the Qt Fusion and Material QQC styles.

## Building

The easiest way to make changes and test QQC2 Ocean Style during development is to [build it with kdesrc-build](https://community.kde.org/Get_Involved/development/Build_software_with_kdesrc-build).

## Contributing

Like other projects in the KDE ecosystem, contributions are welcome from all. This repository is managed in [KDE Invent](https://invent.kde.org/lingmo/qqc2-ocean-style), our GitLab instance.

* Want to contribute code? See the [GitLab wiki page](https://community.kde.org/Infrastructure/GitLab) for a tutorial on how to send a merge request.
* Reporting a bug? Please submit it on the [Issues page](https://invent.kde.org/lingmo/qqc2-ocean-style/-/issues).

If you get stuck or need help with anything at all, head over to the [KDE New Contributors room](https://go.kde.org/matrix/#/#kde-welcome:kde.org) on Matrix. For questions about QQC2 Desktop Style, please ask in the [KDE Development room](https://go.kde.org/matrix/#/#kde-devel:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.

