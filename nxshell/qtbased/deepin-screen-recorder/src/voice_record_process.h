// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VOICE_RECORD_PROCESS_H
#define VOICE_RECORD_PROCESS_H
#include "settings.h"
#include <QProcess>
#include <QThread>
#include <QRect>
#include <proc/readproc.h>
#include <proc/sysinfo.h>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDateTime>
#include <QTimer>
class VoiceRecordProcess : public QThread
{
    Q_OBJECT

public:
    explicit VoiceRecordProcess(QObject *parent = nullptr);

    QString generateRecordingFilepath();
    QString getRecordingFilepath();

    void exitRecord();
    void pauseRecord();
    void renderLevel(const QAudioBuffer &buffer);
    void resumeRecord();
    void startRecord();
    void stopRecord();
protected:
    void run();
private:
    QAudioProbe *audioProbe;
    QAudioRecorder *audioRecorder;
    QString recordPath;
    QDateTime lastUpdateTime;
    QTimer *tickerTimer;
    float recordingTime;
};
#endif // VOICE_RECORDER_PROCESS_H
