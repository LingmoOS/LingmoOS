// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILCACHE_H
#define THUMBNAILCACHE_H

#include <QObject>
#include <QMutex>
#include <QCache>
#include <QImage>

class ThumbnailCache
{
public:
    typedef QPair<QString, int> Key;

    ThumbnailCache();
    ~ThumbnailCache();
    static ThumbnailCache *instance();

    bool contains(const QString &path, int frameIndex = 0);
    QImage get(const QString &path, int frameIndex = 0);
    void add(const QString &path, int frameIndex, const QImage &image);
    void remove(const QString &path, int frameIndex);
    void setMaxCost(int maxCost);
    void clear();

    QList<Key> keys();
    static Key toFindKey(const QString &path, int frameIndex = 0);

private:
    QMutex mutex;
    QCache<Key, QImage> cache;
};

#endif // THUMBNAILCACHE_H
