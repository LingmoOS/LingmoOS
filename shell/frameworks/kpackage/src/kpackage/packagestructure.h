/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2023 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGESTRUCTURE_H
#define KPACKAGE_PACKAGESTRUCTURE_H

#include <QStringList>

#include <KPluginFactory>

#include <kpackage/package.h>
#include <kpackage/package_export.h>

namespace KPackage
{
/**
 * @class PackageStructure kpackage/packagestructure.h <KPackage/PackageStructure>
 *
 * This class is used to define the filesystem structure of a package type.
 * A PackageStructure is implemented as a dynamically loaded plugin, in the reimplementation
 * of initPackage the allowed fines and directories in the package are set into the package,
 * for instance:
 *
 * @code
 * package->addFileDefinition("mainscript", QStringLiteral("ui/main.qml"));
 * package->setDefaultPackageRoot(QStringLiteral("plasma/wallpapers/"));
 * package->addDirectoryDefinition("images", QStringLiteral("images"));
 * package->addDirectoryDefinition("theme", QStringLiteral("theme"));
 * QStringList mimetypes{QStringLiteral("image/svg+xml"), QStringLiteral("image/png"), QStringLiteral("image/jpeg")};
 * package->setMimeTypes("images", mimetypes);
 * @endcode
 */
class KPACKAGE_EXPORT PackageStructure : public QObject
{
    Q_OBJECT

public:
    explicit PackageStructure(QObject *parent = nullptr, const QVariantList &args = QVariantList());

    ~PackageStructure() override;

    /**
     * Called when a the PackageStructure should initialize a Package with the initial
     * structure. This allows setting paths before setPath is called.
     *
     * Note: one special value is "metadata" which can be set to the location of KPluginMetaData
     * compatible .json file within the package. If not defined, it is assumed that this file
     * exists under the top level directory of the package.
     *
     * @param package the Package to set up. The object is empty of all definition when
     *      first passed in.
     */
    virtual void initPackage(Package *package);

    /**
     * Called whenever the path changes so that subclasses may take
     * package specific actions.
     */
    virtual void pathChanged(Package *package);

private:
    void *d;
};

} // KPackage namespace

#endif
