// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRINTIMAGELOADER_H
#define PRINTIMAGELOADER_H

#include <QImage>
#include <QObject>
#include <QSharedPointer>
#include <QFutureWatcher>

enum ImageFileState {  // 图片文件状态
    Normal,
    Loaded,        // 图片已加载
    NoPermission,  // 文件无访问权限
    ContentError,  // 文件错误
    NotExists,     // 文件不存在
};

// 打印文件数据
struct PrintImageData
{
    typedef QSharedPointer<PrintImageData> Ptr;

    QString filePath;               // 文件路径
    int frame = -1;                 // 文件帧号，用于多页图, -1表示单页图
    ImageFileState state = Normal;  // 文件状态
    QImage data;                    // 文件数据

    explicit PrintImageData(const QString &path, int f = -1)
        : filePath(path)
        , frame(f)
    {
    }
};
typedef QList<PrintImageData::Ptr> PrintDataList;

// 数据加载器，含预载和加载数据流程
class PrintImageLoader : public QObject
{
    Q_OBJECT
public:
    explicit PrintImageLoader(QObject *parent = nullptr);
    ~PrintImageLoader() override;

    bool loadImageList(const QStringList &fileList, bool async = true);
    bool isLoading();
    void cancel();
    PrintDataList takeLoadData();

    Q_SIGNAL void loadFinished(bool error, const QString &errorString);

    static PrintDataList preloadImageData(const QString &filePath);
    static PrintDataList preloadMultiImage(const QString &filePath, bool directLoad = false);
    static bool loadImageData(PrintImageData::Ptr &imagePtr);

private:
    // 预载入和数据过滤
    bool syncPreload(const QStringList &fileList);
    void asyncPreload(const QStringList &fileList);
    // 数据加载
    bool syncLoad(PrintDataList &dataList);
    void asyncLoad(PrintDataList &dataList);

    // 内部消息
    Q_SIGNAL void asyncLoadError(const QString &fileName);
    Q_SLOT void onAsyncLoadFinished();
    Q_SLOT void onLoadError(const QString &fileName);

private:
    enum LoaderState { Stopped, Preloading, Loading };
    LoaderState loaderState = Stopped;  // 加载器状态
    PrintDataList loadData;             // 加载数据，Note:在异步加载过程中不可读取
    QFutureWatcher<PrintDataList> preloadWatcher;
    QFutureWatcher<void> loadWatcher;

    Q_DISABLE_COPY(PrintImageLoader)
};

#endif  // PRINTIMAGELOADER_H
