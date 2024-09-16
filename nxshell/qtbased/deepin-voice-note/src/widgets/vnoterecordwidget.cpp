// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoterecordwidget.h"
#include "common/utils.h"
#include <unistd.h>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>

/**
 * @brief VNoteRecordWidget::VNoteRecordWidget
 * @param parent
 */
VNoteRecordWidget::VNoteRecordWidget(QWidget *parent)
    : DFloatingWidget(parent)
{
    initUi();
    initRecord();
    initConnection();
}

/**
 * @brief VNoteRecordWidget::initUi
 */
void VNoteRecordWidget::initUi()
{
    m_recordBtn = new VNote2SIconButton("record.svg", "pause.svg", this);
    m_recordBtn->setIconSize(QSize(28, 28));
    m_recordBtn->setFixedSize(QSize(46, 46));

    m_finshBtn = new VNote2SIconButton("finish_record.svg", "finish_record.svg", this);
    m_finshBtn->setIconSize(QSize(28, 28));
    m_finshBtn->setFixedSize(QSize(46, 46));

    QFont recordTimeFont;
    recordTimeFont.setPixelSize(14);
    m_timeLabel = new DLabel(this);
    m_timeLabel->setFont(recordTimeFont);
    m_timeLabel->setText("00:00");

    m_waveForm = new VNWaveform(this);
    QVBoxLayout *waveLayout = new QVBoxLayout;
    waveLayout->addWidget(m_waveForm);
    waveLayout->setContentsMargins(0, 15, 10, 15);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_recordBtn);
    mainLayout->addLayout(waveLayout, 1);
    mainLayout->addWidget(m_timeLabel);
    mainLayout->addWidget(m_finshBtn);
    mainLayout->setContentsMargins(10, 0, 10, 0);
    onChangeTheme();
    this->setLayout(mainLayout);
}

/**
 * @brief VNoteRecordWidget::initRecord
 */
void VNoteRecordWidget::initRecord()
{
    m_audioRecoder = new GstreamRecorder(this);
}

/**
 * @brief VNoteRecordWidget::initConnection
 */
void VNoteRecordWidget::initConnection()
{
    connect(m_recordBtn, &VNote2SIconButton::clicked, this, &VNoteRecordWidget::onRecordBtnClicked);
    connect(m_finshBtn, &DFloatingButton::clicked, this, &VNoteRecordWidget::stopRecord);
    connect(m_audioRecoder, SIGNAL(audioBufferProbed(const QAudioBuffer &)),
            this, SLOT(onAudioBufferProbed(const QAudioBuffer &)));
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged,
            this, &VNoteRecordWidget::onChangeTheme);
}

/**
 * @brief VNoteRecordWidget::initRecordPath
 */
void VNoteRecordWidget::initRecordPath()
{
    m_recordDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_recordDir += "/voicenote/";
    QDir dir(m_recordDir);
    if (!dir.exists()) {
        dir.mkdir(m_recordDir);
    }
}

/**
 * @brief VNoteRecordWidget::onFinshRecord
 */
void VNoteRecordWidget::stopRecord()
{
    m_audioRecoder->stopRecord();
    QFile f(m_recordPath);
    if (f.open(QIODevice::ReadWrite | QIODevice::Text)) {
        f.flush(); //将用户缓存中的内容写入内核缓冲区
        fsync(f.handle()); //将内核缓冲写入磁盘
        f.close();
    }
    emit sigFinshRecord(m_recordPath, m_recordMsec);
}

void VNoteRecordWidget::onRecordBtnClicked()
{
    bool isPress = m_recordBtn->isPressed();
    if (!isPress) {
        m_audioRecoder->pauseRecord();
    } else {
        m_audioRecoder->startRecord();
    }
}

/**
 * @brief VNoteRecordWidget::startRecord
 * @return true 成功
 */
bool VNoteRecordWidget::startRecord()
{
    QString fileName = QDateTime::currentDateTime()
                           .toString("yyyyMMddhhmmss")
                       + ".mp3";
    initRecordPath();
    m_recordMsec = 0;
    m_recordPath = m_recordDir + fileName;
    m_audioRecoder->setOutputFile(m_recordPath);
    m_timeLabel->setText("00:00");
    bool ret = m_audioRecoder->startRecord();
    if (!ret) {
        stopRecord();
    } else {
        m_recordBtn->setState(VNote2SIconButton::Press);
    }
    return ret;
}

/**
 * @brief VNoteRecordWidget::getRecordPath
 * @return 录音文件路径
 */
QString VNoteRecordWidget::getRecordPath() const
{
    return m_recordPath;
}

/**
 * @brief VNoteRecordWidget::onAudioBufferProbed
 * @param buffer
 */
void VNoteRecordWidget::onAudioBufferProbed(const QAudioBuffer &buffer)
{
    qint64 msec = buffer.startTime();
    if (msec != m_recordMsec) {
        onRecordDurationChange(msec);
    }
    m_waveForm->onAudioBufferProbed(buffer);
}

/**
 * @brief VNoteRecordWidget::onRecordDurationChange
 * @param duration
 */
void VNoteRecordWidget::onRecordDurationChange(qint64 duration)
{
    m_recordMsec = duration;
    QString strTime = Utils::formatMillisecond(duration, 0);
    m_timeLabel->setText(strTime);

    if (duration > (60 * 60 * 1000)) {
        stopRecord();
    }
}

/**
 * @brief VNoteRecordWidget::setAudioDevice
 * @param device
 */
void VNoteRecordWidget::setAudioDevice(QString device)
{
    m_audioRecoder->setDevice(device);
}

void VNoteRecordWidget::onChangeTheme()
{
    DPalette palette = DApplicationHelper::instance()->palette(m_recordBtn);
    DGuiApplicationHelper::ColorType theme =
        DGuiApplicationHelper::instance()->themeType();
    QColor highColor = theme == DGuiApplicationHelper::DarkType ? "#B82222" : "#F45959";
    palette.setBrush(DPalette::Highlight, highColor);
    m_recordBtn->setPalette(palette);
    palette = DApplicationHelper::instance()->palette(m_finshBtn);
    palette.setBrush(DPalette::Highlight, highColor);
    m_finshBtn->setPalette(palette);
}
