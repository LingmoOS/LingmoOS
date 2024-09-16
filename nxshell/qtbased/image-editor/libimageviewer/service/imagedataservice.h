// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef IMAGEDATASERVICE_H
#define IMAGEDATASERVICE_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QMutex>
#include <QQueue>
#include <QThread>

#include "image-viewer_global.h"

class LibReadThumbnailThread;
class LibImageDataService: public QObject
{
    Q_OBJECT
public:
    static LibImageDataService *instance(QObject *parent = nullptr);
    explicit LibImageDataService(QObject *parent = nullptr);
    ~LibImageDataService();

    bool add(const QStringList &paths);
    bool add(const QString &path);
    QString pop();
    bool isRequestQueueEmpty();
    //获取全部图片数量
    int getCount();

    //读取缩略图到缓存map
    bool readThumbnailByPaths(const QString &thumbnailPath, const QStringList &files, bool remake);

    void addImage(const QString &path, const QImage &image);
    QImage getThumnailImageByPath(const QString &path);
    bool imageIsLoaded(const QString &path);

    void addMovieDurationStr(const QString &path, const QString &durationStr);
    QString getMovieDurationStrByPath(const QString &path);

    //设置当前窗口所有数据
    void setAllDataKeys(const QStringList &paths, bool single = false);

    //
    void setVisualIndex(int row);
    int getVisualIndex();

    //取消图片加载
    void stopReadThumbnail();

private slots:
signals:
    void sigeUpdateListview();
public:
private:
    static LibImageDataService *s_ImageDataService;
    QMutex m_queuqMutex;
    QList<QString> m_requestQueue;

    //图片数据锁
    QMutex m_imgDataMutex;
    QMap<QString, QImage> m_AllImageMap;
    QMap<QString, QString> m_movieDurationStrMap;
    QQueue<QString> m_imageKeys;
    int m_visualIndex = 0;//用户查找视图中的model index

    //图片读取线程
    std::vector<LibReadThumbnailThread *> readThreadGroup;
};


//缩略图读取线程
class LibReadThumbnailThread : public QThread
{
    Q_OBJECT

public:
    explicit LibReadThumbnailThread(QObject *parent = nullptr);
    ~LibReadThumbnailThread() override = default;
    void readThumbnail(QString m_path);
    void setQuit(bool quit);

    //判断图片类型
    imageViewerSpace::ImageType getImageType(const QString &imagepath);
    //判断路径类型
    imageViewerSpace::PathType getPathType(const QString &imagepath);

protected:
    void run() override;

private:
    std::atomic_bool m_quit;
};

#endif // IMAGEDATASERVICE_H
