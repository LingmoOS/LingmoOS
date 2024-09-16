/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     ZhangYong <zhangyong@uniontech.com>
 *
 * Maintainer: ZhangYong <ZhangYong@uniontech.com>
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
#ifndef IMAGEDATASERVICE_H
#define IMAGEDATASERVICE_H

#include <QObject>
#include <QMap>
#include <QUrl>
#include <QMutex>
#include <QThread>
#include <QQueue>
#include <deque>

class readThumbnailThread;
class ReadThumbnailManager;
class ImageDataService: public QObject
{
    Q_OBJECT
public:
    static ImageDataService *instance(QObject *parent = nullptr);
    explicit ImageDataService(QObject *parent = nullptr);

    void addImage(const QString &path, const QImage &image);
    QImage getThumnailImageByPathRealTime(const QString &path, bool isTrashFile, bool bReload = false);
    bool imageIsLoaded(const QString &path, bool isTrashFile);

    void addMovieDurationStr(const QString &path, const QString &durationStr);
    QString getMovieDurationStrByPath(const QString &path);

    void stopFlushThumbnail();
    void waitFlushThumbnailFinish();

    bool readerIsRunning();

    //切换图片显示状态
    Q_INVOKABLE void switchLoadMode();

    //获得当前图片显示的状态
    Q_INVOKABLE int getLoadMode();

    // 根据加载模式获取对应加载图片路径
    QString getLoadModePath(const QString &path);

    // 获取等比例缩略图存放路径
    QString getScaledPath(const QString &path);

private slots:
signals:
    void sigeUpdateListview();
    void gotImage(const QString path);
    void startImageLoad();
public:
private:
    bool pathInMap(const QString &path);

    //QImage:图片，bool:是否是从缓存加载
    std::pair<QImage, bool> getImageFromMap(const QString &path);
    // 从缓存清除图片信息
    void removePathFromMap(const QString &path);

    // 清除图片文件对应缩略图文件
    void removeThumbnailFile(const QString &path);

private:
    static ImageDataService *s_ImageDataService;

    //图片数据锁
    QMutex m_imgDataMutex;
    //QString:原图路径 QImage:缩略图
    std::deque<std::pair<QString, QImage>> m_AllImageMap;
    QMap<QString, QString> m_movieDurationStrMap;

    //加载模式控制
    std::atomic_int m_loadMode;

    ReadThumbnailManager *readThumbnailManager;
    QThread *readThread;
};

//缩略图读取类
class ReadThumbnailManager : public QObject
{
    Q_OBJECT
public:
    explicit ReadThumbnailManager(QObject *parent = nullptr);
    void addLoadPath(const QString &path);

    bool isRunning()
    {
        return runningFlag;
    }

    void stopRead()
    {
        stopFlag = true;
    }

public slots:
    void readThumbnail();

private:
    // 将图片裁剪为方图
    QImage clipToRect(const QImage &src);
    // 将图片按比例缩小
    QImage addPadAndScaled(const QImage &src);
private:
    std::deque<QString> needLoadPath;
    QMutex mutex;
    std::atomic_bool runningFlag;
    std::atomic_bool stopFlag;
};

#endif // IMAGEDATASERVICE_H
