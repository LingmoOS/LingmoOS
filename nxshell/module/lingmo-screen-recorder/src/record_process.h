// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDPROCESS_H
#define RECORDPROCESS_H

#include "utils/configsettings.h"
#include "utils/voicevolumewatcher.h"
#include "gstrecord/gstrecordx.h"
#ifdef KF5_WAYLAND_FLAGE_ON
#include "waylandrecord/waylandintegration.h"
#endif
#include <QProcess>
#include <QObject>
#include <QRect>
#include <QTimer>
#include <QtConcurrent>

//不需要开启线程，用信号槽代替 process->waitForFinished(-1); 避免线程等待浪费系统资源
/**
 * @brief The RecordProcess class 录屏的控制类
 * 录屏分类两种情况：x11下的录屏 wayland下的录屏
 * x11录屏：通过使用ffmpeg命令进行录屏
 * wayland录屏：通过调用wayland的接口获取屏幕的的帧，进行裁剪后，通过ffmpeg的接口合成视频
 */
class RecordProcess  : public QObject
{
    Q_OBJECT

public:
    static const int RECORD_TYPE_GIF;
    static const int RECORD_TYPE_MP4;
    static const int RECORD_TYPE_MKV;

    static const int RECORD_MOUSE_NULL;
    static const int RECORD_MOUSE_CURSE;
    static const int RECORD_MOUSE_CHECK;
    static const int RECORD_MOUSE_CURSE_CHECK;

    static const int RECORD_AUDIO_NULL;
    static const int RECORD_AUDIO_MIC;
    static const int RECORD_AUDIO_SYSTEMAUDIO;
    static const int RECORD_AUDIO_MIC_SYSTEMAUDIO;

    static const int RECORD_FRAMERATE_5;
    static const int RECORD_FRAMERATE_10;
    static const int RECORD_FRAMERATE_20;
    static const int RECORD_FRAMERATE_24;
    static const int RECORD_FRAMERATE_30;

    explicit RecordProcess(QObject *parent = nullptr);
    ~RecordProcess();

    /**
     * @brief 设置录屏的基本信息
     * @param 录屏的范围
     * @param filename
     */
    void setRecordInfo(const QRect &recordRect, const QString &filename);
    /**
     * @brief 开始录屏
     */
    void startRecord();
    /**
     * @brief 停止录屏
     */
    void stopRecord();
    /**
     * @brief 退出录屏（先停止插件计时，再弹录屏提示，最后退出计时插件）
     */
    void exitRecord(QString newSavePath);
    /**
     * @brief 定时给任务栏插件发送正在录屏的信号
     */
    void emitRecording();
private:
    /**
     * @brief x11协议下ffmpeg录制视频
     */
    void recordVideo();

    /**
     * @brief wayland协议下ffmpeg录制视频
     */
    void waylandRecord();

    /**
     * @brief gstreamer录制视频
     */
    void GstStartRecord();

    /**
     * @brief gstreamer停止录制视频
     */
    void GstStopRecord();

    /**
     * @brief 初始化进程
     */
    void initProcess();

public slots:
    /**
     * @brief 退出gstreamer录屏
     * wayland下录屏，需要Gstreamer录屏类触发，以保证Gstreamer管道中的数据已写完
     * x11下可以直接调用此函数
     */
    void onExitGstRecord();

    /**
     * @brief 从配置文件获取录屏文件的保存目录
     */
    void getScreenRecordSavePath();

    /**
     * @brief 保存到剪切板
     */
    void save2Clipboard(QString file);
private slots:
    /**
     * @brief onRecordFinish:录屏完成
     */
    void onRecordFinish();

    /**
     * @brief onStartTranscode:开始转码
     */
    void onStartTranscode();

    /**
     * @brief onTranscodeFinish:转码完成
     */
    void onTranscodeFinish();

private:
    /**
     * @brief x11录屏进程
     */
    QProcess *m_recorderProcess = nullptr;

    /**
     * @brief 录屏的类型：gif mkv mp4
     */
    int m_recordType = 0;

    /**
     * @brief 录制的声音类型： 混音 单麦克风音频 单系统音频
     */
    int m_audioType = 0;

    /**
     * @brief 是否录制鼠标
     */
    int m_mouseType = true;

    /**
     * @brief 录屏的范围
     */
    QRect m_recordRect;

    QString savePath;
    QString saveBaseName;
    QString saveTempDir;
    QString saveDir;
    QString saveAreaName;
    QString displayNumber;

    ConfigSettings *m_settings = nullptr;

    /**
     * @brief 录屏的帧率
     */
    int m_framerate;
    /**
     * @brief mp4转码成gif的进程
     */
    QProcess *m_pTranscodeProcess = nullptr;
    /**
      * @brief 定时发送录屏正在运行的标志
      */
    bool m_recordingFlag;
    /**
     * @brief gstreamer录屏处理类
     */
    GstRecordX *m_gstRecordX;
};

#endif //RECORDPROCESS_H
