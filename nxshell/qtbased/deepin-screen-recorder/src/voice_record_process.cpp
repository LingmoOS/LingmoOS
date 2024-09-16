// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "voice_record_process.h"
#include <QAudioEncoderSettings>
#include <QAudioProbe>
#include <QAudioRecorder>
#include <QDir>
#include <QTime>
#include <QUrl>
#include "utils.h"
#include <QDebug>


VoiceRecordProcess::VoiceRecordProcess(QObject *parent) : QThread(parent)
{

    audioRecorder = new QAudioRecorder(this);
    qDebug() << "support codecs:" << audioRecorder->supportedAudioCodecs();
    qDebug() << "support containers:" << audioRecorder->supportedContainers();

    QAudioEncoderSettings audioSettings;
    audioSettings.setQuality(QMultimedia::HighQuality);

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 0)
    audioRecorder->setAudioSettings(audioSettings);
    audioRecorder->setContainerFormat("audio/x-wav");
#else
    audioSettings.setCodec("audio/PCM");
    audioRecorder->setAudioSettings(audioSettings);
    audioRecorder->setContainerFormat("wav");
#endif

    audioProbe = new QAudioProbe(this);
    if (audioProbe->setSource(audioRecorder)) {
        connect(audioProbe, SIGNAL(audioBufferProbed(QAudioBuffer)), this, SLOT(renderLevel(QAudioBuffer)));
    }
}

void VoiceRecordProcess::run()
{
    recordPath = generateRecordingFilepath();
    qDebug() << "recordPath" + recordPath;
    audioRecorder->setOutputLocation(recordPath);
    QDateTime currentTime = QDateTime::currentDateTime();
    lastUpdateTime = currentTime;
    audioRecorder->record();
}

void VoiceRecordProcess::startRecord()
{
    QThread::start();
}
void VoiceRecordProcess::stopRecord()
{
    audioRecorder->stop();
    //Todo: 记录时间
}
void VoiceRecordProcess::exitRecord()
{
    stopRecord();
    //记录文件处理
}
void VoiceRecordProcess::pauseRecord()
{
    audioRecorder->pause();
}

void VoiceRecordProcess::resumeRecord()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    lastUpdateTime = currentTime;
    audioRecorder->record();
}

void VoiceRecordProcess::renderLevel(const QAudioBuffer &buffer)
{
    Q_UNUSED(buffer);
}

QString VoiceRecordProcess::generateRecordingFilepath()
{
    return QDir(Utils::getRecordingSaveDirectory()).filePath(QString("%1 (%2).wav").arg(tr("New recording")).arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss")));
}

QString VoiceRecordProcess::getRecordingFilepath()
{
    return recordPath;
}


