/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_LOADER_H
#define KPACKAGE_LOADER_H

#include <kpackage/package.h>

#include <kpackage/package_export.h>

namespace KPackage
{
class PackageLoaderPrivate;

/**
 * @class PackageLoader kpackage/packageloader.h <KPackage/PackageLoader>
 *
 * This is an abstract base class which defines an interface to which the package
 * loading logic can communicate with a parent application. The plugin loader
 * must be set before any plugins are loaded, otherwise (for safety reasons), the
 * default PackageLoader implementation will be used. The reimplemented version should
 * not do more than simply returning a loaded plugin. It should not init() it, and it should not
 * hang on to it.
 *
 * @author Ryan Rix <ry@n.rix.si>
 **/
class KPACKAGE_EXPORT PackageLoader
{
public:
    /**
     * Load a Package plugin.
     *
     * @param packageFormat the format of the package to load
     * @param packagePath the package name: the path of the package relative to the
     *        packageFormat root path. If not specified it will have to be set manually
     *        with Package::setPath() by the caller.
     *
     * @return a Package object matching name, or an invalid package on failure
     **/
    Package loadPackage(const QString &packageFormat, const QString &packagePath = QString());

    /**
     * List all available packages of a certain type
     *
     * @param packageFormat the format of the packages to list
     * @param packageRoot the root folder where the packages are installed.
     *          If not specified the default from the packageformat will be taken.
     *
     * @return metadata for all the matching packages
     */
    QList<KPluginMetaData> listPackages(const QString &packageFormat, const QString &packageRoot = QString());

    /**
     * @overload
     * @since 6.0
     */
    QList<KPluginMetaData> listPackagesMetadata(const QString &packageFormat, const QString &packageRoot = QString());

    /**
     * List all available packages of a certain type. This should be used in case the package structure modifies the metadata or you need to access the
     * contained files of the package.
     *
     * @param packageFormat the format of the packages to list
     * @param packageRoot the root folder where the packages are installed.
     *          If not specified the default from the packageformat will be taken.
     *
     * @since 6.0
     */
    QList<Package> listKPackages(const QString &packageFormat, const QString &packageRoot = QString());

    /**
     * List package of a certain type that match a certain filter function
     *
     * @param packageFormat the format of the packages to list
     * @param packageRoot the root folder where the packages are installed.
     *          If not specified the default from the packageformat will be taken.
     * @param filter a filter function that will be called on each package:
     *          will return true for the matching ones
     *
     * @return metadata for all the matching packages
     * @since 5.10
     */
    QList<KPluginMetaData> findPackages(const QString &packageFormat,
                                        const QString &packageRoot = QString(),
                                        std::function<bool(const KPluginMetaData &)> filter = std::function<bool(const KPluginMetaData &)>());

    /**
     * Loads a PackageStructure for a given format. The structure can then be used as
     * paramenter for a Package instance constructor
     *
     * @note The returned pointer is managed by KPackage, and should never be deleted
     *
     * @param packageFormat the package format, such as "KPackage/GenericQML"
     * @return the structure instance (ownership retained by KPackage)
     */
    KPackage::PackageStructure *loadPackageStructure(const QString &packageFormat);

    /**
     * Adds a new known package structure that can be used by the functions to load packages such
     * as loadPackage, findPackages etc
     * @param packageFormat the package format, such as "KPackage/GenericQML"
     * @param structure the package structure we want to be able to load packages from
     * @since 5.10
     */
    void addKnownPackageStructure(const QString &packageFormat, KPackage::PackageStructure *structure);

    /**
     * Return the active plugin loader
     **/
    static PackageLoader *self();

protected:
    PackageLoader();
    virtual ~PackageLoader();

private:
    friend class Package;
    friend class PackageJob;
    KPACKAGE_NO_EXPORT static void invalidateCache();

    PackageLoaderPrivate *const d;
    Q_DISABLE_COPY(PackageLoader)
};

}

Q_DECLARE_METATYPE(KPackage::PackageLoader *)

#endif
