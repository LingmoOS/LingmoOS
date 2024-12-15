// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailcache.h"

ThumbnailCache::ThumbnailCache()
{
    // 设置默认缓存为240
    cache.setMaxCost(240);
}

ThumbnailCache::~ThumbnailCache() {}

ThumbnailCache *ThumbnailCache::instance()
{
    static ThumbnailCache ins;
    return &ins;
}

/**
   @return 返回缓存中是否存在文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
 */
bool ThumbnailCache::contains(const QString &path, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    return cache.contains(toFindKey(path, frameIndex));
}

/**
   @return 返回缓存中文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
    QImage内部使用引用计数降低拷贝次数
 */
QImage ThumbnailCache::get(const QString &path, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    QImage *image = cache.object(toFindKey(path, frameIndex));
    if (image) {
        return *image;
    } else {
        return QImage();
    }
}

QImage ThumbnailCache::take(const QString &path, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    QImage *image = cache.take(toFindKey(path, frameIndex));
    if (image) {
        return *image;
    } else {
        return QImage();
    }
}

/**
   @brief 添加文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
 */
void ThumbnailCache::add(const QString &path, int frameIndex, const QImage &image)
{
    // TODO: not need reallocate
    QMutexLocker _locker(&mutex);
    cache.insert(toFindKey(path, frameIndex), new QImage(image));
}

/**
   @brief 移除文件路径为 \a path 和图片帧索引为 \a frameIndex 的缩略图
 */
void ThumbnailCache::remove(const QString &path, int frameIndex)
{
    QMutexLocker _locker(&mutex);
    cache.remove(toFindKey(path, frameIndex));
}

/**
   @brief 设置当前缓存的最大容量为 \a maxCost
 */
void ThumbnailCache::setMaxCost(int maxCost)
{
    QMutexLocker _locker(&mutex);
    cache.setMaxCost(maxCost);
}

/**
   @brief 清空缩略图信息
 */
void ThumbnailCache::clear()
{
    QMutexLocker _locker(&mutex);
    cache.clear();
}

/**
   @return 返回图片的
 */
QList<ThumbnailCache::Key> ThumbnailCache::keys()
{
    QMutexLocker _locker(&mutex);
    return cache.keys();
}

/**
   @return 组合图像文件路径 \a path 和图像帧索引 \a frameIndex 为缩略图缓存处理的 key
 */
ThumbnailCache::Key ThumbnailCache::toFindKey(const QString &path, int frameIndex)
{
    return qMakePair(path, frameIndex);
}
