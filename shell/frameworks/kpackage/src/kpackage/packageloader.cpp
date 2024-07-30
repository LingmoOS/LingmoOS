/*
    SPDX-FileCopyrightText: 2010 Ryan Rix <ry@n.rix.si>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "packageloader.h"
#include "private/packageloader_p.h"
#include "private/utils.h"

#include "kpackage_debug.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDirIterator>
#include <QList>
#include <QStandardPaths>

#include <KLazyLocalizedString>
#include <KPluginFactory>
#include <KPluginMetaData>
#include <unordered_set>

#include "config-package.h"

#include "package.h"
#include "packagestructure.h"
#include "private/packagejobthread_p.h"
#include "private/packages_p.h"

namespace KPackage
{
PackageLoader::PackageLoader()
    : d(new PackageLoaderPrivate)
{
}

PackageLoader::~PackageLoader()
{
    for (auto wp : std::as_const(d->structures)) {
        delete wp.data();
    }
    delete d;
}

PackageLoader *PackageLoader::self()
{
    static PackageLoader *s_packageTrader = new PackageLoader;
    return s_packageTrader;
}

Package PackageLoader::loadPackage(const QString &packageFormat, const QString &packagePath)
{
    if (packageFormat.isEmpty()) {
        return Package();
    }

    if (PackageStructure *structure = loadPackageStructure(packageFormat)) {
        Package p(structure);
        if (!packagePath.isEmpty()) {
            p.setPath(packagePath);
        }
        return p;
    }

    return Package();
}

QList<Package> PackageLoader::listKPackages(const QString &packageFormat, const QString &packageRoot)
{
    QList<Package> lst;

    // has been a root specified?
    QString actualRoot = packageRoot;

    PackageStructure *structure = d->structures.value(packageFormat).data();
    // try to take it from the package structure
    if (actualRoot.isEmpty()) {
        if (!structure) {
            if (packageFormat == QLatin1String("KPackage/Generic")) {
                structure = new GenericPackage();
            } else if (packageFormat == QLatin1String("KPackage/GenericQML")) {
                structure = new GenericQMLPackage();
            } else {
                structure = loadPackageStructure(packageFormat);
            }
        }

        if (structure) {
            d->structures.insert(packageFormat, structure);
            actualRoot = Package(structure).defaultPackageRoot();
        }
    }

    if (actualRoot.isEmpty()) {
        actualRoot = packageFormat;
    }

    QStringList paths;
    if (QDir::isAbsolutePath(actualRoot)) {
        paths = QStringList(actualRoot);
    } else {
        const auto listPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const QString &path : listPath) {
            paths += path + QLatin1Char('/') + actualRoot;
        }
    }

    for (auto const &plugindir : std::as_const(paths)) {
        QDirIterator it(plugindir, QDir::Dirs | QDir::NoDotAndDotDot);
        std::unordered_set<QString> dirs;
        while (it.hasNext()) {
            it.next();

            const QString dir = it.filePath();
            if (!dirs.insert(it.fileInfo().fileName()).second) {
                continue;
            }
            Package package(structure);
            package.setPath(dir);
            if (package.isValid()) {
                // Ignore packages with empty metadata here
                if (packageFormat.isEmpty() || !package.metadata().isValid() || readKPackageType(package.metadata()) == packageFormat) {
                    lst << package;
                } else {
                    qInfo() << "KPackage in" << package.path() << readKPackageType(package.metadata()) << "does not match requested format" << packageFormat;
                }
            }
        }
    }
    return lst;
}
QList<KPluginMetaData> PackageLoader::listPackages(const QString &packageFormat, const QString &packageRoot)
{
    // Note: Use QDateTime::currentSecsSinceEpoch() once we can depend on Qt 5.8
    const qint64 now = qRound64(QDateTime::currentMSecsSinceEpoch() / 1000.0);
    bool useRuntimeCache = true;
    if (now - d->pluginCacheAge > d->maxCacheAge && d->pluginCacheAge != 0) {
        // cache is old and we're not within a few seconds of startup anymore
        useRuntimeCache = false;
        d->pluginCache.clear();
    }

    const QString cacheKey = packageFormat + QLatin1Char('.') + packageRoot;
    if (useRuntimeCache) {
        auto it = d->pluginCache.constFind(cacheKey);
        if (it != d->pluginCache.constEnd()) {
            return *it;
        }
    }
    if (d->pluginCacheAge == 0) {
        d->pluginCacheAge = now;
    }

    QList<KPluginMetaData> lst;

    // has been a root specified?
    QString actualRoot = packageRoot;

    // try to take it from the package structure
    if (actualRoot.isEmpty()) {
        PackageStructure *structure = d->structures.value(packageFormat).data();
        if (!structure) {
            if (packageFormat == QLatin1String("KPackage/Generic")) {
                structure = new GenericPackage();
            } else if (packageFormat == QLatin1String("KPackage/GenericQML")) {
                structure = new GenericQMLPackage();
            } else {
                structure = loadPackageStructure(packageFormat);
            }
        }

        if (structure) {
            d->structures.insert(packageFormat, structure);
            Package p(structure);
            actualRoot = p.defaultPackageRoot();
        }
    }

    if (actualRoot.isEmpty()) {
        actualRoot = packageFormat;
    }

    QSet<QString> uniqueIds;
    QStringList paths;
    if (QDir::isAbsolutePath(actualRoot)) {
        paths = QStringList(actualRoot);
    } else {
        const auto listPath = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
        for (const QString &path : listPath) {
            paths += path + QLatin1Char('/') + actualRoot;
        }
    }

    for (auto const &plugindir : std::as_const(paths)) {
        QDirIterator it(plugindir, QStringList{QStringLiteral("metadata.json")}, QDir::Files, QDirIterator::Subdirectories);
        std::unordered_set<QString> dirs;
        while (it.hasNext()) {
            it.next();

            const QString dir = it.fileInfo().absoluteDir().path();
            if (!dirs.insert(dir).second) {
                continue;
            }

            const QString metadataPath = it.fileInfo().absoluteFilePath();
            KPluginMetaData info = KPluginMetaData::fromJsonFile(metadataPath);

            if (!info.isValid() || uniqueIds.contains(info.pluginId())) {
                continue;
            }

            if (packageFormat.isEmpty() || readKPackageType(info) == packageFormat) {
                uniqueIds << info.pluginId();
                lst << info;
            } else {
                qInfo() << "KPackageStructure of" << info << "does not match requested format" << packageFormat;
            }
        }
    }

    if (useRuntimeCache) {
        d->pluginCache.insert(cacheKey, lst);
    }
    return lst;
}

QList<KPluginMetaData> PackageLoader::listPackagesMetadata(const QString &packageFormat, const QString &packageRoot)
{
    return listPackages(packageFormat, packageRoot);
}

QList<KPluginMetaData>
PackageLoader::findPackages(const QString &packageFormat, const QString &packageRoot, std::function<bool(const KPluginMetaData &)> filter)
{
    QList<KPluginMetaData> lst;
    const auto lstPlugins = listPackages(packageFormat, packageRoot);
    for (auto const &plugin : lstPlugins) {
        if (!filter || filter(plugin)) {
            lst << plugin;
        }
    }
    return lst;
}

KPackage::PackageStructure *PackageLoader::loadPackageStructure(const QString &packageFormat)
{
    PackageStructure *structure = d->structures.value(packageFormat).data();
    if (!structure) {
        if (packageFormat == QLatin1String("KPackage/Generic")) {
            structure = new GenericPackage();
            d->structures.insert(packageFormat, structure);
        } else if (packageFormat == QLatin1String("KPackage/GenericQML")) {
            structure = new GenericQMLPackage();
            d->structures.insert(packageFormat, structure);
        }
    }

    if (structure) {
        return structure;
    }

    const KPluginMetaData metaData = structureForKPackageType(packageFormat);
    if (!metaData.isValid()) {
        qCWarning(KPACKAGE_LOG) << "Invalid metadata for package structure" << packageFormat;
        return nullptr;
    }

    auto result = KPluginFactory::instantiatePlugin<PackageStructure>(metaData);
    if (!result) {
        qCWarning(KPACKAGE_LOG).noquote() << "Could not load installer for package of type" << packageFormat << "Error reported was: " << result.errorString;
        return nullptr;
    }

    structure = result.plugin;

    d->structures.insert(packageFormat, structure);

    return structure;
}

void PackageLoader::addKnownPackageStructure(const QString &packageFormat, KPackage::PackageStructure *structure)
{
    d->structures.insert(packageFormat, structure);
}

void PackageLoader::invalidateCache()
{
    self()->d->maxCacheAge = -1;
}

} // KPackage Namespace
