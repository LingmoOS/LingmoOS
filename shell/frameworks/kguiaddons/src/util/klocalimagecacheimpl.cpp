/*  This file is part of the KDE project.
    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "klocalimagecacheimpl.h"

#include <QBuffer>
#include <QCache>
#include <QCoreApplication>
#include <QDateTime>

#include <QImage>
#include <QPixmap>

/**
 * This is a QObject subclass so we can catch the signal that the application is about
 * to close and properly release any QPixmaps we have cached.
 */
class KLocalImageCacheImplementationPrivate : public QObject
{
    Q_OBJECT

public:
    KLocalImageCacheImplementationPrivate(QObject *parent = nullptr)
        : QObject(parent)
        , timestamp(QDateTime::currentDateTime())
    {
        QObject::connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &KLocalImageCacheImplementationPrivate::clearPixmaps);
    }

    /**
     * Inserts a pixmap into the pixmap cache if the pixmap cache is enabled, with
     * weighting based on image size and bit depth.
     */
    bool insertPixmap(const QString &key, QPixmap *pixmap)
    {
        if (enablePixmapCaching && pixmap && !pixmap->isNull()) {
            // "cost" parameter is based on both image size and depth to make cost
            // based on size in bytes instead of area on-screen.
            return pixmapCache.insert(key, pixmap, pixmap->width() * pixmap->height() * pixmap->depth() / 8);
        }

        return false;
    }

public Q_SLOTS:
    void clearPixmaps()
    {
        pixmapCache.clear();
    }

public:
    QDateTime timestamp;

    /**
     * This is used to cache pixmaps as they are inserted, instead of always
     * converting to image data and storing that in shared memory.
     */
    QCache<QString, QPixmap> pixmapCache;

    bool enablePixmapCaching = true;
};

KLocalImageCacheImplementation::KLocalImageCacheImplementation(unsigned defaultCacheSize)
    : d(new KLocalImageCacheImplementationPrivate)
{
    // Use at least 16 KiB for the pixmap cache
    d->pixmapCache.setMaxCost(qMax(defaultCacheSize / 8, (unsigned int)16384));
}

KLocalImageCacheImplementation::~KLocalImageCacheImplementation() = default;

void KLocalImageCacheImplementation::updateModifiedTime()
{
    d->timestamp = QDateTime::currentDateTime();
}

QByteArray KLocalImageCacheImplementation::serializeImage(const QImage &image) const
{
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    image.save(&buffer, "PNG");
    return buffer.buffer();
}

bool KLocalImageCacheImplementation::insertLocalPixmap(const QString &key, const QPixmap &pixmap) const
{
    return d->insertPixmap(key, new QPixmap(pixmap));
}

bool KLocalImageCacheImplementation::findLocalPixmap(const QString &key, QPixmap *destination) const
{
    if (d->enablePixmapCaching) {
        QPixmap *cachedPixmap = d->pixmapCache.object(key);
        if (cachedPixmap) {
            if (destination) {
                *destination = *cachedPixmap;
            }

            return true;
        }
    }

    return false;
}

void KLocalImageCacheImplementation::clearLocalCache()
{
    d->pixmapCache.clear();
}

QDateTime KLocalImageCacheImplementation::lastModifiedTime() const
{
    return d->timestamp;
}

bool KLocalImageCacheImplementation::pixmapCaching() const
{
    return d->enablePixmapCaching;
}

void KLocalImageCacheImplementation::setPixmapCaching(bool enable)
{
    if (enable != d->enablePixmapCaching) {
        d->enablePixmapCaching = enable;
        if (!enable) {
            d->pixmapCache.clear();
        }
    }
}

int KLocalImageCacheImplementation::pixmapCacheLimit() const
{
    return d->pixmapCache.maxCost();
}

void KLocalImageCacheImplementation::setPixmapCacheLimit(int size)
{
    d->pixmapCache.setMaxCost(size);
}

#include "klocalimagecacheimpl.moc"
