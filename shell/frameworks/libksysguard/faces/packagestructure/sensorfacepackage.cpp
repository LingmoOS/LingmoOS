/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KLocalizedString>
#include <kpackage/package.h>
#include <kpackage/packagestructure.h>

class SensorFacePackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;

    void initPackage(KPackage::Package *package) override
    {
        package->setDefaultPackageRoot(QStringLiteral("ksysguard/sensorfaces"));

        package->addDirectoryDefinition("ui", QStringLiteral("ui"));

        package->addFileDefinition("CompactRepresentation", QStringLiteral("ui/CompactRepresentation.qml"));
        package->setRequired("CompactRepresentation", true);

        package->addFileDefinition("FullRepresentation", QStringLiteral("ui/FullRepresentation.qml"));
        package->setRequired("FullRepresentation", true);

        package->addFileDefinition("ConfigUI", QStringLiteral("ui/Config.qml"));

        package->addDirectoryDefinition("config", QStringLiteral("config"));
        package->addFileDefinition("mainconfigxml", QStringLiteral("config/main.xml"));

        package->addFileDefinition("FaceProperties", QStringLiteral("faceproperties"));
        package->setRequired("FaceProperties", true);
    }
};

K_PLUGIN_CLASS_WITH_JSON(SensorFacePackage, "sensorface-packagestructure.json")

#include "sensorfacepackage.moc"
