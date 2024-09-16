// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILLOAD_H
#define THUMBNAILLOAD_H

#include <QQuickImageProvider>
#include <QQuickWindow>
#include <QImageReader>
#include <QImage>
#include <QCache>
#include <QMutex>
#include <QThreadPool>

#include <deque>

//大图预览下的小图
class ThumbnailLoad : public QQuickImageProvider
{
public:
    explicit ThumbnailLoad();
    //获取缩略图
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);  //预留
    bool imageIsNull(const QString &path);

    // 移除缓存的缩略图信息
    void removeImageCache(const QString &path);

    QMutex m_mutex;
    QImage m_Img;                       // 当前图片
    QMap<QString, QImage> m_imgMap;     // 缩略图缓存
};

//大图预览的大图
class ViewLoad : public QQuickImageProvider
{
public:
    explicit ViewLoad();
    //获取缩略图
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize);
    QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize);  //预留

    //获得当前图片的宽和高
    int getImageWidth(const QString &path);
    int getImageHeight(const QString &path);
    double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight, bool bReverse = false);

    // 移除缓存的图片大小信息
    void removeImageCache(const QString &path);
    // 重新加载图片大小信息
    void reloadImageCache(const QString &path);

    QMutex                  m_mutex;
    QImage                  m_Img;          // 当前图片
    QString                 m_currentPath;  // 加载路径
    QMap<QString, QSize>    m_imgSizes;     // 图片大小
};

/**
 * @brief 提供用于 *.tif 等多页图的单独图像加载处理类
 *      通过分割传入的 id ，判断当前读取的文件的行数和图片索引。
 *      在 QML 中注册的标识为 "multiimage"
 * @warning QQuickImageProvider 派生的接口可能多线程调用，必须保证实现函数是可重入的。
 */
class MultiImageLoad : public QQuickImageProvider
{
public:
    explicit MultiImageLoad();

    // 请求加载图片，获取图片加载信息
    virtual QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;
    virtual QPixmap requestPixmap(const QString &id, QSize *size, const QSize &requestedSize) override;

    // 获得当前图片的宽和高
    int getImageWidth(const QString &path, int frameIndex);
    int getImageHeight(const QString &path, int frameIndex);
    // 获取当前图片和适应窗口的缩放比值
    double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight, int frameIndex);

    // 移除缓存的图片大小信息
    void removeImageCache(const QString &path);

private:
    QMutex              m_mutex;
    QImageReader        m_imageReader;      // 图像读取类

    // 缓存图片信息
    struct CacheImage {
        QImage  imgThumbnail;   // 图片缩略图
        QSize   originSize;     // 原始图片大小

        explicit CacheImage(const QImage &img);
    };
    QCache<QPair<QString, int>, CacheImage> m_imageCache;   // 缩略图缓存(默认最多缓存256组图像)
};

//缩略图
class ImagePublisher : public QQuickImageProvider
{
    Q_OBJECT

public:
    explicit ImagePublisher(QObject *parent = nullptr);

    //切换图片显示状态
    Q_INVOKABLE void switchLoadMode();

    //获得当前图片显示的状态
    Q_INVOKABLE int getLoadMode();

protected:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    //图片裁剪策略
    QImage clipToRect(const QImage &src);
    QImage addPadAndScaled(const QImage &src);

    //加载模式控制，requestImage是由QML引擎多线程调用，此处需要采用原子锁，防止崩溃
    std::atomic_int m_loadMode;

    //图片缓存操作
    QImage getImage(const QUrl &url);

    //图片缓存buffer
    //first: url, second: images
    QMutex imageBuffer_mutex;
    static constexpr size_t QUEUE_MAX_LEN = 150;
    std::deque<std::pair<QString, std::vector<QImage>>> imageBuffer;

    QPixmap m_default;//图片默认图片
    QPixmap m_videoDefault;//视频默认图片
    QPixmap m_damaged;//损坏图片
    QImage m_whiteImage;
    QImage m_defaultImage;
    QImage m_videoDefaultImage;
    QImage m_damagedImage;
};

//聚合图
class CollectionPublisher : public QQuickImageProvider
{

    enum ImageSize {
        ImageSize_Full = 0,
        ImageSize_Half,
        ImageSize_Quarter,
        ImageSize_SplitBig,
        ImageSize_Split_Quarter,
        ImageSize_Split_Fifth,
    };

public:
    explicit CollectionPublisher();

protected:
    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    //图片输出尺寸（黄金矩形）
    static constexpr int outputWidth  = 1000;
    static constexpr int outputHeight = 618;

    //图片裁剪策略
    QImage createYearImage(const QString &year); //生成年视图
    QImage createMonthCellImage(const QString &path, const CollectionPublisher::ImageSize &sizeType);

    //辅助裁剪函数
    QImage clipHelper(const QImage &image, int width, int height);
};

//异步缩略图_start
class AsyncImageResponseAlbum : public QQuickImageResponse, public QRunnable
{
public:
    AsyncImageResponseAlbum(const QString &id, const QSize &requestedSize)
        : m_id(id), m_requestedSize(requestedSize)
    {
        setAutoDelete(false);
    }

    QQuickTextureFactory *textureFactory() const override
    {
        return QQuickTextureFactory::textureFactoryForImage(m_image);
    }

    void run() override;

    void setLoadMode(int mod)
    {
        m_loadMode = mod;
    }

private:
    //图片裁剪策略
    QImage clipToRect(const QImage &src);
    QImage addPadAndScaled(const QImage &src);

private:
    QString m_id;
    QSize m_requestedSize;
    QImage m_image;

    int m_loadMode;
};

class AsyncImageProviderAlbum : public QQuickAsyncImageProvider
{
    Q_OBJECT
public:
    explicit AsyncImageProviderAlbum(QObject *parent = nullptr);

    QQuickImageResponse *requestImageResponse(const QString &id, const QSize &requestedSize) override
    {
        AsyncImageResponseAlbum *response = new AsyncImageResponseAlbum(id, requestedSize);
        response->setLoadMode(m_loadMode);
        pool.start(response);
        return response;
    }

    //切换图片显示状态
    Q_INVOKABLE void switchLoadMode();

    //获得当前图片显示的状态
    Q_INVOKABLE int getLoadMode();

private:
    //加载模式控制，requestImage是由QML引擎多线程调用，此处需要采用原子锁，防止崩溃
    std::atomic_int m_loadMode;

    QThreadPool pool;
};
//异步缩略图_end

class LoadImage : public QObject
{
    Q_OBJECT
public:
    // 多页图帧号类型，Invalid 表示非多页图
    enum FrameType { Invalid = -1 };

    explicit LoadImage(QObject *parent = nullptr);

    ThumbnailLoad   *m_pThumbnail{nullptr};
    ViewLoad        *m_viewLoad{nullptr};
    MultiImageLoad  *m_multiLoad{nullptr};

    ImagePublisher  *m_publisher{nullptr};
    CollectionPublisher *m_collectionPublisher{nullptr};
    AsyncImageProviderAlbum  *m_asynImageProvider{nullptr};

    Q_INVOKABLE double getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight);
    Q_INVOKABLE bool imageIsNull(const QString &path);
    //获得当前图片的宽和高
    Q_INVOKABLE int getImageWidth(const QString &path);
    Q_INVOKABLE int getImageHeight(const QString &path);
    //获得宽高比例
    Q_INVOKABLE double getrealWidthHeightRatio(const QString &path);

    // 设置当前的多页图索引，-1表示当前非多页图
    Q_INVOKABLE void setMultiFrameIndex(int index = Invalid);
    // 设置是否互换宽度高度值(旋转图片时使用)
    Q_INVOKABLE void setReverseHeightWidth(bool b);

    //加载路径
    QString m_path;

public slots:
    //加载多张
    void loadThumbnails(const QStringList list);
    //加载一张
    void loadThumbnail(const QString path);
    //缩略图裁切接口-预留
    void catThumbnail(const QStringList &list);

    // 图片文件变更时清除缓存信息
    void onImageFileChanged(const QString &path, bool isMultiImage = false, bool isExist = false);

signals:
    //通知QML刷新
    void callQmlRefeshImg();

private:
    int     m_FrameIndex = Invalid;
    bool    m_bReverseHeightWidth = false;  // 设置当前是否互换宽度高度值(旋转图片时使用)
};

#endif // THUMBNAILLOAD_H
