// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "scrollScreenshot.h"
#include "utils.h"

#include <QDebug>
#include <QDateTime>
#include <QEventLoop>
#include <X11/Xlibint.h>
#include <X11/extensions/XTest.h>


ScrollScreenshot::ScrollScreenshot(QObject *parent)  : QObject(parent)
{
    Q_UNUSED(parent);
    qRegisterMetaType<PixMergeThread::MergeErrorValue>("MergeErrorValue");
//    if (Utils::isWaylandMode) {
//#ifdef KF5_WAYLAND_FLAGE_ON
//    m_WaylandScrollMonitor = new WaylandMouseSimulator(this); // 初始化wayland模拟滚动
//#endif
//}
    m_mouseWheelTimer = new QTimer(this);
    connect(m_mouseWheelTimer, &QTimer::timeout, this, [ = ] {
        if (!Utils::isWaylandMode)
        {
            // 发送滚轮事件， 自动滚动
            static Display *m_display = XOpenDisplay(nullptr);
            XTestFakeButtonEvent(m_display, Button5, 1, CurrentTime);
            XFlush(m_display);
            XTestFakeButtonEvent(m_display, Button5, 0, CurrentTime);
            XFlush(m_display);
        } else
        {
#ifdef KF5_WAYLAND_FLAGE_ON
            WaylandMouseSimulator::instance()->doWaylandAutoScroll(); //waland滚动
#endif
        }

        //当模拟鼠标进行自动滚动时，会发射此信号
        emit autoScroll(m_autoScrollFlag++);
        // 滚动区域高度 200-300 取值2
        // 滚动区域高度 > 300  取值 3
        // 滚动区域高度 > 600  取值 5
        m_scrollCount++;
        if (m_scrollCount % m_shotFrequency == 0)
        {
            emit getOneImg();
        }
    });

    m_PixMerageThread = new PixMergeThread(this);
    connect(m_PixMerageThread, SIGNAL(updatePreviewImg(QImage)), this, SIGNAL(updatePreviewImg(QImage)));
    connect(m_PixMerageThread, SIGNAL(merageError(PixMergeThread::MergeErrorValue)), this, SLOT(merageImgState(PixMergeThread::MergeErrorValue)));
    connect(m_PixMerageThread, &PixMergeThread::invalidAreaError, this, &ScrollScreenshot::merageInvalidArea);
#ifdef KF5_WAYLAND_FLAGE_ON
    connect(this, &ScrollScreenshot::sigalWheelScrolling, WaylandMouseSimulator::instance(), &WaylandMouseSimulator::slotManualScroll);
#endif
}

ScrollScreenshot::~ScrollScreenshot()
{
    if (nullptr != m_PixMerageThread) {
        m_PixMerageThread->stopTask(); //避免使用terminate
        m_PixMerageThread->wait();
        delete m_PixMerageThread;
        m_PixMerageThread = nullptr;
    }
}

void ScrollScreenshot::addPixmap(const QPixmap &piximg, int wheelDirection)
{
    if (m_startPixMerageThread == false) {
        m_PixMerageThread->start();
        m_startPixMerageThread = true;
    }
    m_PixMerageThread->setScrollModel(m_isManualScrollModel);
    if (m_isManualScrollModel == false) {//自动
        if (m_curStatus == Wait) {
            m_mouseWheelTimer->start(300);
            m_curStatus = Merging;
        }
        if (m_curStatus == Merging) {
            m_lastDirection = PixMergeThread::PictureDirection::ScrollDown; // 记录滚动方向
            m_PixMerageThread->addShotImg(piximg, PixMergeThread::PictureDirection::ScrollDown);
        }
    } else if (m_isManualScrollModel == true) {//手动
        //qDebug() << "function piximg is null: " << __func__ << " ,line: " << __LINE__;
        m_mouseWheelTimer->stop();
        PixMergeThread::PictureDirection  status = (wheelDirection == WheelDown) ? (PixMergeThread::PictureDirection::ScrollDown) : (PixMergeThread::PictureDirection::ScrollUp);
        m_lastDirection = status; // 记录滚动方向
        m_PixMerageThread->addShotImg(piximg, status);
    }
}

void ScrollScreenshot::addLastPixmap(const QPixmap &piximg)
{
    setTimeAndCalculateTimeDiff(static_cast<int>(QDateTime::currentDateTime().toTime_t()));
    m_PixMerageThread->setIsLastImg(true); //添加最后一张图片标记
    if (nullptr != m_PixMerageThread)
        m_PixMerageThread->addShotImg(piximg, m_lastDirection);
}

void ScrollScreenshot::clearPixmap()
{
    m_PixMerageThread->clearCurImg();
}

void ScrollScreenshot::changeState(const bool isStop)
{
    //qDebug() << __FUNCTION__ << "====" << isStop;
    // 暂停
    if (isStop && m_curStatus == Merging) {
        m_curStatus = Stop;
        m_mouseWheelTimer->stop();
    }

    // 开始
    if (!isStop && m_curStatus == Stop) {
        m_curStatus = Merging;
        m_mouseWheelTimer->start(300);
    }
}
QImage ScrollScreenshot::savePixmap()
{
    m_mouseWheelTimer->stop();

    QEventLoop eventloop1; // 延迟500毫秒
    QTimer::singleShot(500, &eventloop1, SLOT(quit()));
    eventloop1.exec();

    m_PixMerageThread->stopTask();
    m_PixMerageThread->wait();
    return m_PixMerageThread->getMerageResult();
}


//设置滚动模式，先设置滚动模式，再添加图片
void ScrollScreenshot::setScrollModel(bool model)
{
    m_isManualScrollModel = model;
}
//获取调整区域
QRect ScrollScreenshot::getInvalidArea()
{
    return m_rect;
}
//设置时间并计算时间差
void ScrollScreenshot::setTimeAndCalculateTimeDiff(int time)
{
    m_PixMerageThread->calculateTimeDiff(time);
}


void ScrollScreenshot::merageImgState(PixMergeThread::MergeErrorValue state)
{
    qDebug() << "拼接状态值:" << state;
    m_mouseWheelTimer->stop();
    if (state == PixMergeThread::MaxHeight) {
        m_curStatus = ScrollStatus::Mistake;
    }
    emit merageError(state);
}

//调整捕捉区域
void ScrollScreenshot::merageInvalidArea(PixMergeThread::MergeErrorValue state, QRect rect)
{
    m_rect = rect;
    m_mouseWheelTimer->stop();
    if (state == PixMergeThread::MaxHeight) {
        m_curStatus = ScrollStatus::Mistake;
    }
    emit merageError(state);
}
