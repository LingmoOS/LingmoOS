/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "config-plasma.h"
#include "packages_p.h"
#include <KPackage/Package>
#include <KPackage/PackageStructure>

class ContainmentActionsPackage : public Plasma::GenericPackage
{
    Q_OBJECT
public:
    using Plasma::GenericPackage::GenericPackage;

    void initPackage(KPackage::Package *package) override
    {
        GenericPackage::initPackage(package);
        package->setDefaultPackageRoot(QStringLiteral(PLASMA_RELATIVE_DATA_INSTALL_DIR "/containmentactions/"));
    }
};

K_PLUGIN_CLASS_WITH_JSON(ContainmentActionsPackage, "plasma_containmentactions_packagestructure.json")

#include "plasma_containmentactions_packagestructure.moc"
