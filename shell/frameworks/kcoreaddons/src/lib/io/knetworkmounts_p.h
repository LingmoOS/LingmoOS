/*
    This software is a contribution of the LiMux project of the city of Munich.
    SPDX-FileCopyrightText: 2021 Robert Hoffmann <robert@roberthoffmann.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KNETWORKMOUNTS_P_H
#define KNETWORKMOUNTS_P_H

#include "knetworkmounts.h"

#include <QMetaEnum>
#include <QSettings>

class KNetworkMountsPrivate
{
public:
    KNetworkMountsPrivate(KNetworkMounts *);

    KNetworkMounts *q;

    QSettings *m_settings = nullptr;

private:
};

// Append trailing slashes to path string if missing
static bool ensureTrailingSlash(QString *path)
{
    bool changed = false;
    if (!path->isEmpty() && !path->endsWith(QLatin1Char('/'))) {
        path->append(QLatin1Char('/'));
        changed = true;
    }

    return changed;
}

// Append trailing slashes to path strings if missing
static bool ensureTrailingSlashes(QStringList *paths)
{
    bool changed = false;
    for (QString &path : *paths) {
        if (ensureTrailingSlash(&path)) {
            changed = true;
        }
    }

    return changed;
}

// Return the matching configured slow path
static QString getMatchingPath(const QString &_path, const QStringList &slowPaths)
{
    if (slowPaths.isEmpty()) {
        return QString();
    }

    QString path = _path;
    if (!path.endsWith(QLatin1Char('/'))) {
        path.append(QLatin1Char('/'));
    }

    for (const QString &slp : slowPaths) {
        if (path.startsWith(slp)) {
            return slp;
        }
    }

    return QString();
}

// Convert the enums KNetworkMountsType and KNetworkMountOption to QString
template<typename EnumType>
static QString enumToString(EnumType type)
{
    const int intValue = static_cast<int>(type);
    return QString::fromUtf8(QMetaEnum::fromType<EnumType>().valueToKey(intValue));
}

#endif
