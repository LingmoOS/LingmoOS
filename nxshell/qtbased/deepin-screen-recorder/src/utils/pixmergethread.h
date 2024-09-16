// Copyright (C) 2020 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PIXMERGETHREAD_H
#define PIXMERGETHREAD_H

#include <QObject>
#include <QMutex>
#include <QThread>
#include <QPixmap>
#include <QImage>
#include <QQueue>
#include <QDebug>

#include<opencv2/opencv.hpp>


class PixMergeThread : public QThread
{
    Q_OBJECT

    Q_ENUMS(MergeErrorValue)
public:

    enum MergeErrorValue {
        Failed = 1,     // 拼接失败， 拼接出现不重叠区域
        ReachBottom,    // 图片长度不再曾涨(滚动到底)，甚至减少（反向滚动出现）
        MaxHeight,      // 最大高度
        InvalidArea,    //无效区域，点击调整捕捉区域
        RoollingTooFast, //滚动速度过快
    };

    enum PictureDirection {//滚动方向
        ScrollDown = 0,     //向下滚动
        ScrollUp,           //向上滚动
    };

    explicit PixMergeThread(QObject *parent = nullptr);
    ~PixMergeThread();
    void stopTask();
    void addShotImg(const QPixmap &picture, PictureDirection direction = ScrollDown);
    QImage getMerageResult() const;
    void run();

    //手动滚动时的函数处理
    void setScrollModel(bool isManualScrollMode); //设置是否为手动模式

    void clearCurImg();
    void calculateTimeDiff(int time); //计算时间差
    bool isOneWay(); //是否单向
    void setIsLastImg(bool isLastImg); //设置最后一张图片标记
protected:
    cv::Mat qPixmapToCvMat(const QPixmap &inPixmap);
    bool mergeImageWork(const cv::Mat &image, int imageStatus = ScrollDown);
    int getTopFixedHigh(cv::Mat &img1, cv::Mat &img2);

    //手动滚动时的函数处理
    int getBottomFixedHigh(cv::Mat &img1, cv::Mat &img2); //裁剪底部固定区域
    bool splicePictureUp(const cv::Mat &image);//向上拼接图片
    bool splicePictureDown(const cv::Mat &image);//向下拼接图片
    QRect getScrollChangeRectArea(cv::Mat &img1, const cv::Mat &img2);//计算可以滚动的区域
    void ScrollUpAddImg(const QPixmap &picture); //滚动向上时添加
    void ScrollDwonAddImg(const QPixmap &picture);//滚动向下时添加
signals:
    void merageError(MergeErrorValue state);
    void updatePreviewImg(QImage img);
    void invalidAreaError(MergeErrorValue state, QRect rect); //调整区域异常
private:
    QMutex m_Mutex;
    bool m_loopTask = true;
    cv::Mat m_curImg;
    QQueue<QPair< QPixmap, PictureDirection >> m_pixImgs; //图片队列
    unsigned int m_ImageCount = 0;
    unsigned int m_MeragerCount = 0;
    // 长图顶部固定区域高度
    int m_headHeight = -1;
    static const int LONG_IMG_MAX_HEIGHT;
    static const int TEMPLATE_HEIGHT;

    //手动截图状态
    //bool m_successfullySplicedUp = false; //向上拼接成功
    //bool m_successfullySplicedDwon = false;//向下拼接成功
    bool m_isManualScrollModel = false;//是否手动模式
    int m_bottomHeight = -1;// 长图底部固定区域高度
    int m_curTimeDiff = 0; //当前时间差
    int m_lastTime = 0;    //上一次的时间
    int m_upCount = 0;
    int m_downCount = 0;
    bool m_isLastPixmap = false; // 是否是最后一张
};

Q_DECLARE_METATYPE(PixMergeThread::MergeErrorValue);
#endif //
