/*
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

/**
 * This is a horrifically simple implementation of KSharedDataCache that is
 * basically missing the "shared" part to it, for use on Windows or other platforms
 * that don't support POSIX.
 */
#include "kshareddatacache.h"

#include <QByteArray>
#include <QCache>
#include <QString>

class Q_DECL_HIDDEN KSharedDataCache::Private
{
public:
    KSharedDataCache::EvictionPolicy evictionPolicy;
    QCache<QString, QByteArray> cache;
};

KSharedDataCache::KSharedDataCache(const QString &cacheName, unsigned defaultCacheSize, unsigned expectedItemSize)
    : d(new Private)
{
    d->cache.setMaxCost(defaultCacheSize);

    Q_UNUSED(cacheName);
    Q_UNUSED(expectedItemSize);
}

KSharedDataCache::~KSharedDataCache()
{
    delete d;
}

KSharedDataCache::EvictionPolicy KSharedDataCache::evictionPolicy() const
{
    return d->evictionPolicy;
}

void KSharedDataCache::setEvictionPolicy(KSharedDataCache::EvictionPolicy newPolicy)
{
    d->evictionPolicy = newPolicy;
}

bool KSharedDataCache::insert(const QString &key, const QByteArray &data)
{
    return d->cache.insert(key, new QByteArray(data));
}

bool KSharedDataCache::find(const QString &key, QByteArray *destination) const
{
    QByteArray *value = d->cache.object(key);

    if (value) {
        if (destination) {
            *destination = *value;
        }
        return true;
    } else {
        return false;
    }
}

void KSharedDataCache::clear()
{
    d->cache.clear();
}

void KSharedDataCache::deleteCache(const QString &cacheName)
{
    Q_UNUSED(cacheName);
}

bool KSharedDataCache::contains(const QString &key) const
{
    return d->cache.contains(key);
}

unsigned KSharedDataCache::totalSize() const
{
    return static_cast<unsigned>(d->cache.maxCost());
}

unsigned KSharedDataCache::freeSize() const
{
    if (d->cache.totalCost() < d->cache.maxCost()) {
        return static_cast<unsigned>(d->cache.maxCost() - d->cache.totalCost());
    } else {
        return 0;
    }
}

unsigned KSharedDataCache::timestamp() const
{
    return 0;
}

void KSharedDataCache::setTimestamp(unsigned newTimestamp)
{
    Q_UNUSED(newTimestamp);
}
