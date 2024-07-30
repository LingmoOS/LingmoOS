#ifndef OPTIONS_H
#define OPTIONS_H

#include <QCommandLineOption>

namespace Options
{
static QCommandLineOption hash()
{
    static QCommandLineOption o{QStringLiteral("hash"),
                                i18nc("Do not translate <path>", "Generate a SHA1 hash for the package at <path>"),
                                QStringLiteral("path")};
    return o;
}
static QCommandLineOption global()
{
    static QCommandLineOption o{QStringList{QStringLiteral("g"), QStringLiteral("global")},
                                i18n("For install or remove, operates on packages installed for all users.")};
    return o;
}
static QCommandLineOption type()
{
    static QCommandLineOption o{QStringList{QStringLiteral("t"), QStringLiteral("type")},
                                i18nc("theme, wallpaper, etc. are keywords, but they may be translated, as both versions "
                                      "are recognized by the application "
                                      "(if translated, should be same as messages with 'package type' context below)",
                                      "The type of package, corresponding to the service type of the package plugin, e.g. KPackage/Generic, Plasma/Theme, "
                                      "Plasma/Wallpaper, Plasma/Applet, etc."),
                                QStringLiteral("type"),
                                QStringLiteral("KPackage/Generic")};
    return o;
}
static QCommandLineOption install()
{
    static QCommandLineOption o{QStringList{QStringLiteral("i"), QStringLiteral("install")},
                                i18nc("Do not translate <path>", "Install the package at <path>"),
                                QStringLiteral("path")};
    return o;
}
static QCommandLineOption show()
{
    static QCommandLineOption o{QStringList{QStringLiteral("s"), QStringLiteral("show")},
                                i18nc("Do not translate <name>", "Show information of package <name>"),
                                QStringLiteral("name")};
    return o;
}
static QCommandLineOption upgrade()
{
    static QCommandLineOption o{QStringList{QStringLiteral("u"), QStringLiteral("upgrade")},
                                i18nc("Do not translate <path>", "Upgrade the package at <path>"),
                                QStringLiteral("path")};
    return o;
}
static QCommandLineOption list()
{
    static QCommandLineOption o{QStringList{QStringLiteral("l"), QStringLiteral("list")}, i18n("List installed packages")};
    return o;
}
static QCommandLineOption listTypes()
{
    static QCommandLineOption o{QStringList{QStringLiteral("list-types")}, i18n("List all known package types that can be installed")};
    return o;
}
static QCommandLineOption remove()
{
    static QCommandLineOption o{QStringList{QStringLiteral("r"), QStringLiteral("remove")},
                                i18nc("Do not translate <name>", "Remove the package named <name>"),
                                QStringLiteral("name")};
    return o;
}
static QCommandLineOption packageRoot()
{
    static QCommandLineOption o{QStringList{QStringLiteral("p"), QStringLiteral("packageroot")},
                                i18n("Absolute path to the package root. If not supplied, then the standard data"
                                     " directories for this KDE session will be searched instead."),
                                QStringLiteral("path")};
    return o;
}
static QCommandLineOption appstream()
{
    static QCommandLineOption o{QStringLiteral("appstream-metainfo"),
                                i18nc("Do not translate <path>", "Outputs the metadata for the package <path>"),
                                QStringLiteral("path")};
    return o;
}
static QCommandLineOption appstreamOutput()
{
    static QCommandLineOption o{QStringLiteral("appstream-metainfo-output"),
                                i18nc("Do not translate <path>", "Outputs the metadata for the package into <path>"),
                                QStringLiteral("path")};
    return o;
}
}

#endif // OPTIONS_H
