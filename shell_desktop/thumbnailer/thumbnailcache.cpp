/*
 * Copyright (C) 2025 Lingmo OS Team.
 *
 * Author:     Lingmo OS Team <team@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "thumbnailcache.h"
#include <QCryptographicHash>
#include <QStandardPaths>
#include <QImageReader>
#include <QFileInfo>
#include <QDateTime>
#include <QImage>
#include <QSize>
#include <QFile>
#include <QDir>
#include <QUrl>

static ThumbnailCache *SELF = nullptr;

ThumbnailCache *ThumbnailCache::self()
{
    if (!SELF) {
        SELF = new ThumbnailCache;
    }

    return SELF;
}

ThumbnailCache::ThumbnailCache(QObject *parent)
    : QObject(parent)
{
    m_thumbnailsDir = QStandardPaths::writableLocation(QStandardPaths::GenericCacheLocation) + QLatin1String("/thumbnails/");

    QDir dir(m_thumbnailsDir);
    if (!dir.exists())
        dir.mkdir(m_thumbnailsDir);
}

QString ThumbnailCache::requestThumbnail(const QString &filePath, const QSize &requestedSize)
{
    QString path(filePath);
    if (path.startsWith("file://")) {
        path = path.mid(7);
    }

    if (!QFile::exists(path)) {
        return QString();
    }

    int width = requestedSize.width();
    int height = requestedSize.height();
    int cacheSize = 0;

    // 首先需要找到目录
    if (width <= 128 && height <= 128)
        cacheSize = 128;
    else if (width <= 256 && height <= 256)
        cacheSize = 256;
    else
        cacheSize = 512;

    struct CachePool {
        QString path;
        int minSize;
    };

    const static auto pools = {
        CachePool{ QStringLiteral("/normal/"), 128 },
        CachePool{ QStringLiteral("/large/"), 256 },
        CachePool{ QStringLiteral("/x-large/"), 512 },
        CachePool{ QStringLiteral("/xx-large/"), 1024 },
    };

    QString thumbDir;
    int wants = /*devicePixelRatio **/ cacheSize;
    for (const auto &p : pools) {
        if (p.minSize < wants) {
            continue;
        } else {
            thumbDir = p.path;
            break;
        }
    }

    // 尺寸文件夹
    QString sizeDir = m_thumbnailsDir + thumbDir;

    // 不存在需要创建路径
    if (!QDir(sizeDir).exists()) {
        if (QDir().mkpath(sizeDir)) {
            QFile f(sizeDir);
            f.setPermissions(QFile::ReadUser | QFile::WriteUser | QFile::ExeUser); // 0700
        }
    }

    // Md5
    QByteArray origName;
    const QFileInfo info(path);
    const QString canonicalPath = info.canonicalFilePath();
    origName = QUrl::fromLocalFile(canonicalPath).toEncoded(QUrl::RemovePassword | QUrl::FullyEncoded);

    if (origName.isEmpty()) {
        return QString();
    }

    QCryptographicHash md5(QCryptographicHash::Md5);
    md5.addData(origName);
    // Image size
    md5.addData(QString::number(width).toStdString().c_str());
    md5.addData(QString::number(height).toStdString().c_str());
    // Time
    md5.addData(QString::number(info.lastModified().toTime_t()).toStdString().c_str());

    QString thumbnailsName = QString::fromLatin1(md5.result().toHex()) + QLatin1String(".png");
    QString thumbnailsPath = m_thumbnailsDir + thumbDir + thumbnailsName;

    if (!QFile::exists(thumbnailsPath)) {
        return generateThumbnail(path, thumbnailsPath, requestedSize);
    } else {
        return thumbnailsPath;
    }
}

QString ThumbnailCache::generateThumbnail(const QString &source, const QString &target, const QSize &requestedSize)
{
    QImageReader reader(source);
    if (reader.canRead()) {
        // Quality in the jpeg reader is binary. >= 50: high quality, < 50 fast
        reader.setQuality(49);

        reader.setScaledSize(reader.size().scaled(requestedSize.width(),
                                                  requestedSize.height(), Qt::KeepAspectRatio));
        reader.setAutoTransform(true);

        QImage image(reader.read());
        return writeCacheFile(target, image);
    }

    return QString();
}

QString ThumbnailCache::writeCacheFile(const QString &path, const QImage &image)
{
    const QString thumbnailPath(path);
    QFile thumbnailFile(path);

    if (!thumbnailFile.open(QIODevice::WriteOnly)) {
        return QString();
    }

    image.save(&thumbnailFile, image.hasAlphaChannel() ? "PNG" : "JPG");
    thumbnailFile.flush();
    thumbnailFile.close();

    return thumbnailPath;
}
