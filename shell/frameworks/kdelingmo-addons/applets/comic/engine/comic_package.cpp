/*
 *   SPDX-FileCopyrightText: 2008 Petri Damstén <damu@iki.fi>
 *   SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de<
 *
 *   SPDX-License-Identifier: LGPL-2.0-only
 */

#include <KConfigGroup>
#include <KDesktopFile>
#include <KPackage/Package>
#include <KPackage/PackageStructure>
#include <KPackage/packagestructure_compat_p>

class ComicPackage : public KPackage::PackageStructure
{
    Q_OBJECT
public:
    using KPackage::PackageStructure::PackageStructure;
    void initPackage(KPackage::Package *package) override
    {
        package->addDirectoryDefinition("images", QStringLiteral("images"));
        package->setMimeTypes("images", QStringList{QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")});

        package->addDirectoryDefinition("scripts", QStringLiteral("code"));
        package->setMimeTypes("scripts", QStringList{QStringLiteral("text/*")});

        package->addFileDefinition("mainscript", QStringLiteral("code/main.js"));
        package->addFileDefinition("mainscript", QStringLiteral("code/main.es"));
        package->addFileDefinition("metadata", QStringLiteral("metadata.desktop"));
        package->setRequired("metadata", true);
        package->setRequired("mainscript", true);
        package->setDefaultPackageRoot(QStringLiteral("lingmo/comics/"));
    }
    void pathChanged(KPackage::Package *package) override
    {
        QMap<QString, QMetaType::Type> extra{{QStringLiteral("X-KDE-LingmoComicProvider-SuffixType"), QMetaType::QString}};
        KPackagePrivate::convertCompatMetaDataDesktopFile(package, extra);
    }
};

K_PLUGIN_CLASS_WITH_JSON(ComicPackage, "lingmo-packagestructure-comic.json")

#include "comic_package.moc"
