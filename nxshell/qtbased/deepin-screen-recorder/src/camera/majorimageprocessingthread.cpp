// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "majorimageprocessingthread.h"
#include "../utils.h"

extern "C" {
#include <libimagevisualresult/visualresult.h>
}

#include <QFile>
#include <QDate>
#include <QDir>
#include <QEventLoop>
#include <QTimer>
MajorImageProcessingThread::MajorImageProcessingThread()
{
    init();
}

void MajorImageProcessingThread::stop()
{
    //只有线程还是运行状态时才会退出
    if (m_stopped) {
        qDebug() << "正在停止传递摄像头画面...";
        m_stopped = 0;
        QEventLoop loop;
        connect(this, SIGNAL(isEnd()), &loop, SLOT(quit()));
        loop.exec();
        qDebug() << "已停止采集摄像头画面";
    }
}

void MajorImageProcessingThread::init()
{
    m_stopped = 0;
    m_frame = nullptr;
    m_videoDevice = nullptr;
}

void MajorImageProcessingThread::setCameraDevice(v4l2_dev_t *camDevice)
{
    m_videoDevice = camDevice;
}

void MajorImageProcessingThread::run()
{
    qDebug() << "正在启动采集摄像头画面线程...";
    m_stopped = 1;
    v4l2_dev_t *vd =  get_v4l2_device_handler();
    if (vd == nullptr) {
        qWarning() << "启动采集摄像头画面线程失败！摄像头设备不存在！";
        return;
    }
    qDebug() << "已启动采集摄像头画面线程，开始采集画面";
    v4l2core_start_stream(vd);
    while (m_stopped) {
//        qDebug() << __FUNCTION__ << __LINE__ << "正在采集摄像头(" << vd->videodevice << ")画面 >>> 1";
        if (vd) {
            m_frame = v4l2core_get_decoded_frame(vd);
        } else {
            continue;
        }
//        qDebug() << __FUNCTION__ << __LINE__ << "正在采集摄像头(" << vd->videodevice << ")画面 >>> 2";
        if (m_frame == nullptr) {
            continue;
        }
        //某些摄像头无法通过此方法获取图像数据
//            QImage jpgImage;
//            QByteArray temp;
//            temp.append(reinterpret_cast<const char *>(m_frame->raw_frame), static_cast<int>(m_frame->raw_frame_max_size));
//            jpgImage.loadFromData(temp);
//            jpgImage = jpgImage.convertToFormat(QImage::Format_RGB888);
//            if (jpgImage.isNull()) {
//                qWarning() << "未采集到摄像头画面！" ;
//            } else {
//                //jpgImage.save(QString("/home/wangcong/Desktop/test/test_%1.png").arg(QDateTime::currentMSecsSinceEpoch()));
//                emit SendMajorImageProcessing(jpgImage);
//                //qInfo() << "已采集摄像头画面，正在传递摄像头画面";
//            }
        uint rgbsize = 0;
        uint yuvsize = 0;
        unsigned int nVdWidth = static_cast<unsigned int>(m_frame->width);
        unsigned int nVdHeight = static_cast<unsigned int>(m_frame->height);
        uint8_t   *rgbPtr = nullptr;
        uint8_t   *yuvPtr = nullptr;
        QImage tempImg;

        // wayland环境下，解码后的帧数据为yu12格式
        yuvsize = nVdWidth * nVdHeight * 3 / 2;
        yuvPtr = new uchar[yuvsize];
        memcpy(yuvPtr, m_frame->yuv_frame, yuvsize);

        rgbsize = nVdWidth * nVdHeight * 3;
        rgbPtr = static_cast<uint8_t *>(calloc(rgbsize, sizeof(uint8_t)));
        // yu12到rgb数据高性能转换
        yu12_to_rgb24_higheffic(rgbPtr, yuvPtr, m_frame->width, m_frame->height);

        tempImg = QImage(rgbPtr, m_frame->width, m_frame->height, QImage::Format_RGB888);
        if (tempImg.isNull()) {
            qWarning() << __FUNCTION__ << __LINE__ <<  " 未采集到摄像头画面！" ;
        } else {
            //tempImg.save(QString("/home/uos/Desktop/test/test_%1.png").arg(QDateTime::currentMSecsSinceEpoch()));
            emit SendMajorImageProcessing(tempImg);
//            qInfo() << __FUNCTION__ << __LINE__ << " 已采集摄像头画面，正在传递摄像头画面";
        }
        if (yuvPtr != nullptr) {
            delete [] yuvPtr;
            yuvPtr = nullptr;
        }
        if (rgbPtr != nullptr) {
            free(rgbPtr);
            rgbPtr = nullptr;
        }

//        qDebug() << __FUNCTION__ << __LINE__ << "正在释放当前画面帧 >>> 3";
        if (vd != nullptr)
            v4l2core_release_frame(vd, m_frame);
//        qDebug() << __FUNCTION__ << __LINE__ << "已释放释放当前画面帧 >>> 3";
    }
    v4l2core_stop_stream(vd);
    close_v4l2_device_handler();
    qDebug() << "已停止摄像头画面采集线程";
    emit isEnd();
}

MajorImageProcessingThread::~MajorImageProcessingThread()
{
    stop();
    qDebug() << __FUNCTION__;
}
