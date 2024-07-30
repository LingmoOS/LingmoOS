/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packagestructure.h"
#include <KPluginFactory>

class PlasmoidTestPackageStructure : public KPackage::PackageStructure
{
    Q_OBJECT

public:
    using KPackage::PackageStructure::PackageStructure;

    void initPackage(KPackage::Package *package) override
    {
        KPackage::PackageStructure::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral("plasma/plasmoids/"));

        package->addDirectoryDefinition("ui", QStringLiteral("ui"));
        package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
        package->setRequired("mainscript", true);

        package->addFileDefinition("configmodel", QStringLiteral("config/config.qml"));
        package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"));

        package->addDirectoryDefinition("images", QStringLiteral("images"));
        package->setMimeTypes("images", {QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")});

        package->addDirectoryDefinition("scripts", QStringLiteral("code"));
        package->setMimeTypes("scripts", {QStringLiteral("text/plain")});
    }
};

K_PLUGIN_CLASS_WITH_JSON(PlasmoidTestPackageStructure, "plasmoidpackagestructure.json")

#include "plasmoidstructure.moc"
