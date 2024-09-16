// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGERENDERTHREAD_H
#define PAGERENDERTHREAD_H

#include "Global.h"
#include "Model.h"

#include <QThread>
#include <QMutex>
#include <QStack>
#include <QImage>

class DocSheet;
class BrowserPage;
class SheetRenderer;
class SideBarImageViewModel;

struct DocPageNormalImageTask {//正常取图
    DocSheet *sheet = nullptr;
    BrowserPage *page = nullptr;
    int pixmapId = 0;           //任务艾迪
    QRect rect = QRect();       //整个大小
};

struct DocPageSliceImageTask {//取切片
    DocSheet *sheet = nullptr;
    BrowserPage *page = nullptr;
    int pixmapId = 0;           //任务艾迪
    QRect whole = QRect();      //整个大小
    QRect slice = QRect();      //切片大小
};

struct DocPageBigImageTask {//取大图
    DocSheet *sheet = nullptr;
    BrowserPage *page = nullptr;
    int pixmapId = 0;           //任务艾迪
    QRect rect = QRect();       //整个大小
};

struct DocPageWordTask {//取页码文字
    DocSheet *sheet = nullptr;
    BrowserPage *page = nullptr;
};

struct DocPageAnnotationTask {//取页码注释
    DocSheet *sheet = nullptr;
    BrowserPage *page = nullptr;
};

struct DocPageThumbnailTask {//缩略图
    DocSheet *sheet = nullptr;
    SideBarImageViewModel *model = nullptr;
    int index = -1;
};

struct DocOpenTask {//打开文档
    DocSheet *sheet = nullptr;
    QString password;
    SheetRenderer *renderer = nullptr;
};

struct DocCloseTask {//关闭文档
    deepin_reader::Document *document = nullptr;
    QList<deepin_reader::Page *> pages;
};

/**
 * @brief The PageRenderThread class
 * 执行加载图片和文字等耗时操作的线程,由于pdfium非常线程不安全，所有操作都在本线程中进行
 */
class PageRenderThread : public QThread
{
    Q_OBJECT
public:
    /**
     * @brief clearImageTasks
     * 清除需要读取图片的任务
     * @param sheet
     * @param item 项指针
     * @param pixmapId 删除不同的pixmapId,-1为删除所有
     * @return 是否成功
     */
    static bool clearImageTasks(DocSheet *sheet, BrowserPage *page, int pixmapId = -1);

    /**
     * @brief appendTask
     * 添加任务到队列
     * @param task
     */
    static void appendTask(DocPageNormalImageTask task);

    static void appendTask(DocPageSliceImageTask task);

    static void appendTask(DocPageBigImageTask task);

    static void appendTask(DocPageWordTask task);

    static void appendTask(DocPageAnnotationTask task);

    static void appendTask(DocPageThumbnailTask task);

    static void appendTask(DocOpenTask task);

    static void appendTask(DocCloseTask task);

    /**
     * @brief destroyForever
     * 销毁线程且不会再被创建
     */
    static void destroyForever();

private:
    explicit PageRenderThread(QObject *parent = nullptr);

    ~PageRenderThread();

    void run();

private:
    /**
     * @brief hasNextTask 任务池中是否还存在任务
     * @return true:存在 false:不存在
     */
    bool hasNextTask();

    /**
     * @brief popNextDocPageNormalImageTask 任务池中是否还存在正常取图任务
     * @param task
     * @return
     */
    bool popNextDocPageNormalImageTask(DocPageNormalImageTask &task);

    /**
     * @brief popNextDocPageSliceImageTask 任务池中是否还存在取切片任务
     * @param task
     * @return
     */
    bool popNextDocPageSliceImageTask(DocPageSliceImageTask &task);

    /**
     * @brief popNextDocPageBigImageTask 任务池中是否还存在取大图任务
     * @param task
     * @return
     */
    bool popNextDocPageBigImageTask(DocPageBigImageTask &task);

    /**
     * @brief popNextDocPageWordTask 任务池中是否还存在取页码文字任务
     * @param task
     * @return
     */
    bool popNextDocPageWordTask(DocPageWordTask &task);

    /**
     * @brief popNextDocPageAnnotationTask 任务池中是否还存在取页码注释任务
     * @param task
     * @return
     */
    bool popNextDocPageAnnotationTask(DocPageAnnotationTask &task);

    /**
     * @brief popNextDocPageThumbnailTask 任务池中是否还存在缩略图任务
     * @param task
     * @return
     */
    bool popNextDocPageThumbnailTask(DocPageThumbnailTask &task);

    /**
     * @brief popNextDocOpenTask 任务池中是否还存在打开文档任务
     * @param task
     * @return
     */
    bool popNextDocOpenTask(DocOpenTask &task);

    /**
     * @brief popNextDocCloseTask 任务池中是否还存在文档关闭任务
     * @param task
     * @return true: 是 false:否
     */
    bool popNextDocCloseTask(DocCloseTask &task);

private:

    /**
     * @brief execNextDocPageNormalImageTask 执行正常取图任务
     * @return
     */
    bool execNextDocPageNormalImageTask();

    /**
     * @brief execNextDocPageSliceImageTask 执行取切片任务
     * @return
     */
    bool execNextDocPageSliceImageTask();

    /**
     * @brief execNextDocPageWordTask 执行取页码文字
     * @return
     */
    bool execNextDocPageWordTask();

    /**
     * @brief execNextDocPageAnnotationTask 执行取页码注释
     * @return
     */
    bool execNextDocPageAnnotationTask();

    /**
     * @brief execNextDocPageThumbnailTask 执行缩略图
     * @return
     */
    bool execNextDocPageThumbnailTask();

    /**
     * @brief execNextDocOpenTask 执行文档打开任务
     * @return
     */
    bool execNextDocOpenTask();

    /**
     * @brief execNextDocCloseTask 执行文档关闭任务
     * @return
     */
    bool execNextDocCloseTask();

signals:
    void sigDocPageNormalImageTaskFinished(DocPageNormalImageTask, QPixmap);

    void sigDocPageSliceImageTaskFinished(DocPageSliceImageTask, QPixmap);

    void sigDocPageBigImageTaskFinished(DocPageBigImageTask, QPixmap);

    void sigDocPageWordTaskFinished(DocPageWordTask, QList<deepin_reader::Word>);

    void sigDocPageAnnotationTaskFinished(DocPageAnnotationTask, QList<deepin_reader::Annotation *>);

    void sigDocPageThumbnailTaskFinished(DocPageThumbnailTask, QPixmap);

    void sigDocOpenTask(DocOpenTask, deepin_reader::Document::Error, deepin_reader::Document *, QList<deepin_reader::Page *>);

private slots:
    void onDocPageNormalImageTaskFinished(DocPageNormalImageTask task, QPixmap pixmap);

    void onDocPageSliceImageTaskFinished(DocPageSliceImageTask task, QPixmap pixmap);

    void onDocPageBigImageTaskFinished(DocPageBigImageTask task, QPixmap pixmap);

    void onDocPageWordTaskFinished(DocPageWordTask task, QList<deepin_reader::Word> words);

    void onDocPageAnnotationTaskFinished(DocPageAnnotationTask task, QList<deepin_reader::Annotation *> annots);

    void onDocPageThumbnailTask(DocPageThumbnailTask task, QPixmap pixmap);

    void onDocOpenTask(DocOpenTask task, deepin_reader::Document::Error error, deepin_reader::Document *document, QList<deepin_reader::Page *> pages);

private:
    QMutex m_pageNormalImageMutex;
    QList<DocPageNormalImageTask> m_pageNormalImageTasks;

    QMutex m_pageSliceImageMutex;
    QList<DocPageSliceImageTask> m_pageSliceImageTasks;

    QMutex m_pageBigImageMutex;
    QList<DocPageBigImageTask> m_pageBigImageTasks;

    QMutex m_pageWordMutex;
    QList<DocPageWordTask> m_pageWordTasks;

    QMutex m_pageAnnotationMutex;
    QList<DocPageAnnotationTask> m_pageAnnotationTasks;

    QMutex m_pageThumbnailMutex;
    QList<DocPageThumbnailTask> m_pageThumbnailTasks;

    QMutex m_openMutex;
    QList<DocOpenTask> m_openTasks;

    QMutex m_closeMutex;
    QList<DocCloseTask> m_closeTasks;

    bool m_quit = false;

    static bool s_quitForever;

    static PageRenderThread *s_instance;

    static PageRenderThread *instance();
};

#endif // PAGERENDERTHREAD_H
