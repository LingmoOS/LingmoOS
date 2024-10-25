/*
    SPDX-FileCopyrightText: 2007-2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <KPackage/PackageStructure>
#include <KRuntimePlatform>

class QmlWallpaperPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;

    void initPackage(KPackage::Package *package) override
    {
        package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
        package->setRequired("mainscript", true);

        QStringList platform = KRuntimePlatform::runtimePlatform();
        if (!platform.isEmpty()) {
            QMutableStringListIterator it(platform);
            while (it.hasNext()) {
                it.next();
                it.setValue(QStringLiteral("platformcontents/") + it.value());
            }

            platform.append(QStringLiteral("contents"));
            package->setContentsPrefixPaths(platform);
        }

        package->setDefaultPackageRoot(QStringLiteral("lingmo/wallpapers/"));

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
};

K_PLUGIN_CLASS_WITH_JSON(QmlWallpaperPackage, "lingmo-packagestructure-wallpaper.json")

#include "wallpaper.moc"
