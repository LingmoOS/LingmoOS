# QQC2 Desktop Style

This is a style for Qt Quick Controls (also known as QQC2 in Qt5) which uses the application's [QStyle](https://doc.qt.io/qt-6/qstyle.html) to paint the controls in order to give them native look and feel.

This framework has no public API, applications should not (and cannot) use it directly. Instead, developers should add this framework as a dependency of their desktop apps.

## Usage

The name of the style is `org.kde.desktop`. 

On Plasma, this style is picked up automatically without the need to set an environment variable as long as both [Breeze](https://invent.kde.org/plasma/breeze) and [Plasma Integration](https://invent.kde.org/plasma/plasma-integration) are installed.

To support non-Plasma environments like Windows or GNOME, applications will need to define this style in code like [any other QQC style](https://doc.qt.io/qt-6/qtquickcontrols2-styles.html#using-styles-in-qt-quick-controls), e.g.:

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

**NOTE: the application must be a QApplication rather than a QGuiApplication instance in order for this style to be used.**

If the application supports Android, check if  `Q_OS_ANDROID` is defined first:

```c++
#ifdef Q_OS_ANDROID
    QGuiApplication app(argc, argv);
    // Set your preferred Android QQC style here, in this case QQC2 Breeze Style
    QQuickStyle::setStyle(QStringLiteral("org.kde.breeze"));
#else
    QApplication app(argc, argv);
    if (qEnvironmentVariableIsEmpty("QT_QUICK_CONTROLS_STYLE")) {
        QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));
    }
#endif
```

## Differences from QQC2 Breeze Style

QQC2 Desktop Style should be used in desktop applications thanks to its native QStyling, making it look and feel like a QtWidgets application. However it has a noticeable performance impact on mobile systems like Plasma Mobile and Android. It's also desirable to avoid using QtWidgets on those platforms too. 

It's recommended to use [QQC2 Breeze Style](https://invent.kde.org/plasma/qqc2-breeze-style) as shown above for those platforms. It replicates the Breeze visual design without depending on QStyle. 

## Building

The easiest way to make changes and test QQC2 Desktop Style during development is to [build it with kdesrc-build](https://community.kde.org/Get_Involved/development/Build_software_with_kdesrc-build).

## Contributing

Like other projects in the KDE ecosystem, contributions are welcome from all. This repository is managed in [KDE Invent](https://invent.kde.org/frameworks/qqc2-desktop-style), our GitLab instance.

* Want to contribute code? See the [GitLab wiki page](https://community.kde.org/Infrastructure/GitLab) for a tutorial on how to send a merge request.
* Reporting a bug? Please submit it on the [KDE Bugtracking System](https://bugs.kde.org/enter_bug.cgi?format=guided&product=frameworks-qqc2-desktop-style). Please do not use the Issues tab to report bugs.

If you get stuck or need help with anything at all, head over to the [KDE New Contributors room](https://go.kde.org/matrix/#/#kde-welcome:kde.org) on Matrix. For questions about QQC2 Desktop Style, please ask in the [KDE Development room](https://go.kde.org/matrix/#/#kde-devel:kde.org). See [Matrix](https://community.kde.org/Matrix) for more details.

