// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEPROVIDER_H
#define IMAGEPROVIDER_H

#include "thumbnailcache.h"

#include <QQuickImageProvider>
#include <QImageReader>
#include <QImage>
#include <QMutex>

class ProviderCache
{
public:
    ProviderCache();
    virtual ~ProviderCache();

    void rotateImageCached(int angle, const QString &imagePath, int frameIndex = 0);
    void removeImageCache(const QString &imagePath);
    void clearCache();

    virtual void preloadImage(const QString &filePath);

protected:
    QMutex mutex;
    ThumbnailCache imageCache;  ///< 图像数据缓存(已存在锁保护)
    QString lastRotatePath;     ///< 缓存的旋转文件路径
    QImage lastRotateImage;     ///< 缓存的旋转图像信息
    int lastRotation { 0 };     ///< 缓存的旋转角度

    Q_DISABLE_COPY(ProviderCache)
};

// 异步图片加载器
class AsyncImageProvider : public QQuickAsyncImageProvider, public ProviderCache
{
public:
    explicit AsyncImageProvider();
    ~AsyncImageProvider() override;

    virtual QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override;
    void preloadImage(const QString &filePath) override;

private:
    friend class AsyncImageResponse;
};

// 同步图片加载器
class ImageProvider : public QQuickImageProvider, public ProviderCache
{
public:
    explicit ImageProvider();
    ~ImageProvider() override;

    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
};

// 缩略图加载器
class ThumbnailProvider : public QQuickImageProvider
{
public:
    explicit ThumbnailProvider();
    ~ThumbnailProvider() override;

    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;
};

#endif  // IMAGEPROVIDER_H
