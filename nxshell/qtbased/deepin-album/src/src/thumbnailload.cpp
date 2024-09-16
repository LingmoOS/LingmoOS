// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "thumbnailload.h"
#include "unionimage/unionimage.h"
#include "configsetter.h"
#include "imageengine/movieservice.h"
#include "dbmanager/dbmanager.h"
#include <QPainter>

const QString SETTINGS_GROUP = "Thumbnail";
const QString SETTINGS_DISPLAY_MODE = "ThumbnailMode";
const int THUMBNAIL_MAX_SIZE = 180;

ThumbnailLoad::ThumbnailLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

QImage ThumbnailLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(id);
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    if (!m_imgMap.keys().contains(tempPath)) {
        LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
        // 保存图片比例缩放
        QImage reImg = Img.scaled(100, 100, Qt::KeepAspectRatio);
        m_imgMap[tempPath] = reImg;
        return reImg;
    } else {
        return m_imgMap[tempPath];
    }
}

QPixmap ThumbnailLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(id);
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
    return QPixmap::fromImage(Img);
}

bool ThumbnailLoad::imageIsNull(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);

    QMutexLocker _locker(&m_mutex);
    if (m_imgMap.find(tempPath) != m_imgMap.end()) {
        return m_imgMap[tempPath].isNull();
    }

    return false;
}

/**
 * @brief 移除缓存的 \a path 文件图像缩略图信息
 */
void ThumbnailLoad::removeImageCache(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    QMutexLocker _locker(&m_mutex);
    m_imgMap.remove(tempPath);
}

LoadImage::LoadImage(QObject *parent) :
    QObject(parent)
{
    m_pThumbnail = new ThumbnailLoad();
    m_viewLoad = new ViewLoad();
    m_multiLoad = new MultiImageLoad();
    m_publisher = new ImagePublisher(this);
    m_collectionPublisher = new CollectionPublisher();
    m_asynImageProvider = new AsyncImageProviderAlbum();
}

double LoadImage::getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight)
{
    if (Invalid != m_FrameIndex) {
        return m_multiLoad->getFitWindowScale(path, WindowWidth, WindowHeight, m_FrameIndex);
    }

    return m_viewLoad->getFitWindowScale(path, WindowWidth, WindowHeight, m_bReverseHeightWidth);
}

bool LoadImage::imageIsNull(const QString &path)
{
    return m_pThumbnail->imageIsNull(path);
}

int LoadImage::getImageWidth(const QString &path)
{
    if (Invalid != m_FrameIndex) {
        return m_multiLoad->getImageWidth(path, m_FrameIndex);
    }

    return m_bReverseHeightWidth ? m_viewLoad->getImageHeight(path)
           : m_viewLoad->getImageWidth(path);
}

int LoadImage::getImageHeight(const QString &path)
{
    if (Invalid != m_FrameIndex) {
        return m_multiLoad->getImageHeight(path, m_FrameIndex);
    }

    return m_bReverseHeightWidth ? m_viewLoad->getImageWidth(path)
           : m_viewLoad->getImageHeight(path);
}

double LoadImage::getrealWidthHeightRatio(const QString &path)
{
    if (Invalid != m_FrameIndex) {
        double width = double(m_multiLoad->getImageWidth(path, m_FrameIndex));
        double height = double(m_multiLoad->getImageHeight(path, m_FrameIndex));
        return width / height;
    }

    double width = double(m_viewLoad->getImageWidth(path));
    double height = double(m_viewLoad->getImageHeight(path));

    return m_bReverseHeightWidth ? height / width
           : width / height;
}

void LoadImage::setMultiFrameIndex(int index)
{
    m_FrameIndex = index;

}

/**
 * @brief 设置当前是否反转图片宽高设置
 */
void LoadImage::setReverseHeightWidth(bool b)
{
    m_bReverseHeightWidth = b;
}

void LoadImage::loadThumbnail(const QString path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    qDebug() << "----path--" << tempPath;
    QImage Img;
    QString error;
    if (LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error)) {
        m_pThumbnail->m_Img = Img;
        emit callQmlRefeshImg();
    } else {
        qDebug() << "load failded,the error is:" << error;
    }
}

void LoadImage::catThumbnail(const QStringList &list)
{
    if (list.size() < 1) {
        return;
    }
    for (QString path : list) {
        QString imgPath = path;

        if (imgPath.startsWith("file://")) {
            imgPath = LibUnionImage_NameSpace::localPath(imgPath);
        }

        QImage tImg(imgPath);
        //保持横纵比裁切
        if (abs((tImg.width() - tImg.height()) * 10 / tImg.width()) >= 1) {
            QRect rect = tImg.rect();
            int x = rect.x() + tImg.width() / 2;
            int y = rect.y() + tImg.height() / 2;
            if (tImg.width() > tImg.height()) {
                x = x - tImg.height() / 2;
                y = 0;
                tImg = tImg.copy(x, y, tImg.height(), tImg.height());
            } else {
                y = y - tImg.width() / 2;
                x = 0;
                tImg = tImg.copy(x, y, tImg.width(), tImg.width());
            }
        }
        //压缩画质
        if (0 != tImg.height() && 0 != tImg.width() && (tImg.height() / tImg.width()) < 10 && (tImg.width() / tImg.height()) < 10) {
            if (tImg.height() != /*m_height*/100 || tImg.width() != /*m_with*/100) {
                if (tImg.height() >= tImg.width()) {
                    tImg = tImg.scaledToWidth(/*m_with*/100,  Qt::FastTransformation);
                } else if (tImg.height() <= tImg.width()) {
                    tImg = tImg.scaledToHeight(/*m_height*/100,  Qt::FastTransformation);
                }
            }
        }
    }
}

/**
 * @brief 接收图片变更信号，当前图片文件被移动、替换、删除时触发，清除图片相关的缓存信息
 * @param path          图片文件路径
 * @param isMultiImage  是否为多页图，多页图存在特殊处理
 * @param isExist       修改后的文件是否存在
 *
 * @note 不清除缩略图信息，用于提示文件变更时获取。
 * @threadsafe
 */
void LoadImage::onImageFileChanged(const QString &path, bool isMultiImage, bool isExist)
{
    if (isMultiImage) {
        m_multiLoad->removeImageCache(path);
    }

    // 判断变更后文件是否存在，若存在，重新加载缩略图(防止文件被替换), 重新获取图像大小信息
    if (isExist) {
        m_pThumbnail->removeImageCache(path);
        QSize size, requestSize;
        m_pThumbnail->requestImage(path, &size, requestSize);

        // 文件替换，重新获取图片大小信息
        m_viewLoad->reloadImageCache(path);
    } else {
        // 文件移除，移除图片信息
        m_viewLoad->removeImageCache(path);
    }
}

void LoadImage::loadThumbnails(const QStringList list)
{
    QImage Img;
    QString error;
    for (QString path : list) {
        loadThumbnail(path);
    }
}


ViewLoad::ViewLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{

}

QImage ViewLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(id);
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    if (tempPath == m_currentPath) {
        if (m_Img.size() != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
            m_Img = m_Img.scaled(requestedSize);
        }
        return m_Img;
    }
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
    m_imgSizes[tempPath] = Img.size() ;
    m_Img = Img;
    m_currentPath = tempPath;
    if (m_Img.size() != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
        Img = m_Img.scaled(requestedSize);
    }

    return Img;
}

QPixmap ViewLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(id);
    QImage Img;
    QString error;

    QMutexLocker _locker(&m_mutex);
    if (tempPath == m_currentPath) {
        return QPixmap::fromImage(m_Img);
    }
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);
    m_imgSizes[tempPath] = Img.size();
    m_Img = Img;
    m_currentPath = tempPath;
    return QPixmap::fromImage(Img);
}

int ViewLoad::getImageWidth(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);

    QMutexLocker _locker(&m_mutex);
    return m_imgSizes[tempPath].width();
}

int ViewLoad::getImageHeight(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);

    QMutexLocker _locker(&m_mutex);
    return m_imgSizes[tempPath].height();
}

double ViewLoad::getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight, bool bReverse)
{
    double scale = 0.0;
    double width = getImageWidth(path);
    double height = getImageHeight(path);
    double scaleWidth = width / WindowWidth;
    double scaleHeight = height / WindowHeight;

    if (scaleWidth > scaleHeight) {
        scale = scaleWidth;
    } else {
        scale = scaleHeight;
    }

    return scale;
}

/**
 * @brief 移除缓存的 \a path 文件图像大小信息
 */
void ViewLoad::removeImageCache(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);

    QMutexLocker _locker(&m_mutex);
    m_imgSizes.remove(tempPath);

    // 为当前展示的图片，移除缓存的信息
    if (tempPath == m_currentPath) {
        m_currentPath.clear();
    }
}

/**
 * @brief 文件被替换等操作后，重新获取 \a path 文件对应的图像大小信息
 */
void ViewLoad::reloadImageCache(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    QImage Img;
    QString error;
    LibUnionImage_NameSpace::loadStaticImageFromFile(tempPath, Img, error);

    QMutexLocker _locker(&m_mutex);
    m_imgSizes[tempPath] = Img.size();
    if (tempPath == m_currentPath) {
        m_Img = Img;
    }
}


MultiImageLoad::MultiImageLoad()
    : QQuickImageProvider(QQuickImageProvider::Image)
{
    m_imageCache.setMaxCost(256);
}

/**
 * @brief 外部请求多页图中指定帧的图像，指定帧号通过传入的 \a id 进行区分。
 *      \a id 格式为 \b{图像路径#frame_帧号_缩略图标识} ，例如 "/home/tmp.tif#frame_3_thumbnail" ，
 *      表示 tmp.tif 图像文件的第四帧图像缩略图。_thumbnail 可移除，表示需要完整图片
 *      这个 id 在 QML 文件中组合。
 * @param id            图像索引(0 ~ frameCount - 1)
 * @param size          图像的原始大小，有需要时可传出
 * @param requestedSize 请求的图像大小
 * @return 读取的图像数据
 *
 * @note 当前需要读取多页图的图像格式仅为 *.tif ，通过默认 QImageReader 即可读取，
 *      后续其它格式考虑在 LibUnionImage_NameSpace 中添加新的接口。
 */
QImage MultiImageLoad::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    Q_UNUSED(size)
    // 拆分id，获取当前读取的文件和图片索引
    static const QString s_tagFrame = "#frame_";
    static const QString s_tagThumbnail = "_thumbnail";
    QString checkId = id;
    bool useThumbnail = checkId.endsWith(s_tagThumbnail);
    if (useThumbnail) {
        checkId.chop(s_tagThumbnail.size());
    }

    // 从后向前查询索引标识
    int index = checkId.lastIndexOf(QRegularExpression(QString("%1\\d+$").arg(s_tagFrame)));
    if (-1 == index) {
        return QImage();
    }
    QString path = checkId.left(index);
    // 移除 "#frame_" 字段
    int frame = checkId.right(checkId.size() - index - s_tagFrame.size()).toInt();

    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    QImage img;

    // 数据变更前加锁
    QMutexLocker _locker(&m_mutex);
    if (tempPath != m_imageReader.fileName()
            || !m_imageReader.canRead()) {
        // 重新设置图像读取类
        m_imageReader.setFileName(tempPath);
    }

    // 获取图像数据
    auto key = qMakePair(tempPath, frame);
    bool hasThumbnail = m_imageCache.contains(key);
    if (hasThumbnail && useThumbnail) {
        // 返回缓存缩略图信息
        img = m_imageCache.object(key)->imgThumbnail;
    } else {
        if (m_imageReader.jumpToImage(frame)) {
            // 读取图像数据
            img = m_imageReader.read();
            // 判断是否正常读取
            if (img.isNull()) {
                return img;
            }

            // 不存在缩略图信息，缓存图片
            if (!hasThumbnail) {
                m_imageCache.insert(key, new CacheImage(img));
            }
        }
    }

    // 调整图像大小
    if (!img.isNull() && img.size() != requestedSize && requestedSize.width() > 0 && requestedSize.height() > 0) {
        img = img.scaled(requestedSize);
    }
    return img;
}

/**
 * @brief 调用 requestImage() 获取图形信息，返回格式为 QPixmap 。
 */
QPixmap MultiImageLoad::requestPixmap(const QString &id, QSize *size, const QSize &requestedSize)
{
    return QPixmap::fromImage(requestImage(id, size, requestedSize));
}

/**
 * @return 根据传入的图片路径 \a path 和图片帧号索引 \a frameIndex , 返回图片的宽度值
 */
int MultiImageLoad::getImageWidth(const QString &path, int frameIndex)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    auto key = qMakePair(tempPath, frameIndex);

    QMutexLocker _locker(&m_mutex);
    CacheImage *cache = m_imageCache.object(key);
    if (cache) {
        return cache->originSize.width();
    }
    return 0;
}

/**
 * @return 根据传入的图片路径 \a path 和图片帧号索引 \a frameIndex , 返回图片的高度值
 */
int MultiImageLoad::getImageHeight(const QString &path, int frameIndex)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    auto key = qMakePair(tempPath, frameIndex);

    QMutexLocker _locker(&m_mutex);
    CacheImage *cache = m_imageCache.object(key);
    if (cache) {
        return cache->originSize.height();
    }
    return 0;
}

/**
 * @brief 根据传入的图片路径 \a path 和图片帧号索引 \a frameIndex ,
 *      返回图片相对适应窗口的缩放比值。
 * @param path          图片路径
 * @param WindowWidth   适应窗口宽度
 * @param WindowHeight  适应窗口高度
 * @param frameIndex    图片帧号索引
 * @return 图片相对适应窗口的缩放比值
 */
double MultiImageLoad::getFitWindowScale(const QString &path, double WindowWidth, double WindowHeight, int frameIndex)
{
    double scale = 0.0;
    double width = getImageWidth(path, frameIndex);
    double height = getImageHeight(path, frameIndex);
    double scaleWidth = width / WindowWidth;
    double scaleHeight = height / WindowHeight;

    if (scaleWidth > scaleHeight) {
        scale = scaleWidth;
    } else {
        scale = scaleHeight;
    }

    return scale;
}

/**
 * @brief 移除缓存的 \a path 文件图像大小信息
 * @note 需要考虑存在 *.tif 文件同名替换的问题，此情况下需要完全重新加载文件。
 *      此方式是否存在优化空间
 */
void MultiImageLoad::removeImageCache(const QString &path)
{
    QString tempPath = LibUnionImage_NameSpace::localPath(path);
    QMutexLocker _locker(&m_mutex);
    // 移除关联的图像
    QList<QPair<QString, int> > keys = m_imageCache.keys();
    for (auto itr = keys.begin(); itr != keys.end(); ++itr) {
        if (itr->first == tempPath) {
            m_imageCache.remove(*itr);
        }
    }
    // 移除图像读取类
    if (m_imageReader.fileName() == tempPath) {
        m_imageReader.setDevice(nullptr);
    }
}

MultiImageLoad::CacheImage::CacheImage(const QImage &img)
{
    // 缩略图大小
    static const QSize s_ThumbnailSize(100, 100);
    imgThumbnail = img.scaled(s_ThumbnailSize);
    originSize = img.size();
}

ImagePublisher::ImagePublisher(QObject *parent)
    : QQuickImageProvider(Image)
{
    //初始化的时候读取上次退出时的状态
    m_loadMode = LibConfigSetter::instance()->value(SETTINGS_GROUP, SETTINGS_DISPLAY_MODE, 0).toInt();
}

//切换加载策略
void ImagePublisher::switchLoadMode()
{
    switch (m_loadMode) {
    case 0:
        m_loadMode = 1;
        break;
    case 1:
        m_loadMode = 0;
        break;
    default:
        m_loadMode = 0;
        break;
    }

    //切完以后保存状态
    LibConfigSetter::instance()->setValue(SETTINGS_GROUP, SETTINGS_DISPLAY_MODE, m_loadMode.load());
}

int ImagePublisher::getLoadMode()
{
    return m_loadMode;
}

//将图片裁剪为方图，逻辑与原来一样
QImage ImagePublisher::clipToRect(const QImage &src)
{
    auto tImg = src;

    if (!tImg.isNull() && 0 != tImg.height() && 0 != tImg.width() && (tImg.height() / tImg.width()) < 10 && (tImg.width() / tImg.height()) < 10) {
        bool cache_exist = false;
        if (tImg.height() != THUMBNAIL_MAX_SIZE && tImg.width() != THUMBNAIL_MAX_SIZE) {
            if (tImg.height() >= tImg.width()) {
                cache_exist = true;
                tImg = tImg.scaledToWidth(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            } else if (tImg.height() <= tImg.width()) {
                cache_exist = true;
                tImg = tImg.scaledToHeight(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            }
        }
        if (!cache_exist) {
            if ((static_cast<float>(tImg.height()) / (static_cast<float>(tImg.width()))) > 3) {
                tImg = tImg.scaledToWidth(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            } else {
                tImg = tImg.scaledToHeight(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            }
        }
    }

    if (!tImg.isNull()) {
        int width = tImg.width();
        int height = tImg.height();
        if (abs((width - height) * 10 / width) >= 1) {
            QRect rect = tImg.rect();
            int x = rect.x() + width / 2;
            int y = rect.y() + height / 2;
            if (width > height) {
                x = x - height / 2;
                y = 0;
                tImg = tImg.copy(x, y, height, height);
            } else {
                y = y - width / 2;
                x = 0;
                tImg = tImg.copy(x, y, width, width);
            }
        }
    }

    return tImg;
}

//将图片按比例缩小
QImage ImagePublisher::addPadAndScaled(const QImage &src)
{
    auto result = src.convertToFormat(QImage::Format_RGBA8888);

    if (result.height() > result.width()) {
        result = result.scaledToHeight(THUMBNAIL_MAX_SIZE, Qt::SmoothTransformation);
    } else {
        result = result.scaledToWidth(THUMBNAIL_MAX_SIZE, Qt::SmoothTransformation);
    }

    return result;
}

//图片请求类
//警告：这个函数将会被多线程执行，需要确保它是可重入的
QImage ImagePublisher::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    //id的前几个字符是强制刷新用的，需要排除出去
    auto startIndex = id.indexOf('_') + 1;

    QUrl url(id.mid(startIndex));

    QString error;
    QImage image;
    LibUnionImage_NameSpace::loadStaticImageFromFile(LibUnionImage_NameSpace::localPath(url), image, error);

    //如果是视频，则采用视频加载
    if (LibUnionImage_NameSpace::isVideo(LibUnionImage_NameSpace::localPath(url))) {
        image = MovieService::instance()->getMovieCover(url);
    }
    if (m_loadMode == 0) {
        image = clipToRect(image);
    } else { //m_loadMode == 1
        image = addPadAndScaled(image);
    }

    if (size != nullptr) {
        *size = image.size();
    }

    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        return image.scaled(requestedSize, Qt::KeepAspectRatio);
    } else {
        return image;
    }
}

CollectionPublisher::CollectionPublisher()
    : QQuickImageProvider(Image)
{
}

//id: random_Y_2022_0 random_M_2022_6
QImage CollectionPublisher::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    auto tokens = id.split("_");

    QImage result;

    if (id.size() < 4) {
        return result;
    }

    auto type = tokens[1];
    if (type == "Y") {
        result = createYearImage(tokens[2]);
    } else if (type == "M") {
        CollectionPublisher::ImageSize imageSize = static_cast<CollectionPublisher::ImageSize>(tokens[3].toInt());
        result = createMonthCellImage(id.mid(id.indexOf("/")), imageSize);
    }

    if (size != nullptr) {
        *size = result.size();
    }
    if (requestedSize.width() > 0 && requestedSize.height() > 0) {
        return result.scaled(requestedSize, Qt::KeepAspectRatio);
    } else {
        return result;
    }
}

QImage CollectionPublisher::createYearImage(const QString &year)
{
    auto paths = DBManager::instance()->getYearPaths(year, 1);
    if (paths.isEmpty()) {
        return QImage();
    }
    auto picPath = paths.at(0);

    //TODO: 异常处理：裂图问题

    //加载原图
    QImage image;
    QString error;
    LibUnionImage_NameSpace::loadStaticImageFromFile(picPath, image, error);
    image = image.scaled(outputWidth, outputHeight, Qt::KeepAspectRatioByExpanding);

    return image;
}

QImage CollectionPublisher::createMonthCellImage(const QString &path, const CollectionPublisher::ImageSize &sizeType)
{
    // 计算图片尺寸
    QSize requestSize(outputWidth, outputHeight);
    if (ImageSize_Full == sizeType)
        requestSize = QSize(outputWidth, outputHeight);
    else if (ImageSize_Half == sizeType)
        requestSize = QSize(outputWidth / 2, outputHeight);
    else if (ImageSize_Quarter == sizeType)
        requestSize = QSize(outputWidth / 2, outputHeight / 2);
    else if (ImageSize_SplitBig == sizeType)
        requestSize = QSize(outputWidth, static_cast<int>(outputHeight * 0.618));
    else if (ImageSize_Split_Quarter == sizeType)
        requestSize = QSize(outputWidth / 4, static_cast<int>(outputHeight * (1 - 0.618)));
    else if (ImageSize_Split_Fifth == sizeType)
        requestSize = QSize(outputWidth / 5, static_cast<int>(outputHeight * (1 - 0.618)));

    //1.加载原图
    QImage image;
    QString error;
    if (LibUnionImage_NameSpace::isVideo(path))
        image = MovieService::instance()->getMovieCover(QUrl::fromLocalFile(path));
    else
        LibUnionImage_NameSpace::loadStaticImageFromFile(path, image, error);
    image = image.scaled(outputWidth, outputHeight, Qt::KeepAspectRatioByExpanding);

    // 2.根据比例裁剪
    image = clipHelper(image, requestSize.width(), requestSize.height());

    return image;
}

//KeepAspectRatioByExpanding，但是保留中间，Qt是裁的右侧或下侧
QImage CollectionPublisher::clipHelper(const QImage &image, int width, int height)
{
    QImage temp;
    double x = 0;
    double y = 0;

    double resizeW = 0;
    double resizeH = 0;
    double widthF = width;
    double heightF = height;
    if (image.width() > image.height()) {
        resizeH = heightF;
        resizeW = image.width() / (image.height() / heightF);
        if (resizeW < widthF) {
            resizeH = resizeH * widthF / resizeW;
            resizeW = widthF;
            y = (resizeH - heightF) / 2;
        } else {
            x = (resizeW - widthF) / 2;
        }
    } else {
        resizeW = widthF;
        resizeH = image.height() / (image.width() / widthF);
        if (resizeH < heightF) {
            resizeW = resizeW * heightF / resizeH;
            resizeH = heightF;
            x = (resizeW - widthF) / 2;
        } else {
            y = (resizeH - heightF) / 2;
        }
    }

    temp = image.scaled(static_cast<int>(resizeW), static_cast<int>(resizeH));

    QRect rect(static_cast<int>(x), static_cast<int>(y), width, height);
    QImage result(width, height, QImage::Format_RGB888);
    result = temp.copy(rect);
    return result;
}

void AsyncImageResponseAlbum::run()
{
    //id的前几个字符是强制刷新用的，需要排除出去
    auto startIndex = m_id.indexOf('_') + 1;

    QUrl url(m_id.mid(startIndex));

    QString error;
    LibUnionImage_NameSpace::loadStaticImageFromFile(LibUnionImage_NameSpace::localPath(url), m_image, error);

    //如果是视频，则采用视频加载
    if (LibUnionImage_NameSpace::isVideo(LibUnionImage_NameSpace::localPath(url))) {
        m_image = MovieService::instance()->getMovieCover(url);
    }

    if (m_loadMode == 0) {
        m_image = clipToRect(m_image);
    } else { //m_loadMode == 1
        m_image = addPadAndScaled(m_image);
    }

    if (m_requestedSize.width() > 0 && m_requestedSize.height() > 0) {
        m_image = m_image.scaled(m_requestedSize, Qt::KeepAspectRatio);
    }

    emit finished();
}

//将图片裁剪为方图，逻辑与原来一样
QImage AsyncImageResponseAlbum::clipToRect(const QImage &src)
{
    auto tImg = src;

    if (!tImg.isNull() && 0 != tImg.height() && 0 != tImg.width() && (tImg.height() / tImg.width()) < 10 && (tImg.width() / tImg.height()) < 10) {
        bool cache_exist = false;
        if (tImg.height() != THUMBNAIL_MAX_SIZE && tImg.width() != THUMBNAIL_MAX_SIZE) {
            if (tImg.height() >= tImg.width()) {
                cache_exist = true;
                tImg = tImg.scaledToWidth(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            } else if (tImg.height() <= tImg.width()) {
                cache_exist = true;
                tImg = tImg.scaledToHeight(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            }
        }
        if (!cache_exist) {
            if ((static_cast<float>(tImg.height()) / (static_cast<float>(tImg.width()))) > 3) {
                tImg = tImg.scaledToWidth(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            } else {
                tImg = tImg.scaledToHeight(THUMBNAIL_MAX_SIZE,  Qt::FastTransformation);
            }
        }
    }

    if (!tImg.isNull()) {
        int width = tImg.width();
        int height = tImg.height();
        if (abs((width - height) * 10 / width) >= 1) {
            QRect rect = tImg.rect();
            int x = rect.x() + width / 2;
            int y = rect.y() + height / 2;
            if (width > height) {
                x = x - height / 2;
                y = 0;
                tImg = tImg.copy(x, y, height, height);
            } else {
                y = y - width / 2;
                x = 0;
                tImg = tImg.copy(x, y, width, width);
            }
        }
    }

    return tImg;
}

//将图片按比例缩小
QImage AsyncImageResponseAlbum::addPadAndScaled(const QImage &src)
{
    auto result = src.convertToFormat(QImage::Format_RGBA8888);

    if (result.height() > result.width()) {
        result = result.scaledToHeight(THUMBNAIL_MAX_SIZE, Qt::SmoothTransformation);
    } else {
        result = result.scaledToWidth(THUMBNAIL_MAX_SIZE, Qt::SmoothTransformation);
    }

    return result;
}

AsyncImageProviderAlbum::AsyncImageProviderAlbum(QObject *parent)
    : QQuickAsyncImageProvider()
{
    //初始化的时候读取上次退出时的状态
    m_loadMode = LibConfigSetter::instance()->value(SETTINGS_GROUP, SETTINGS_DISPLAY_MODE, 0).toInt();
}

//切换加载策略
void AsyncImageProviderAlbum::switchLoadMode()
{
    switch (m_loadMode) {
    case 0:
        m_loadMode = 1;
        break;
    case 1:
        m_loadMode = 0;
        break;
    default:
        m_loadMode = 0;
        break;
    }

    //切完以后保存状态
    LibConfigSetter::instance()->setValue(SETTINGS_GROUP, SETTINGS_DISPLAY_MODE, m_loadMode.load());
}

int AsyncImageProviderAlbum::getLoadMode()
{
    return m_loadMode;
}
