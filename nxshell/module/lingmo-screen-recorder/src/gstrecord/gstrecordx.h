// Copyright (C) 2020 ~ now Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef GSTRECORDX_H
#define GSTRECORDX_H
#include "gstinterface.h"
#include <QDebug>

#include <QString>
#include <QRect>
#include <QMutex>
#include <QDateTime>
#include <QtConcurrent>
#include <QObject>

#include <gst/gst.h>

class Utils;
/**
 * @brief 此类是Gstreamer在x11环境下进行录屏的处理类
 */
class GstRecordX : public QObject
{
    Q_OBJECT
public:
    enum AudioType {
        None = 0,  //无声音
        Mic = 1, //单麦克风
        Sys, //单系统音
        Mix //混音

    };
    enum VideoType {
        webm = 0,
        ogg
    };
public:
    GstRecordX(QObject *parent = nullptr);
    ~GstRecordX();

    /**
     * @brief x11协议下gstreamer录制管道构建及启动录制视频
     */
    void x11GstStartRecord();

    /**
     * @brief x11协议下gstreamer停止录制视频
     */
    void x11GstStopRecord();

    /**
     * @brief wayland协议下gstreamer录制管道构建及启动录制视频
     */
    void waylandGstStartRecord();

    /**
     * @brief wayland协议下gstreamer停止录制视频
     */
    void waylandGstStopRecord();

    /**
     * @brief wayland下写入视频帧
     */
    bool waylandWriteVideoFrame(const unsigned char *frame, const int framewidth, const int frameheight);

    /**
     * @brief 设置输入设备名称
     * 设备名称即使为空，也不影响功能
     * 设备名称不会用作判断是否采集音频，只是存储设备名称
     * 判断是否采集有另外的字段
     * @param device:输入设备名称（麦克风）
     */
    void setInputDeviceName(const QString &device);

    /**
     * @brief 设置输出设备名称
     * 设备名称即使为空，也不影响功能
     * 设备名称不会用作判断是否采集音频，只是存储设备名称
     * 判断是否采集有另外的字段
     * @param device:输出设备名称（扬声器等）
     */
    void setOutputDeviceName(const QString &device);


    /**
     * @brief 设置音频类型
     */
    void setAudioType(AudioType audioType);

    /**
     * @brief 设置视频类型
     */
    void setVidoeType(VideoType videoType);

    /**
     * @brief 设置视频帧率
     * @param 视频帧率
     */
    void setFramerate(const int &framerate);

    /**
     * @brief 设置录制区域的位置及大小
     * @param 录制区域的位置及大小
     */
    void setRecordArea(const QRect &recordArea);

    /**
     * @brief 设置保存路径
     * @param 保存路径
     */
    void setSavePath(const QString &savePath);

    /**
     * @brief 设置x11录制光标
     * @param 是否录制光标
     */
    void setX11RecordMouse(const bool recordMouse);

    /**
     * @brief 设置主板供应商类型
     * @param boardVendorType: 1:hw主板 0:非hw主板和kunpeng(hw主板)
     */
    void setBoardVendorType(int boardVendorType);

    GMainLoop *getGloop() {return m_gloop;}

signals:
    /**
     * @brief Gstreamer录屏已结束
     */
    void waylandGstRecrodFinish();

private:

    /**
     * @brief 创建Gstreamer录屏管道，x11和wayland可共用
     * @return
     */
    bool createPipeline(QStringList);

    /**
     * @brief 根据传入的参数获取，音频管道创建命令
     * @param audioDevName:音频设备名称
     * @param audioType:音频类型（字符串 mic sys）
     * @param arg:管道参数（mix(音频混流器) mux(复用器)）
     * @return 音频管道命令
     */
    QString getAudioPipeline(const QString &audioDevName, const QString &audioType, const QString &arg);

    /**
     * @brief 停止管道，x11和wayland可共用
     */
    void stopPipeline();

    /**
     * @brief 初始化成员变量
     */
    void initMemberVariables();

    /**
     * @brief 格式化输出gstreamer命令
     * @param pipeline：管道命令
     */
    void pipelineStructuredOutput(QString pipeline);

    /**
     * @brief getGstBin
     * @return
     */
    GstBin *getGstBin(GstElement *element);
private:
    /**
     * @brief gstreamer的管道元素
     */
    GstElement *m_pipeline;

    GMainLoop *m_gloop;

    /**
     * @brief 音频类型
     */
    AudioType m_audioType;

    /**
     * @brief 视频类型
     */
    VideoType m_videoType;

    /**
     * @brief 系统音频设备名称
     */
    QString m_sysDevcieName;

    /**
     * @brief 麦克风音频设备名称
     */
    QString m_micDeviceName;

    /**
     * @brief 保存路径
     */
    QString m_savePath;

    /**
     * @brief 主板供应商类型:1:hw主板 0:非hw主板和kunpeng(hw主板)
     */
    int m_boardVendorType;

    /********录制的视频参数********/
    /**
     * @brief 视频帧率
     */
    int m_framerate;

    /**
     * @brief 录制区域的位置及大小
     */
    QRect m_recordArea;

    /**
     * @brief 是否录制光标
     */
    QString m_isRecordMouse;

    /********录制的音频参数**（不对外暴露set接口）******/
    /**
     * @brief 音频通道
     */
    int m_channels;

    /**
     * @brief 音频采样率
     */
    int m_rate;

};

#endif // GSTRECORDX_H
