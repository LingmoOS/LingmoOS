/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-lingmo.h"
#include <KConfigGroup>
#include <KDesktopFile>
#include <KPackage/Package>
#include <KPackage/PackageStructure>
#include <KPackage/packagestructure_compat_p>

class ThemePackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;

    void initPackage(KPackage::Package *package) override
    {
        // by default the packages have "contents/" as contentsPrefixPaths
        // but for the themes we don't want that, so unset it.
        package->setContentsPrefixPaths(QStringList());
        package->setDefaultPackageRoot(QStringLiteral(LINGMO_RELATIVE_DATA_INSTALL_DIR "/desktoptheme/"));

        package->addDirectoryDefinition("dialogs", QStringLiteral("dialogs/"));
        package->addFileDefinition("dialogs/background", QStringLiteral("dialogs/background.svg"));
        package->addFileDefinition("dialogs/background", QStringLiteral("dialogs/background.svgz"));
        package->addFileDefinition("dialogs/shutdowndialog", QStringLiteral("dialogs/shutdowndialog.svg"));
        package->addFileDefinition("dialogs/shutdowndialog", QStringLiteral("dialogs/shutdowndialog.svgz"));

        package->addDirectoryDefinition("wallpapers", QStringLiteral("wallpapers/"));

        package->addDirectoryDefinition("widgets", QStringLiteral("widgets/"));
        package->addFileDefinition("widgets/background", QStringLiteral("widgets/background.svg"));
        package->addFileDefinition("widgets/background", QStringLiteral("widgets/background.svgz"));
        package->addFileDefinition("widgets/clock", QStringLiteral("widgets/clock.svg"));
        package->addFileDefinition("widgets/clock", QStringLiteral("widgets/clock.svgz"));
        package->addFileDefinition("widgets/panel-background", QStringLiteral("widgets/panel-background.svg"));
        package->addFileDefinition("widgets/panel-background", QStringLiteral("widgets/panel-background.svgz"));
        package->addFileDefinition("widgets/plot-background", QStringLiteral("widgets/plot-background.svg"));
        package->addFileDefinition("widgets/plot-background", QStringLiteral("widgets/plot-background.svgz"));
        package->addFileDefinition("widgets/tooltip", QStringLiteral("widgets/tooltip.svg"));
        package->addFileDefinition("widgets/tooltip", QStringLiteral("widgets/tooltip.svgz"));

        package->addDirectoryDefinition("opaque/dialogs", QStringLiteral("opaque/dialogs/"));
        package->addFileDefinition("opaque/dialogs/background", QStringLiteral("opaque/dialogs/background.svg"));
        package->addFileDefinition("opaque/dialogs/background", QStringLiteral("opaque/dialogs/background.svgz"));
        package->addFileDefinition("opaque/dialogs/shutdowndialog", QStringLiteral("opaque/dialogs/shutdowndialog.svg"));
        package->addFileDefinition("opaque/dialogs/shutdowndialog", QStringLiteral("opaque/dialogs/shutdowndialog.svgz"));

        package->addDirectoryDefinition("opaque/widgets", QStringLiteral("opaque/widgets/"));
        package->addFileDefinition("opaque/widgets/panel-background", QStringLiteral("opaque/widgets/panel-background.svg"));
        package->addFileDefinition("opaque/widgets/panel-background", QStringLiteral("opaque/widgets/panel-background.svgz"));
        package->addFileDefinition("opaque/widgets/tooltip", QStringLiteral("opaque/widgets/tooltip.svg"));
        package->addFileDefinition("opaque/widgets/tooltip", QStringLiteral("opaque/widgets/tooltip.svgz"));

        package->addFileDefinition("colors", QStringLiteral("colors"));

        package->setDefaultMimeTypes({QStringLiteral("image/svg+xml")});

        package->addFileDefinition("metadata", QStringLiteral("metadata.desktop"));
        package->setRequired("metadata", true);
    }
    void pathChanged(KPackage::Package *package) override
    {
        // The KCM uses KPackage to list available themes
        KPackagePrivate::convertCompatMetaDataDesktopFile(package);
    }
};

K_PLUGIN_CLASS_WITH_JSON(ThemePackage, "lingmo_theme_packagestructure.json")

#include "lingmo_theme_packagestructure.moc"
