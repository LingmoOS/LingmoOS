// Copyright (C) 2019 ~ 2019 UnionTech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSTREAMRECORDER_H
#define GSTREAMRECORDER_H

#include <QObject>
#include <QMutex>
#include <QAudioBuffer>
#include <QAudioDeviceInfo>

#include <gst/gst.h>

class GstreamRecorder : public QObject
{
    Q_OBJECT
public:
    explicit GstreamRecorder(QObject *parent = nullptr);
    ~GstreamRecorder();
    //开始录音
    bool startRecord();
    //暂停录音
    void pauseRecord();
    //停止录音
    void stopRecord();
    //设置录音设备名称
    void setDevice(const QString &device);
    //设置录音文件
    void setOutputFile(const QString &path);
    //处理gstreamer总线消息
    bool doBusMessage(GstMessage *message);
    //获取编码器的输入数据
    bool doBufferProbe(GstBuffer *buffer);
    //设置录音状态为NULL
    void setStateToNull();

private slots:
    //处理编码器的输入数据
    void bufferProbed();
Q_SIGNALS:
    //录音过程中发生错误，发送错误信息
    void errorMsg(QString msg);
    //语音数据发生变化
    void audioBufferProbed(const QAudioBuffer &buffer);

private:
    //创建录音流水线通道
    bool createPipe();
    //对象使用计数减1
    void objectUnref(gpointer object);
    //初始化数据格式
    void initFormat();
    //资源释放
    void deinit();
    //获取录音状态
    void GetGstState(int *state, int *pending);

    GstElement *m_pipeline {nullptr};
    QString m_outputFile {""};
    QString m_currentDevice;
    QAudioBuffer m_pendingBuffer;
    QMutex m_bufferMutex;
    QAudioFormat m_format;
};

#endif // GSTREAMRECORDER_H
