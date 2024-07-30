/*
    SPDX-FileCopyrightText: 2013 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KPackage/PackageLoader>
#include <KPackage/PackageStructure>

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#define DEFAULT_SHELL "org.kde.lingmo.desktop"

class ShellPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;

    void initPackage(KPackage::Package *package) override
    {
        package->setDefaultPackageRoot(QStringLiteral("lingmo/shells/"));

        // Directories
        package->addDirectoryDefinition("applet", QStringLiteral("applet"));
        package->addDirectoryDefinition("configuration", QStringLiteral("configuration"));
        package->addDirectoryDefinition("explorer", QStringLiteral("explorer"));
        package->addDirectoryDefinition("views", QStringLiteral("views"));

        package->setMimeTypes("applet", QStringList{QStringLiteral("text/x-qml")});
        package->setMimeTypes("configuration", QStringList{QStringLiteral("text/x-qml")});
        package->setMimeTypes("views", QStringList{QStringLiteral("text/x-qml")});

        // Files
        // Default layout
        package->addFileDefinition("defaultlayout", QStringLiteral("layout.js"));
        package->addFileDefinition("defaults", QStringLiteral("defaults"));
        package->setMimeTypes("defaultlayout", QStringList{QStringLiteral("application/javascript"), QStringLiteral("text/javascript")});
        package->setMimeTypes("defaults", QStringList{QStringLiteral("text/plain")});

        // OSD
        package->addDirectoryDefinition("osd", QStringLiteral("osd"));
        package->addFileDefinition("osdmainscript", QStringLiteral("osd/Osd.qml"));

        // Applet furniture
        package->addFileDefinition("appleterror", QStringLiteral("applet/AppletError.qml"));
        package->addFileDefinition("compactapplet", QStringLiteral("applet/CompactApplet.qml"));
        package->addFileDefinition("defaultcompactrepresentation", QStringLiteral("applet/DefaultCompactRepresentation.qml"));

        // Configuration
        package->addFileDefinition("appletconfigurationui", QStringLiteral("configuration/AppletConfiguration.qml"));
        package->addFileDefinition("containmentconfigurationui", QStringLiteral("configuration/ContainmentConfiguration.qml"));
        package->addFileDefinition("panelconfigurationui", QStringLiteral("configuration/PanelConfiguration.qml"));
        package->addFileDefinition("appletalternativesui", QStringLiteral("explorer/AppletAlternatives.qml"));
        package->addFileDefinition("containmentmanagementui", QStringLiteral("configuration/ShellContainmentConfiguration.qml"));

        // Widget explorer
        package->addFileDefinition("widgetexplorer", QStringLiteral("explorer/WidgetExplorer.qml"));

        // Lock screen
        package->addDirectoryDefinition("lockscreen", QStringLiteral("lockscreen"));
        package->addFileDefinition("lockscreenmainscript", QStringLiteral("lockscreen/LockScreen.qml"));

        package->addFileDefinition("interactiveconsole", QStringLiteral("InteractiveConsole.qml"));
    }

    void pathChanged(KPackage::Package *package) override
    {
        if (!package->metadata().isValid()) {
            return;
        }

        const QString pluginName = package->metadata().pluginId();
        if (!pluginName.isEmpty() && pluginName != QStringLiteral(DEFAULT_SHELL)) {
            const QString fallback = package->metadata().value(QStringLiteral("X-Lingmo-FallbackPackage"), QStringLiteral(DEFAULT_SHELL));

            KPackage::Package pkg = KPackage::PackageLoader::self()->loadPackage(QStringLiteral("Lingmo/Shell"), fallback);
            package->setFallbackPackage(pkg);
        } else if (package->fallbackPackage().isValid() && pluginName == QStringLiteral(DEFAULT_SHELL)) {
            package->setFallbackPackage(KPackage::Package());
        }
    }
};

K_PLUGIN_CLASS_WITH_JSON(ShellPackage, "lingmo-packagestructure-lingmo-shell.json")

#include "shellpackage.moc"
