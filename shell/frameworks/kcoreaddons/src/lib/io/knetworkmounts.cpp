/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "knetworkmounts.h"
#include "knetworkmounts_p.h"

#include <QCoreApplication>
#include <QGlobalStatic>

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

KNetworkMountsPrivate::KNetworkMountsPrivate(KNetworkMounts *qq)
    : q(qq)
{
}

KNetworkMounts *KNetworkMounts::self()
{
    static KNetworkMounts s_self;
    return &s_self;
}

KNetworkMounts::KNetworkMounts()
    : d(new KNetworkMountsPrivate(this))
{
    const QString configFileName = QStringLiteral("%1/network_mounts").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation));
    d->m_settings = new QSettings(configFileName, QSettings::Format::IniFormat, this);

    for (const auto type : {KNetworkMounts::NfsPaths, KNetworkMounts::SmbPaths, KNetworkMounts::SymlinkDirectory, KNetworkMounts::SymlinkToNetworkMount}) {
        QString typeStr = enumToString(type);
        QStringList slowPaths = d->m_settings->value(typeStr, QStringList()).toStringList();

        if (ensureTrailingSlashes(&slowPaths)) {
            d->m_settings->setValue(typeStr, slowPaths);
        }
    }
}

KNetworkMounts::~KNetworkMounts()
{
}

bool KNetworkMounts::isSlowPath(const QString &path, KNetworkMountsType type)
{
    return !getMatchingPath(path, paths(type)).isEmpty();
}

bool KNetworkMounts::isOptionEnabledForPath(const QString &path, KNetworkMountOption option)
{
    if (!isEnabled()) {
        return false;
    }

    if (!isSlowPath(path)) {
        return false;
    }

    return isOptionEnabled(option, true);
}

bool KNetworkMounts::isEnabled() const
{
    return d->m_settings->value(QStringLiteral("EnableOptimizations"), false).toBool();
}

void KNetworkMounts::setEnabled(const bool value)
{
    d->m_settings->setValue(QStringLiteral("EnableOptimizations"), value);
}

bool KNetworkMounts::isOptionEnabled(const KNetworkMountOption option, const bool defaultValue) const
{
    return d->m_settings->value(enumToString(option), defaultValue).toBool();
}

void KNetworkMounts::setOption(const KNetworkMountOption option, const bool value)
{
    d->m_settings->setValue(enumToString(option), value);
}

QStringList KNetworkMounts::paths(KNetworkMountsType type) const
{
    if (type == Any) {
        QStringList paths;
        paths.reserve(4);
        for (const auto networkMountType :
             {KNetworkMounts::NfsPaths, KNetworkMounts::SmbPaths, KNetworkMounts::SymlinkDirectory, KNetworkMounts::SymlinkToNetworkMount}) {
            paths.append(d->m_settings->value(enumToString(networkMountType), QStringList()).toStringList());
        }
        return paths;
    } else {
        return d->m_settings->value(enumToString(type), QStringList()).toStringList();
    }
}

void KNetworkMounts::setPaths(const QStringList &paths, KNetworkMountsType type)
{
    QStringList _paths(paths);
    ensureTrailingSlashes(&_paths);
    d->m_settings->setValue(enumToString(type), _paths);
}

void KNetworkMounts::addPath(const QString &path, KNetworkMountsType type)
{
    QString _path(path);
    ensureTrailingSlash(&_path);
    QStringList newPaths = paths(type);
    newPaths.append(_path);
    d->m_settings->setValue(enumToString(type), newPaths);
}

typedef QHash<QString /*symlink*/, QString /*canonical path*/> symlinkCanonicalPathHash;
Q_GLOBAL_STATIC(symlinkCanonicalPathHash, s_canonicalLinkSpacePaths)

QString KNetworkMounts::canonicalSymlinkPath(const QString &path)
{
    bool useCache = isOptionEnabled(KNetworkMountOption::SymlinkPathsUseCache, true);
    if (useCache) {
        const QString resolved = s_canonicalLinkSpacePaths->value(path);

        if (!resolved.isEmpty()) {
            return resolved;
        }
    }

    QString symlinkPath = getMatchingPath(path, paths(KNetworkMountsType::SymlinkToNetworkMount));
    if (!symlinkPath.isEmpty()) {
        // remove trailing slash
        symlinkPath.chop(1);

        QFileInfo link(symlinkPath);
        QString linkPath(path);
        QString target = link.symLinkTarget();

        if (target.isEmpty()) {
            // not a symlink
            if (useCache) {
                s_canonicalLinkSpacePaths->insert(path, path);
            }
            return path;
        } else {
            // symlink
            // replace only the first occurence of symlinkPath in linkPath with the link target
            // linkPath.startsWith(symlinkPath) because of getMatchingPath
            linkPath.replace(0, symlinkPath.size(), target);

            if (useCache) {
                s_canonicalLinkSpacePaths->insert(path, linkPath);
            }
            return linkPath;
        }
    }

    QString linkSpacePath = getMatchingPath(path, paths(KNetworkMountsType::SymlinkDirectory));
    if (!linkSpacePath.isEmpty()) {
        QString _path = path;
        if (!_path.endsWith(QLatin1Char('/'))) {
            _path.append(QLatin1Char('/'));
        }

        if (_path == linkSpacePath) {
            if (useCache) {
                s_canonicalLinkSpacePaths->insert(path, path);
            }
            return path;
        }

        // search for symlink, linkSpacePath always ends with '/'
        int linkIndex = path.indexOf(QLatin1Char('/'), linkSpacePath.length());
        const QString symlink = path.left(linkIndex);

        if (useCache && s_canonicalLinkSpacePaths->contains(symlink)) {
            QString linkPath(path);
            // replace only the first occurence of symlink in linkPath
            linkPath.replace(0, symlink.size(), s_canonicalLinkSpacePaths->value(symlink));
            s_canonicalLinkSpacePaths->insert(path, linkPath);
            return linkPath;
        } else {
            QFileInfo link(symlink);

            if (link.isSymLink()) {
                QString linkPath(path);
                // replace only the first occurence of symlink in linkPath
                linkPath.replace(0, symlink.size(), link.symLinkTarget());

                if (useCache) {
                    s_canonicalLinkSpacePaths->insert(path, linkPath);
                }
                return linkPath;
            } else {
                if (useCache) {
                    s_canonicalLinkSpacePaths->insert(path, path);
                }
            }
        }
    }

    return path;
}

void KNetworkMounts::clearCache()
{
    if (s_canonicalLinkSpacePaths.exists()) {
        s_canonicalLinkSpacePaths->clear();
    }
}

void KNetworkMounts::sync()
{
    d->m_settings->sync();
}

#include "moc_knetworkmounts.cpp"
