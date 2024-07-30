/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packages_p.h"

#include <math.h>

#include <KLocalizedString>

#include "kpackage/package.h"

void GenericPackage::initPackage(KPackage::Package *package)
{
    KPackage::PackageStructure::initPackage(package);

    package->setDefaultPackageRoot(QStringLiteral("kpackage/generic/"));

    package->addDirectoryDefinition("images", QStringLiteral("images"));
    package->addDirectoryDefinition("theme", QStringLiteral("theme"));
    const QStringList mimetypes{QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")};
    package->setMimeTypes("images", mimetypes);
    package->setMimeTypes("theme", mimetypes);

    package->addDirectoryDefinition("config", QStringLiteral("config"));
    package->setMimeTypes("config", QStringList{QStringLiteral("text/xml")});

    package->addDirectoryDefinition("ui", QStringLiteral("ui"));

    package->addDirectoryDefinition("data", QStringLiteral("data"));

    package->addDirectoryDefinition("scripts", QStringLiteral("code"));
    package->setMimeTypes("scripts", QStringList{QStringLiteral("text/plain")});

    package->addDirectoryDefinition("translations", QStringLiteral("locale"));
}

void GenericQMLPackage::initPackage(KPackage::Package *package)
{
    GenericPackage::initPackage(package);

    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
    package->setRequired("mainscript", true);
    package->setDefaultPackageRoot(QStringLiteral("kpackage/genericqml/"));
}

#include "moc_packages_p.cpp"
