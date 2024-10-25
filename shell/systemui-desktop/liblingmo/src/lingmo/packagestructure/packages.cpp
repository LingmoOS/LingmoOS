/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>

#include <KConfigGroup>
#include <KDesktopFile>
#include <KPackage/Package>
#include <KRuntimePlatform>

#include "config-lingmo.h"
#include "packages_p.h"

namespace Lingmo
{
void GenericPackage::initPackage(KPackage::Package *package)
{
    package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
    package->setRequired("mainscript", true);
    package->addFileDefinition("test", QStringLiteral("tests/test.qml"));

    if (QStringList platform = KRuntimePlatform::runtimePlatform(); !platform.isEmpty()) {
        for (QString &platformEntry : platform) {
            platformEntry.prepend(QLatin1String("platformcontents/"));
        }
        platform.append(QStringLiteral("contents"));
        package->setContentsPrefixPaths(platform);
    }

    package->setDefaultPackageRoot(QStringLiteral(LINGMO_RELATIVE_DATA_INSTALL_DIR "/packages/"));

    package->addDirectoryDefinition("images", QStringLiteral("images"));
    package->addDirectoryDefinition("theme", QStringLiteral("theme"));
    const QStringList imageMimeTypes{QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")};
    package->setMimeTypes("images", imageMimeTypes);
    package->setMimeTypes("theme", imageMimeTypes);

    package->addDirectoryDefinition("config", QStringLiteral("config"));
    package->setMimeTypes("config", QStringList{QStringLiteral("text/xml")});

    package->addDirectoryDefinition("ui", QStringLiteral("ui"));

    package->addDirectoryDefinition("data", QStringLiteral("data"));

    package->addDirectoryDefinition("scripts", QStringLiteral("code"));
    package->setMimeTypes("scripts", QStringList{QStringLiteral("text/plain")});
    package->addFileDefinition("screenshot", QStringLiteral("screenshot.png"));

    package->addDirectoryDefinition("translations", QStringLiteral("locale"));
}

} // namespace Lingmo

#include "moc_packages_p.cpp"
