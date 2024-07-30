/*  This file is part of the KDE project.
    SPDX-FileCopyrightText: 2010 Michael Pyne <mpyne@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KLOCALIMAGECACHEIMPL_H
#define KLOCALIMAGECACHEIMPL_H

#include <kguiaddons_export.h>

#include <memory>

class KLocalImageCacheImplementationPrivate;

class QImage;
class QPixmap;
class QByteArray;
class QDateTime;
class QString;

/**
 * You are not supposed to use this class directly, use KImageCache instead
 *
 * @internal
 */
class KGUIADDONS_EXPORT KLocalImageCacheImplementation
{
private:
    explicit KLocalImageCacheImplementation(unsigned defaultCacheSize);

public:
    virtual ~KLocalImageCacheImplementation();

    QDateTime lastModifiedTime() const;

    bool pixmapCaching() const;
    void setPixmapCaching(bool enable);

    int pixmapCacheLimit() const;
    void setPixmapCacheLimit(int size);

protected:
    void updateModifiedTime();
    QByteArray serializeImage(const QImage &image) const;

    bool insertLocalPixmap(const QString &key, const QPixmap &pixmap) const;
    bool findLocalPixmap(const QString &key, QPixmap *destination) const;
    void clearLocalCache();

private:
    std::unique_ptr<KLocalImageCacheImplementationPrivate> const d; ///< @internal

    template<class T>
    friend class KSharedPixmapCacheMixin;
};

#endif /* KLOCALIMAGECACHEIMPL_H */
