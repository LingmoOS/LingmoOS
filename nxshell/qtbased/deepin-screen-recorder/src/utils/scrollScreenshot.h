// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCROLLSCREENSHOT_H
#define SCROLLSCREENSHOT_H

#include "pixmergethread.h"
#ifdef KF5_WAYLAND_FLAGE_ON
#include "waylandmousesimulator.h"
#endif
#include <QPixmap>
#include <QTimer>
#include <QRect>
#include<opencv2/opencv.hpp>
#include "keydefine.h"

class ScrollScreenshot: public QObject
{
    Q_OBJECT

public:
    enum ScrollStatus {
        Wait = 0,  // 初始状态
        Merging,   // 拼接过程中
        Stop,      // 停止拼接
        Mistake    // 拼接失败
    };

    explicit ScrollScreenshot(QObject *parent = nullptr);
    ~ScrollScreenshot();
    void addPixmap(const QPixmap &piximg, int wheelDirection = WheelDown); //添加图片到拼接线程
    /**
     * @brief 保存时，添加最后一张图片到拼接线程
     */
    void addLastPixmap(const QPixmap &piximg);
    /**
     * @brief 清除内存中用来存储图片的矩阵数据
     */
    void clearPixmap();
    void changeState(const bool isStop);
    QImage savePixmap();

    //手动滚动时的函数处理
    void setScrollModel(bool model); //设置滚动模式，先设置滚动模式，再添加图片
    QRect getInvalidArea();//获取调整区域
    void setTimeAndCalculateTimeDiff(int time); //设置时间并计算时间差
signals:
    void getOneImg();
    void updatePreviewImg(QImage img);
    void merageError(PixMergeThread::MergeErrorValue);

    /**
     * @brief 当模拟鼠标进行自动滚动时，会发射此信号
     */
    void autoScroll(int autoScrollFlag);
    void sigalWheelScrolling(float direction);
public slots:
    void merageImgState(PixMergeThread::MergeErrorValue state);
    void merageInvalidArea(PixMergeThread::MergeErrorValue state, QRect rect); //调整捕捉区域
private:
    /**
     * @brief 用来监听模拟自动滚动截图的标志,只有当进行自动滚动截图时此属性的值会一直增加
     */
    int m_autoScrollFlag = 1;

    PixMergeThread *m_PixMerageThread = nullptr;

    QTimer *m_mouseWheelTimer = nullptr;
    // 截图实际高度
    unsigned int m_shotImgHeight = 0;
    // 截图宽度
    unsigned int m_shotImgWidth = 0;

    // 滚动事件发送m_shotFrequency次，截图一次。控制截图频率
    unsigned int m_shotFrequency = 2;

    // 滚动计数
    unsigned int m_scrollCount = 0;

    ScrollStatus m_curStatus = ScrollStatus::Wait;

    //处理手动滚动时新增
    bool m_isManualScrollModel = false;//是否手动模式
    QRect m_rect;//调整区域
    bool m_startPixMerageThread = false;
//#ifdef KF5_WAYLAND_FLAGE_ON
//    WaylandMouseSimulator *m_WaylandScrollMonitor = nullptr;
//#endif
    /**
     * @brief 用来记录当前滚动方向
     */
    PixMergeThread::PictureDirection m_lastDirection = PixMergeThread::PictureDirection::ScrollDown;
};

#endif // SCROLLSCREENSHOT_H
