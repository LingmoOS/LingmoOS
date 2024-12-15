// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef READERIMAGETHREADPOOLMANAGER_H
#define READERIMAGETHREADPOOLMANAGER_H

#include <QThreadPool>
#include <QPixmap>
#include <QRunnable>
#include <QMap>
#include <QMutex>

class DocSheet;
typedef struct ReaderImageParam_t {
    bool bForceUpdate = false;

    int pageIndex = 0;
    int maxPixel = 174;
    QSize boundedRect = QSize(0, 0);//幻灯片放映时的boundedRect
    DocSheet *sheet = nullptr;

    QObject *receiver = nullptr;
    QString slotFun = "";

    bool operator == (const ReaderImageParam_t &other) const
    {
        return this->pageIndex == other.pageIndex
                && this->maxPixel == other.maxPixel
                && this->sheet == other.sheet
                && this->boundedRect == other.boundedRect
                ;
    }

    bool operator < (const ReaderImageParam_t &other) const
    {
        return (this->pageIndex < other.pageIndex);
    }

    bool operator > (const ReaderImageParam_t &other) const
    {
        return (this->pageIndex > other.pageIndex);
    }

private:
    friend class ReaderImageThreadPoolManager;
    QRunnable *task = nullptr;
} ReaderImageParam_t;
Q_DECLARE_METATYPE(ReaderImageParam_t);

/**
 * @brief The ReadImageTask class
 * 获取文档图片线程类
 */
class ReadImageTask: public QRunnable
{
public:
    /**
     * @brief addgetDocImageTask
     * 添加获取图片任务
     * @param readImageParam
     */
    void addgetDocImageTask(const ReaderImageParam_t &readImageParam);

    /**
     * @brief setThreadPoolManager
     * 设置线程池管理对象
     * @param object
     */
    void setThreadPoolManager(QObject *object);

    /**
     * @brief run
     * run
     */
    void run() override;

private:
    QObject *m_threadpoolManager = nullptr;
    ReaderImageParam_t m_docParam;
};

/**
 * @brief The ReaderImageThreadPoolManager class
 * 图片获取线程池管理类
 */
class ReaderImageThreadPoolManager: public QThreadPool
{
    Q_OBJECT

public:
    explicit ReaderImageThreadPoolManager();
    static ReaderImageThreadPoolManager *getInstance();

public:
    /**
     * @brief addgetDocImageTask
     * 添加图片获取任务
     * @param readImageParam
     */
    void addgetDocImageTask(const ReaderImageParam_t &readImageParam);

    /**
     * @brief getImageForDocSheet
     * 获取指定文档,指定页数的图片
     * @param sheet 文档
     * @param pageIndex 页数
     * @return 图片
     */
    QPixmap getImageForDocSheet(DocSheet *sheet, int pageIndex);

    /**
     * @brief 设置Sheet对应缩略图缓存Index图片
     * @param sheet
     * @param pageIndex
     * @param pixmap
     */
    void setImageForDocSheet(DocSheet *sheet, int pageIndex, const QPixmap &pixmap);

private slots:
    /**
     * @brief onTaskFinished
     * 获取图片任务结束
     * @param task
     * @param image
     */
    void onTaskFinished(const ReaderImageParam_t &task, const QImage &image);

    /**
     * @brief onDocProxyDestroyed
     * 文档已被销毁
     * @param obj
     */
    void onDocProxyDestroyed(QObject *obj);

    /**
     * @brief onReceiverDestroyed
     * 接收函数对象已被销毁
     * @param obj
     */
    void onReceiverDestroyed(QObject *obj);

private:
    QMutex m_runMutex;
    QList<QObject *> m_docProxylst;
    QList<ReaderImageParam_t> m_taskList;
    QMap<QObject *, QVector<QPixmap>> m_docSheetImgMap;
};

#endif // READERIMAGETHREADPOOLMANAGER_H;
