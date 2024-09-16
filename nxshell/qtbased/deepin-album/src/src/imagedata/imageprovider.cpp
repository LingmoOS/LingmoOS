// SPDX-FileCopyrightText: 2023 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "imageprovider.h"
#include "unionimage/unionimage.h"
#include "imagedata/thumbnailcache.h"

#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

static const QString s_tagFrame = "#frame_";

/**
   @brief 解析图像处理器 \a id , 取得请求的文件路径 \a filePath 和 \a frameIndex
   @note QML 中使用 ImageProvider 取得图像信息，\a id 格式为 \b{图像路径#frame_帧号} ，例如 "/home/tmp.tif#frame_3" ，
        表示 tmp.tif 图像文件的第四帧图像，这个 id 在 QML 文件中组合。
 */
static void parseProviderID(const QString &id, QString &filePath, int &frameIndex)
{
    // 从后向前查询索引标识
    int index = id.lastIndexOf(QRegularExpression(QString("%1\\d+$").arg(s_tagFrame)));
    if (-1 == index) {
        filePath = QUrl(id).toLocalFile();
        frameIndex = 0;
    } else {
        // 移除 "#frame_" 字段
        filePath = QUrl(id.left(index)).toLocalFile();
        frameIndex = id.right(id.size() - index - s_tagFrame.size()).toInt();
    }
}

/**
   @return 使用 FreeImage 库读取 \a imagePath 的图像数据并返回
 */
static QImage readNormalImage(const QString &imagePath)
{
    QImage image;
    QString error;
    if (!LibUnionImage_NameSpace::loadStaticImageFromFile(imagePath, image, error)) {
        qWarning() << QString("Load image %1 error: %2").arg(imagePath).arg(error);
    }
    return image;
}

/**
   @return 读取图像路径 \a imagePath 和 \a frameIndex 指向的图像信息。
 */
static QImage readMultiImage(const QString &imagePath, int frameIndex)
{
    // 重新设置图像读取类
    QImageReader reader(imagePath);

    if (reader.jumpToImage(frameIndex)) {
        // 读取图像数据
        return reader.read();
    }
    return QImage();
}

/**
   @class AsyncImageResponse
   @brief 异步图像加载应答，在子线程完成图像加载后，通过 finished() 信号报告加载状态。
 */
class AsyncImageResponse : public QQuickImageResponse, public QRunnable
{
public:
    AsyncImageResponse(AsyncImageProvider *p, const QString &i, const QSize &r);
    ~AsyncImageResponse() override;

    QQuickTextureFactory *textureFactory() const override;
    void run() override;

    AsyncImageProvider *provider = nullptr;
    QString providerId;
    QSize requestedSize;
    QImage image;
};

AsyncImageResponse::AsyncImageResponse(AsyncImageProvider *p, const QString &i, const QSize &r)
    : provider(p)
    , providerId(i)
    , requestedSize(r)
{
    setAutoDelete(false);
}

AsyncImageResponse::~AsyncImageResponse() { }

QQuickTextureFactory *AsyncImageResponse::textureFactory() const
{
    return QQuickTextureFactory::textureFactoryForImage(image);
}

/**
   @brief 线程中执行加载图像
 */
void AsyncImageResponse::run()
{
    // 解析id，获取当前读取的文件和图片索引
    QString tempPath;
    int frameIndex;
    parseProviderID(providerId, tempPath, frameIndex);

    // 判断缓存中是否存在图片
    image = provider->imageCache.get(tempPath, frameIndex);
    if (image.isNull()) {
        if (frameIndex) {
            image = readMultiImage(tempPath, frameIndex);
        } else {
            image = readNormalImage(tempPath);
        }

        // 缓存图片信息，即使是异常图片
        provider->imageCache.add(tempPath, frameIndex, image);
    }

    // 调整图像大小
    if (!image.isNull() && image.size() != requestedSize && requestedSize.isValid()) {
        image = image.scaled(requestedSize);
    }

    emit finished();
}

/**
   @class ProviderCache
   @brief 图像加载器缓存，存储最近的图像数据并处理旋转等操作
 */
ProviderCache::ProviderCache() { }

ProviderCache::~ProviderCache() { }

/**
   @brief 对缓存的 \a imagePath 图片执行旋转 \a angle 的操作。
   @note 待操作的图片必须在缓存中，当前展示的图片必定在缓存中
 */
void ProviderCache::rotateImageCached(int angle, const QString &imagePath, int frameIndex)
{
    // 旋转角度为0时，清除旋转状态缓存，防止外部文件变更后仍使用上一次的旋转状态。
    QMutexLocker _locker(&mutex);
    if (0 == angle) {
        return;
    }

    QImage image;
    if (imagePath != lastRotatePath) {
        image = imageCache.get(imagePath, frameIndex);

        // 首次处理时记录图像数据，防止多次旋转处理导致图片质量降低
        lastRotateImage = image;
        lastRotatePath = imagePath;
        lastRotation = angle;
    } else {
        image = lastRotateImage;
        lastRotation += angle;
    }
    _locker.unlock();

    if (!image.isNull()) {
        // 360度不执行旋转
        if (!!(lastRotation % 360)) {
            LibUnionImage_NameSpace::rotateImage(lastRotation, image);
        }

        // 更新图片缓存
        imageCache.add(imagePath, frameIndex, image);

        // 同样更新缩略图缓存
        QImage tmpImage = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
        ThumbnailCache::instance()->add(imagePath, frameIndex, tmpImage);
    }
}

/**
   @brief 移除文件路径为 \a imagePath 的图片信息，主要用于文件删除，重命名等变更时重置状态
 */
void ProviderCache::removeImageCache(const QString &imagePath)
{
    // 直接缓存的图像信息较少，遍历查询是否包含对应的图片
    QList<ThumbnailCache::Key> keys;
    QMutexLocker _locker(&mutex);
    keys = imageCache.keys();
    _locker.unlock();

    for (const ThumbnailCache::Key &key : keys) {
        if (key.first == imagePath) {
            _locker.relock();
            imageCache.remove(key.first, key.second);
            _locker.unlock();
        }
    }
}

/**
   @brief 移除图像加载器中的缓存数据
 */
void ProviderCache::clearCache()
{
    QMutexLocker _locker(&mutex);
    imageCache.clear();
    lastRotatePath.clear();
    lastRotateImage = QImage();
}

/**
   @brief 预载图片数据并缓存
 */
void ProviderCache::preloadImage(const QString &)
{
    // Nothing
}

/**
   @class AsyncImageProvider
   @brief 异步图像加载器，提供主要图像的并行加载，主要用于展示图像的加载，会缓存最近的图像信息。
        缩略图通过 ThumbnailProvider 加载
 */
AsyncImageProvider::AsyncImageProvider()
{
    // 缓存最近 3 张图片 + 1 张切换之前的图片
    imageCache.setMaxCost(4);
}

AsyncImageProvider::~AsyncImageProvider() { }

/**
   @brief 请求图像加载并返回应答，当图像加载成功时，通过接收信号进行实际图像的加载
   @param id            图像索引(0 ~ frameCount - 1)
   @param requestedSize 请求的图像大小
   @return 返回图像加载应答
 */
QQuickImageResponse *AsyncImageProvider::requestImageResponse(const QString &id, const QSize &requestedSize)
{
    AsyncImageResponse *response = new AsyncImageResponse(this, id, requestedSize);
    QThreadPool::globalInstance()->start(response, QThread::HighPriority);
    return response;
}

/**
   @brief 预加载图片 \a filePath 数据并缓存，用于首次打开应用
 */
void AsyncImageProvider::preloadImage(const QString &filePath)
{
    AsyncImageResponse *response = new AsyncImageResponse(this, filePath, QSize());
    response->setAutoDelete(true);
    QThreadPool::globalInstance()->start(response, QThread::TimeCriticalPriority);
}

/**
   @class ImageProvider
   @brief 图片加载类，读取图像信息并加载。
   @note 当 QML 中手动设置async时同样可能触发多线程加载
 */
ImageProvider::ImageProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

ImageProvider::~ImageProvider() { }

/**
   @brief 外部请求图像文件中指定帧的图像，指定帧号通过传入的 \a id 进行区分。
        \a id 格式为 \b{图像路径#frame_帧号} ，例如 "/home/tmp.tif#frame_3" ，
        表示 tmp.tif 图像文件的第四帧图像缩略图，这个 id 在 QML 文件中组合。
   @param id            图像索引(0 ~ frameCount - 1)
   @param size          图像的原始大小，有需要时可传出
   @param requestedSize 请求的图像大小
   @return 读取的图像数据
 */
QImage ImageProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // 解析id，获取当前读取的文件和图片索引
    QString tempPath;
    int frameIndex;
    parseProviderID(id, tempPath, frameIndex);

    // 判断缓存中是否存在图片
    QImage image = imageCache.get(tempPath, frameIndex);
    if (image.isNull()) {
        if (frameIndex) {
            image = readMultiImage(tempPath, frameIndex);
        } else {
            image = readNormalImage(tempPath);
        }

        if (size) {
            *size = image.size();
        }

        // 缓存图片信息，即使是异常图片
        imageCache.add(tempPath, frameIndex, image);
    }

    // 调整图像大小
    if (!image.isNull() && image.size() != requestedSize && requestedSize.isValid()) {
        image = image.scaled(requestedSize);
    }

    return image;
}

/**
   @class MultiThumbnailProvider
   @brief 提供含 *.tif 多页图的缩略图像加载处理类
        通过分割传入的 id ，判断当前读取的文件的行数和图片索引。
        在 QML 中注册的标识为 "multiimage"
   @warning QQuickImageProvider 派生的接口可能多线程调用，必须保证实现函数是可重入的。
   @threadsafe
 */

ThumbnailProvider::ThumbnailProvider()
    : QQuickImageProvider(QQmlImageProviderBase::Image)
{
}

ThumbnailProvider::~ThumbnailProvider() { }

/**
   @brief 外部请求图像文件中指定帧的图像，指定帧号通过传入的 \a id 进行区分。
        \a id 格式为 \b{图像路径#frame_帧号} ，例如 "/home/tmp.tif#frame_3" ，
        表示 tmp.tif 图像文件的第四帧图像缩略图，这个 id 在 QML 文件中组合。
   @param id            图像索引(0 ~ frameCount - 1)
   @param size          图像的原始大小，有需要时可传出
   @param requestedSize 请求的图像大小
   @return 读取的图像数据

   @note 当前需要读取多页图的图像格式仅为 *.tif ，通过默认 QImageReader 即可读取，
        后续其它格式考虑在 LibUnionImage_NameSpace 中添加新的接口。
 */
QImage ThumbnailProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    // 解析id，获取当前读取的文件和图片索引
    QString tempPath;
    int frameIndex;
    parseProviderID(id, tempPath, frameIndex);

    // 判断缓存中是否存在缩略图
    if (ThumbnailCache::instance()->contains(tempPath, frameIndex)) {
        return ThumbnailCache::instance()->get(tempPath, frameIndex);
    }

    QImage image;
    if (frameIndex) {
        image = readMultiImage(tempPath, frameIndex);
    } else {
        image = readNormalImage(tempPath);
    }
    // 不存在缩略图信息，缓存图片
    QImage tmpImage = image.scaled(100, 100, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    ThumbnailCache::instance()->add(tempPath, frameIndex, tmpImage);

    if (size) {
        *size = image.size();
    }
    // 调整图像大小
    if (!image.isNull() && image.size() != requestedSize && requestedSize.isValid()) {
        image = image.scaled(requestedSize);
    }
    return image;
}

/**
   @brief 调用 requestImage() 获取图形信息，返回格式为 QPixmap 。
 */
QPixmap ThumbnailProvider::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QPixmap::fromImage(requestImage(id, size, requestedSize));
}
