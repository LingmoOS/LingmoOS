// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VNOTERECORDWIDGET_H
#define VNOTERECORDWIDGET_H

#include "widgets/vnote2siconbutton.h"
#include "widgets/vnwaveform.h"
#include "common/gstreamrecorder.h"

#include <DFloatingWidget>
#include <DLabel>
#include <DWidget>

DWIDGET_USE_NAMESPACE

class VNoteRecordWidget : public DFloatingWidget
{
    Q_OBJECT
public:
    explicit VNoteRecordWidget(QWidget *parent = nullptr);
    //开始录音
    bool startRecord();
    //结束录音
    void stopRecord();
    //设置录音设备
    void setAudioDevice(QString device);
    //获取录音文件路径
    QString getRecordPath() const;

signals:
    void sigFinshRecord(const QString &voicePath, qint64 voiceSize);

public slots:
    void onRecordBtnClicked();
    //录音时长改变
    void onRecordDurationChange(qint64 duration);
    //录音数据改变
    void onAudioBufferProbed(const QAudioBuffer &buffer);
    void onChangeTheme();

private:
    //初始化UI布局
    void initUi();
    //创建文件保存路径
    void initRecordPath();
    //初始化录音库
    void initRecord();
    //连接槽函数
    void initConnection();

private:
    VNote2SIconButton *m_recordBtn {nullptr};
    VNote2SIconButton *m_finshBtn {nullptr};
    DLabel *m_timeLabel {nullptr};
    VNWaveform *m_waveForm {nullptr};
    GstreamRecorder *m_audioRecoder {nullptr};
    QString m_recordDir {""};
    QString m_recordPath {""};
    qint64 m_recordMsec {0};
};

#endif // VNOTERECORDWIDGET_H
