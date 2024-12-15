// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BROWSERMAGNIFIER_H
#define BROWSERMAGNIFIER_H

#include <QLabel>
#include <QThread>
#include <QMutex>

class BrowserPage;
class SheetBrowser;

typedef struct MagnifierInfo_t {
    QObject *target = nullptr;
    QString slotFun = "";
    QPoint mousePos;
    double scaleFactor;
    BrowserPage *page;
} MagnifierInfo_t;
Q_DECLARE_METATYPE(MagnifierInfo_t)

/**
 * @brief 放大镜任务线程
 */
class ReadMagnifierManager: public QThread
{
public:
    explicit ReadMagnifierManager(QWidget *parent);
    ~ReadMagnifierManager() override;

public:
    /**
     * @brief addTask
     * 添加放大镜任务
     * @param task
     */
    void addTask(const MagnifierInfo_t &task);

protected:
    /**
     * @brief run
     * 具体任务执行接口
     */
    void run() override;

private:
    QMutex m_mutex;
    QWidget *m_parent;
    QList<MagnifierInfo_t> m_tTasklst;
};

/**
 * @brief 放大镜控件
 */
class BrowserMagniFier : public QLabel
{
    Q_OBJECT

public:
    explicit BrowserMagniFier(SheetBrowser *parent);

    ~BrowserMagniFier();

    /**
     * @brief 显示放大区域的图片
     * @param viewPoint
     * @param magnifierPos
     * @param scaleFactor
     */
    void showMagnigierImage(QPoint viewPoint, QPoint magnifierPos, double scaleFactor);

    /**
     * @brief 更新显示的图片
     */
    void updateImage();

private:
    /**
     * @brief 设置要显示的图片
     * @param image
     */
    void setMagniFierImage(const QImage &image);

private slots:
    /**
     * @brief 更新放大镜区域内的图片
     * @param task
     * @param image
     */
    void onUpdateMagnifierImage(const MagnifierInfo_t &task, const QImage &image);

private:
    qreal m_lastScaleFactor = 0.0;
    QPoint m_lastPoint;
    QPoint m_lastScenePoint;
    SheetBrowser *m_brwoser = nullptr;
    ReadMagnifierManager *m_readManager = nullptr;
};

#endif // BROWSERMAGNIFIER_H
