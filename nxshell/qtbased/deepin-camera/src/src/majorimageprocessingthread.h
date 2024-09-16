// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MajorImageProcessingThread_H
#define MajorImageProcessingThread_H

#include <QThread>
#include <QPixmap>
#include <QDebug>
#include <QMutex>
#include <QWaitCondition>

#include "datamanager.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "core_io.h"
#include "LPF_V4L2.h"
#include "camview.h"
#include "stdlib.h"
#include "malloc.h"
#include "stdio.h"
#include "math.h"
#include "save_image.h"
#include "colorspaces.h"
#include "render.h"
#ifdef __cplusplus
}
#endif

/**
 * @brief stop 线程处理图片
 */
class MajorImageProcessingThread : public QThread
{
    Q_OBJECT
public:
    MajorImageProcessingThread();

    ~MajorImageProcessingThread();

    /**
     * @brief setHorizontalMirror 设置水平镜像
     * @param bMirror 是否镜像
     */
    void setHorizontalMirror(bool bMirror) {m_bHorizontalMirror = bMirror;}

    /**
     * @brief stop 停止线程
     */
    void stop();

    /**
     * @brief init 线程初始化
     */
    void init();

    /**
     * @brief getStatus 获取状态
     */
    QAtomicInt getStatus()
    {
        return m_stopped;
    }

    /**
     * @brief setFilter 设置滤镜
     * @param filter 滤镜名称
     */
    void setFilter(QString filter);

    /**
     * @brief setExposure 设置曝光
     * @param exposure 曝光值
     */
    void setExposure(int exposure);

    /**
     * @brief setState 设置拍照，录像状态
     * @param bPhoto  true 拍照状态， false 录像状态
     */
    void setState(bool bPhoto){
        m_bPhoto = bPhoto;
    }

    /**
     * @brief setRecording 设置视频是否正在录制
     * @param bStatus  true 录制中 false 未录制
     */
    void setRecording(bool bRecording) {
        m_bRecording = bRecording;
    }

    /**
     * @brief setFilterGroupState 设置滤镜按钮组展开状态
     * @param bDisplay  true 展开 false 关闭
     */
    void setFilterGroupState(bool bDisplay){
        m_filtersGroupDislay = bDisplay;
    }

    int getRecCount();

protected:
    /**
     * @brief run 运行线程
     */
    void run();

signals:
    /**
     * @brief SendMajorImageProcessing 向预览界面发送帧数据  mips平台、wayland下使用该接口
     * @param image 图像
     * @param result 结果
     */
    void SendMajorImageProcessing(QImage *image, int result);

    /**
     * @brief SendFilterImageProcessing 向滤镜预览类发送帧数据
     * @param image 预览图像 大小40*40
     */
    void SendFilterImageProcessing(QImage *image);

    /**
     * @brief sigReflushSnapshotLabel 发送刷新照片预览图信号
     */
    void sigReflushSnapshotLabel();

#ifndef __mips__
    /**
     * @brief sigYUVFrame YUV框架信号
     * @param yuv YUV
     * @param width 宽度
     * @param height 高度
     */
    void sigYUVFrame(uchar *yuv, uint width, uint height);

    /**
     * @brief sigRenderYuv 发送Yuv信号
     * @param width
     */
    void sigRenderYuv(bool);

#endif

    /**
     * @brief sigRecordFrame 发送GStreamer视频帧写入信号
     * @param rgb  rgb24帧数据
     * @param size yuv数据大小
     */
    void sigRecordFrame(uchar *rgb, uint size);

    /**
     * @brief reachMaxDelayedFrames 到达最大延迟信号
     */
    void reachMaxDelayedFrames();

public:
    QMutex  m_rwMtxImg;
    QString m_strPath;
    QMutex  m_rwMtxPath;
    bool    m_bTake; //是否拍照

private:
    void ImageHorizontalMirror(const uint8_t* src, uint8_t* dst, int width, int height);

public slots:
    void processingImage(QImage&);

private:
    int               m_result;
    uint              m_nVdWidth;
    uint              m_nVdHeight;
    volatile int      m_majorindex;
    QString           m_filter;//当前选择的滤镜名称
    QAtomicInt        m_stopped;
    v4l2_dev_t        *m_videoDevice;
    v4l2_frame_buff_t *m_frame;
    uint8_t           *m_yuvPtr;// yu12视频帧数据
    uint8_t           *m_rgbPtr;// rgb视频帧数据

    bool              m_bPhoto = true; //相机当前状态，默认为拍照状态
    bool              m_bRecording;//是否处理视频录制状态 GStreamer环境下使用
    bool              m_bHorizontalMirror;   //水平镜像
    EncodeEnv         m_eEncodeEnv;          //编码环境
    int               m_exposure = 0;
    bool              m_filtersGroupDislay = false;//滤镜按钮组是否显示
    int               m_nCount;
    uint64_t          m_firstPts;

    QImage            m_Img;   //mips、wayland下使用该变量
    QImage            m_filterImg; //滤镜预览类使用 大小40*40
    QImage            m_jpgImage; // 从v4l2获取的jpg格式的视频帧图片

};

#endif // MajorImageProcessingThread_H
