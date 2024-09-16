// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "printimageloader.h"
#include "unionimage/unionimage.h"

#include <QImageReader>
#include <QtConcurrent>

static const int s_SingleFrame = -1;

/**
   @class PrintImageLoader
   @brief 打印图片数据加载器，图像加载经过如下流程
    1. 预加载：过滤非图片文件，简单判断文件是否存在，文件是否有读取权限2，排序加载图片文件，多页图、动图将按帧号展开；
    2. 加载：加载图片数据，多页图、动图会按帧号顺序加载。
    图片可同步/异步加载。
 */
PrintImageLoader::PrintImageLoader(QObject *parent)
    : QObject(parent)
{
    connect(this, &PrintImageLoader::asyncLoadError, this, &PrintImageLoader::onLoadError);
}

PrintImageLoader::~PrintImageLoader()
{
    if (isLoading()) {
        cancel();
    }
}

/**
   @brief 加载图片列表 \a fileLists ，默认设置 async ，使用异步加载；反之同步加载图片数据。
        如果同步加载失败、传入数据为空、当前仍在加载状态，返回false. 异步加载需要等待
        loadFinished() 加载完成信号。
        加载失败会自动清空缓存。
 */
bool PrintImageLoader::loadImageList(const QStringList &fileList, bool async)
{
    if (fileList.isEmpty() || loaderState != Stopped) {
        return false;
    }

    qInfo() << QString("Start load print images, async: %1").arg(async);
    loadData.clear();
    loaderState = Preloading;

    if (async) {
        // 开始异步预加载
        asyncPreload(fileList);
    } else {
        if (!syncPreload(fileList)) {
            return false;
        }

        loaderState = Loading;
        if (!syncLoad(loadData)) {
            return false;
        }

        loaderState = Stopped;
        Q_EMIT loadFinished(false, {});
    }

    return true;
}

/**
   @return 当前是否处于加载状态
 */
bool PrintImageLoader::isLoading()
{
    return loaderState != Stopped;
}

/**
   @brief 如果是异步加载，将被清空数据
 */
void PrintImageLoader::cancel()
{
    qInfo() << "Print image cancel triggerd.";

    if (preloadWatcher.isRunning()) {
        disconnect(&preloadWatcher, &QFutureWatcherBase::finished, this, &PrintImageLoader::onAsyncLoadFinished);
        preloadWatcher.cancel();
        preloadWatcher.waitForFinished();
    }
    if (loadWatcher.isRunning()) {
        disconnect(&loadWatcher, &QFutureWatcherBase::finished, this, &PrintImageLoader::onAsyncLoadFinished);
        loadWatcher.cancel();
        loadWatcher.waitForFinished();
    }

    loadData.clear();
    loaderState = Stopped;
}

/**
   @return 提取当前取得的预载信息，缓存的数据将被清空，数据加载过程将不可读取
 */
PrintDataList PrintImageLoader::takeLoadData()
{
    if (loaderState != Stopped || preloadWatcher.isRunning() || loadWatcher.isRunning()) {
        qCritical() << qPrintable("Read load data while async load thread still running!");
        return {};
    }

    PrintDataList tmp = std::move(loadData);
    return tmp;
}

/**
   @brief 判断传入文件 \a filePath 是否为图片文件，及图片是否有权限读取
   @note 当前系统环境大于 1060 时，文管不会通过 mimetype 过滤，需要手动过滤传入文件类型。
        如果格式不支持，则返回空列表；如果有准确错误，则标注错误类型并返回。
 */
PrintDataList PrintImageLoader::preloadImageData(const QString &filePath)
{
    // 判断文件是否存在，是否有权限读取
    QFileInfo info(filePath);
    if (!info.exists()) {
        PrintImageData::Ptr notExistsPtr(new PrintImageData(filePath));
        notExistsPtr->state = NotExists;
        return {notExistsPtr};
    }

    if (!info.permission(QFile::ReadUser)) {
        PrintImageData::Ptr permissionPtr(new PrintImageData(filePath));
        permissionPtr->state = NoPermission;
        return {permissionPtr};
    }

    // 根据文件类型区分处理
    PrintDataList dataList;
    const imageViewerSpace::ImageType type = LibUnionImage_NameSpace::getImageType(filePath);
    switch (type) {
        case imageViewerSpace::ImageTypeSvg:
            Q_FALLTHROUGH();
        case imageViewerSpace::ImageTypeStatic: {
            // 单张图片处理
            dataList.append(PrintImageData::Ptr(new PrintImageData(filePath)));
            break;
        }

        case imageViewerSpace::ImageTypeDynamic:
            Q_FALLTHROUGH();
        case imageViewerSpace::ImageTypeMulti: {
            // Note: 由于 QGifHandler 没有实现 jumpToImage() 接口，因此动图直接进行加载
            dataList = preloadMultiImage(filePath, bool(imageViewerSpace::ImageTypeDynamic == type));
            break;
        }

        case imageViewerSpace::ImageTypeDamaged: {
            PrintImageData::Ptr damangePtr(new PrintImageData(filePath));
            damangePtr->state = ContentError;
            dataList.append(damangePtr);
            break;
        }

        default:
            return {};
    }

    return dataList;
}

/**
   @brief 预读取多帧图片文件 \a filePath (动图/多页图)， \a directLoad 一般用于动图，在预加载过程中
        直接读取文件数据。部分动图格式特殊，需要连续读取，不能跳转帧号，拆分读取性能损耗较大。
 */
PrintDataList PrintImageLoader::preloadMultiImage(const QString &filePath, bool directLoad)
{
    PrintDataList dataList;

    // 多页图，动图，展开处理，-1为读取错误
    QImageReader reader(filePath);
    int frames = reader.imageCount();
    if (-1 == frames) {
        PrintImageData::Ptr damangePtr(new PrintImageData(filePath));
        damangePtr->state = ContentError;
        dataList.append(damangePtr);
        return dataList;
    }

    // 动图将直接加载数据
    if (directLoad) {
        try {
            if (0 == frames) {
                // 特殊处理，mng等动图图片，默认状态没有更新 imageCount() 仅在所有数据读取完成后更新
                int index = 0;
                while (reader.canRead()) {
                    QImage tmpImage = reader.read();
                    if (tmpImage.isNull()) {
                        // 下一帧无法继续读取，认为读取完成，没有数据的存在异常
                        if (dataList.isEmpty()) {
                            PrintImageData::Ptr damangePtr(new PrintImageData(filePath));
                            damangePtr->state = ContentError;
                            dataList.append(damangePtr);
                            return dataList;
                        }
                    }

                    PrintImageData::Ptr imagePtr(new PrintImageData(filePath, index++));
                    imagePtr->data = tmpImage;
                    imagePtr->state = Loaded;
                    dataList.append(imagePtr);
                    reader.jumpToNextImage();

                    // 帧数更新或超过最大帧限制(防止循环读取)
                    static int s_loopFrameLimit = 1024;
                    if ((0 != reader.imageCount()) || (index > s_loopFrameLimit)) {
                        break;
                    }
                }

            } else {
                for (int i = 0; i < frames; ++i) {
                    PrintImageData::Ptr imagePtr(new PrintImageData(filePath, i));
                    imagePtr->data = reader.read();
                    imagePtr->state = Loaded;
                    dataList.append(imagePtr);
                }
            }

        } catch (const std::exception &e) {
            // 图片读取，考虑未界定异常
            qCritical() << qPrintable("Exception: load dynamic image failed!") << qPrintable(e.what());
            PrintImageData::Ptr damangePtr(new PrintImageData(filePath));
            damangePtr->state = ContentError;
            dataList.clear();
            dataList.append(damangePtr);
            return dataList;
        }

    } else {
        // 多页图在后续流程加载
        for (int i = 0; i < frames; ++i) {
            dataList.append(PrintImageData::Ptr(new PrintImageData(filePath, i)));
        }
    }

    return dataList;
}

/**
   @brief 加载图片数据，并将图片数据写入到 \a imagePtr 中，如果图片不存在
        或读取的文件数据为空，将返回false。
 */
bool PrintImageLoader::loadImageData(PrintImageData::Ptr &imagePtr)
{
    // 动图数据在预加载流程加载
    if (Loaded == imagePtr->state) {
        return true;
    }

    if (!QFileInfo::exists(imagePtr->filePath)) {
        imagePtr->state = NotExists;
        return false;
    }

    try {
        if (s_SingleFrame == imagePtr->frame) {
            QString errorMsg;
            if (!LibUnionImage_NameSpace::loadStaticImageFromFile(imagePtr->filePath, imagePtr->data, errorMsg)) {
                qWarning() << QString("Load image failed: %1").arg(errorMsg);
                imagePtr->state = ContentError;
                return false;
            }
        } else {
            QImageReader reader(imagePtr->filePath);
            // jumpToImage 可能返回 false, 但数据正常读取
            reader.jumpToImage(imagePtr->frame);
            if (!reader.canRead()) {
                qWarning() << QString("Load multi frame image failed(jump to image): %1").arg(reader.errorString());
                imagePtr->state = ContentError;
                return false;
            }

            imagePtr->data = reader.read();
            if (imagePtr->data.isNull()) {
                qWarning() << QString("Load multi frame image failed: %1").arg(reader.errorString());
                imagePtr->state = ContentError;
                return false;
            }
        }

    } catch (const std::exception &e) {
        // 图片读取，考虑未界定异常
        qCritical() << qPrintable("Exception: load image failed!") << qPrintable(e.what());
        imagePtr->state = ContentError;
        return false;
    }

    imagePtr->state = Loaded;
    return true;
}

/**
   @brief 同步预载文件列表 \a fileLists 并返回是否加载成功，加载失败将自动清空数据
 */
bool PrintImageLoader::syncPreload(const QStringList &fileList)
{
    for (const QString &filePath : fileList) {
        auto result = preloadImageData(filePath);
        if (!result.isEmpty() && !(result.first()->state == Normal || result.first()->state == Loaded)) {
            onLoadError(filePath);
            return false;
        }

        loadData.append(result);
    }

    return true;
}

/**
   @brief 使用 QtConcurrent 并行预载 \a fileLists 文件，成功预载后自动触发加载 asyncLoad()
   @sa onAsyncLoadFinished(), asyncLoad()
 */
void PrintImageLoader::asyncPreload(const QStringList &fileList)
{
    // 默认 lambda 捕获没带 result_type 信息，用 std::function 包装
    std::function<PrintDataList(const QString &)> preloadMapedFunc = [this](const QString &filePath) -> PrintDataList {
        PrintDataList parsed = PrintImageLoader::preloadImageData(filePath);
        // 异常文件，通知停止处理，处于不同线程
        if (!parsed.isEmpty() && !(parsed.first()->state == Normal || parsed.first()->state == Loaded)) {
            Q_EMIT this->asyncLoadError(filePath);
        }

        return parsed;
    };

    // 按*顺序*插入数据
    QFuture<PrintDataList> asyncData = QtConcurrent::mappedReduced<PrintDataList>(
        fileList,
        preloadMapedFunc,
        [](PrintDataList &result, const PrintDataList &parsed) { result.append(parsed); },
        (QtConcurrent::OrderedReduce | QtConcurrent::SequentialReduce));

    // 仅在调用时绑定信号
    connect(&preloadWatcher, &QFutureWatcherBase::finished, this, &PrintImageLoader::onAsyncLoadFinished);
    preloadWatcher.setFuture(asyncData);
}

/**
   @brief 异步读取预载的图片数据 \a dataList 并返回是否正常加载，加载失败将自动清空数据
 */
bool PrintImageLoader::syncLoad(PrintDataList &dataList)
{
    for (PrintImageData::Ptr &dataPtr : dataList) {
        if (!loadImageData(dataPtr)) {
            onLoadError(dataPtr->filePath);
            return false;
        }
    }

    return true;
}

/**
   @brief 异步读取预载的图片数据 \a dataList
   @sa onAsyncLoadFinished(), asyncPreload()
 */
void PrintImageLoader::asyncLoad(PrintDataList &dataList)
{
    QFuture<void> asyncData = QtConcurrent::map(dataList, [this](PrintImageData::Ptr &dataPtr) {
        if (!PrintImageLoader::loadImageData(dataPtr)) {
            Q_EMIT this->asyncLoadError(dataPtr->filePath);
        }
    });

    // 仅在调用时绑定信号
    connect(&loadWatcher, &QFutureWatcherBase::finished, this, &PrintImageLoader::onAsyncLoadFinished);
    loadWatcher.setFuture(asyncData);
}

/**
   @brief 异步数据加载完成，记录预载的文件信息并清理缓存
 */
void PrintImageLoader::onAsyncLoadFinished()
{
    switch (loaderState) {
        case Preloading:
            qInfo() << "Async print image preload finished.";
            // 清理缓存的数据, 继续加载数据
            loadData = preloadWatcher.result();
            disconnect(&preloadWatcher, &QFutureWatcherBase::finished, this, &PrintImageLoader::onAsyncLoadFinished);
            preloadWatcher.setFuture(QFuture<PrintDataList>());
            loaderState = Loading;
            asyncLoad(loadData);
            break;

        case Loading:
            qInfo() << "Async print image load finished.";
            disconnect(&loadWatcher, &QFutureWatcherBase::finished, this, &PrintImageLoader::onAsyncLoadFinished);
            loaderState = Stopped;
            Q_EMIT loadFinished(false, {});
            break;

        default:
            qWarning() << QString("Async load state error %1").arg(loaderState);
            break;
    }
}

/**
   @brief 文件 \a fileName 加载错误，可能文件损坏或无读取权限
 */
void PrintImageLoader::onLoadError(const QString &fileName)
{
    // 多线程下可能被异步触发多次，过滤
    if (isLoading()) {
        cancel();
        Q_EMIT loadFinished(true, fileName);
    }
}
