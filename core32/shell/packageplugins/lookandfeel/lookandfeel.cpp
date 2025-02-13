/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "lookandfeel.h"

#include <KLocalizedString>
#include <KPackage/PackageLoader>

#define DEFAULT_LOOKANDFEEL "org.lingmo.ocean.desktop"

void LookAndFeelPackage::initPackage(KPackage::Package *package)
{
    // https://community.kde.org/Plasma/lookAndFeelPackage#
    package->setDefaultPackageRoot(QStringLiteral("plasma/look-and-feel/"));

    // Defaults
    package->addFileDefinition("defaults", QStringLiteral("defaults"), i18n("Default settings for theme, etc."));
    package->addFileDefinition("layoutdefaults", QStringLiteral("layouts/defaults"), i18n("Default layout-related settings for titlebars, etc."));
    package->addDirectoryDefinition("plasmoidsetupscripts", QStringLiteral("plasmoidsetupscripts"), i18n("Script to tweak default configs of plasmoids"));
    // Colors
    package->addFileDefinition("colors", QStringLiteral("colors"), i18n("Color scheme to use for applications."));

    // Directories
    package->addDirectoryDefinition("previews", QStringLiteral("previews"), i18n("Preview Images"));
    package->addFileDefinition("preview", QStringLiteral("previews/preview.png"), i18n("Preview for the whole style"));
    package->addFileDefinition("fullscreenpreview", QStringLiteral("previews/fullscreenpreview.jpg"), i18n("Full size preview for the whole style"));
    package->addFileDefinition("loginmanagerpreview", QStringLiteral("previews/loginmanager.png"), i18n("Preview for the Login Manager"));
    package->addFileDefinition("lockscreenpreview", QStringLiteral("previews/lockscreen.png"), i18n("Preview for the Lock Screen"));
    package->addFileDefinition("userswitcherpreview", QStringLiteral("previews/userswitcher.png"), i18n("Preview for the Userswitcher"));
    package->addFileDefinition("desktopswitcherpreview", QStringLiteral("previews/desktopswitcher.png"), i18n("Preview for the Virtual Desktop Switcher"));
    package->addFileDefinition("splashpreview", QStringLiteral("previews/splash.png"), i18n("Preview for Splash Screen"));
    package->addFileDefinition("runcommandpreview", QStringLiteral("previews/runcommand.png"), i18n("Preview for KRunner"));
    package->addFileDefinition("windowdecorationpreview", QStringLiteral("previews/windowdecoration.png"), i18n("Preview for the Window Decorations"));
    package->addFileDefinition("windowswitcherpreview", QStringLiteral("previews/windowswitcher.png"), i18n("Preview for Window Switcher"));

    package->addDirectoryDefinition("loginmanager", QStringLiteral("loginmanager"), i18n("Login Manager"));
    package->addFileDefinition("loginmanagermainscript", QStringLiteral("loginmanager/LoginManager.qml"), i18n("Main Script for Login Manager"));

    package->addDirectoryDefinition("logout", QStringLiteral("logout"), i18n("Logout Dialog"));
    package->addFileDefinition("logoutmainscript", QStringLiteral("logout/Logout.qml"), i18n("Main Script for Logout Dialog"));

    package->addDirectoryDefinition("lockscreen", QStringLiteral("lockscreen"), i18n("Screenlocker"));
    package->addFileDefinition("lockscreenmainscript", QStringLiteral("lockscreen/LockScreen.qml"), i18n("Main Script for Lock Screen"));

    package->addDirectoryDefinition("userswitcher", QStringLiteral("userswitcher"), i18n("UI for fast user switching"));
    package->addFileDefinition("userswitchermainscript", QStringLiteral("userswitcher/UserSwitcher.qml"), i18n("Main Script for User Switcher"));

    package->addDirectoryDefinition("desktopswitcher", QStringLiteral("desktopswitcher"), i18n("Virtual Desktop Switcher"));
    package->addFileDefinition("desktopswitchermainscript",
                               QStringLiteral("desktopswitcher/DesktopSwitcher.qml"),
                               i18n("Main Script for Virtual Desktop Switcher"));

    package->addDirectoryDefinition("osd", QStringLiteral("osd"), i18n("On-Screen Display Notifications"));
    package->addFileDefinition("osdmainscript", QStringLiteral("osd/Osd.qml"), i18n("Main Script for On-Screen Display Notifications"));

    package->addDirectoryDefinition("splash", QStringLiteral("splash"), i18n("Splash Screen"));
    package->addFileDefinition("splashmainscript", QStringLiteral("splash/Splash.qml"), i18n("Main Script for Splash Screen"));

    package->addDirectoryDefinition("runcommand", QStringLiteral("runcommand"), i18n("KRunner UI"));
    package->addFileDefinition("runcommandmainscript", QStringLiteral("runcommand/RunCommand.qml"), i18n("Main Script KRunner"));

    package->addDirectoryDefinition("windowdecoration", QStringLiteral("windowdecoration"), i18n("Window Decoration"));
    package->addFileDefinition("windowdecorationmainscript",
                               QStringLiteral("windowdecoration/WindowDecoration.qml"),
                               i18n("Main Script for Window Decoration"));

    package->addDirectoryDefinition("windowswitcher", QStringLiteral("windowswitcher"), i18n("Window Switcher"));
    package->addFileDefinition("windowswitchermainscript", QStringLiteral("windowswitcher/WindowSwitcher.qml"), i18n("Main Script for Window Switcher"));

    package->addDirectoryDefinition("systemdialog", QStringLiteral("systemdialog"), i18n("System Dialog"));
    package->addFileDefinition("systemdialogscript", QStringLiteral("systemdialog/SystemDialog.qml"), i18n("The system dialog"));

    package->addDirectoryDefinition("layouts", QStringLiteral("layouts"), i18n("Default layout scripts"));

    package->setPath(DEFAULT_LOOKANDFEEL);
}

void LookAndFeelPackage::pathChanged(KPackage::Package *package)
{
    if (!package->metadata().isValid()) {
        return;
    }

    const QString pluginName = package->metadata().pluginId();

    if (!pluginName.isEmpty() && pluginName != DEFAULT_LOOKANDFEEL) {
        KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Plasma/LookAndFeel"), DEFAULT_LOOKANDFEEL);
        package->setFallbackPackage(pkg);
    } else if (package->fallbackPackage().isValid() && pluginName == DEFAULT_LOOKANDFEEL) {
        package->setFallbackPackage(KPackage::Package());
    }
}

K_PLUGIN_CLASS_WITH_JSON(LookAndFeelPackage, "lingmo-packagestructure-lookandfeel.json")

#include "lookandfeel.moc"
