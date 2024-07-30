/*
    SPDX-FileCopyrightText: 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPACKAGE_PACKAGE_P_H
#define KPACKAGE_PACKAGE_P_H

#include "../package.h"

#include <QCryptographicHash>
#include <QDir>
#include <QHash>
#include <QPointer>
#include <QSharedData>
#include <QString>
#include <optional>
namespace KPackage
{
class ContentStructure
{
public:
    ContentStructure()
    {
    }

    ContentStructure(const ContentStructure &other)
    {
        paths = other.paths;
        mimeTypes = other.mimeTypes;
        directory = other.directory;
        required = other.required;
    }

    ContentStructure &operator=(const ContentStructure &) = default;

    QStringList paths;
    QStringList mimeTypes;
    bool directory = false;
    bool required = false;
};

class PackagePrivate : public QSharedData
{
public:
    PackagePrivate();
    PackagePrivate(const PackagePrivate &other);
    ~PackagePrivate();

    PackagePrivate &operator=(const PackagePrivate &rhs);

    void createPackageMetadata(const QString &path);
    QString unpack(const QString &filePath);
    void updateHash(const QString &basePath, const QString &subPath, const QDir &dir, QCryptographicHash &hash);
    QString fallbackFilePath(const QByteArray &key, const QString &filename = QString()) const;
    bool hasCycle(const KPackage::Package &package);
    bool isInsidePackageDir(const QString &canonicalPath) const;

    QPointer<PackageStructure> structure;
    QString path;
    QString tempRoot;
    QStringList contentsPrefixPaths;
    QString defaultPackageRoot;
    QHash<QString, QString> discoveries;
    QHash<QByteArray, ContentStructure> contents;
    std::unique_ptr<Package> fallbackPackage;
    QStringList mimeTypes;
    std::optional<KPluginMetaData> metadata;
    bool externalPaths = false;
    bool valid = false;
    bool checkedValid = false;
};

}

#endif
