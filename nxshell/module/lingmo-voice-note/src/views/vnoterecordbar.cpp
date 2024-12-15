// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoterecordbar.h"
#include "globaldef.h"
#include "widgets/vnoterecordwidget.h"
#include "widgets/vnote2siconbutton.h"
#include "widgets/vnoteplaywidget.h"
#include "common/vnoteitem.h"
#include "common/audiowatcher.h"
#include "common/setting.h"

#include "dialog/vnotemessagedialog.h"

#include <DApplication>
#include <DLog>

#include <QVBoxLayout>

/**
 * @brief VNoteRecordBar::VNoteRecordBar
 * @param parent
 */
VNoteRecordBar::VNoteRecordBar(QWidget *parent)
    : QWidget(parent)
{
    initUI();
    initConnections();
    initSetting();
    initAudioWatcher();
    onAudioDeviceChange(m_currentMode);
}

/**
 * @brief VNoteRecordBar::initUI
 */
void VNoteRecordBar::initUI()
{
    m_mainLayout = new QStackedLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_recordPanel = new VNoteRecordWidget(this);
    m_recordPanel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    m_mainLayout->addWidget(m_recordPanel);

    m_recordBtnHover = new DWidget(this);
    m_recordBtn = new VNote2SIconButton("circle_record.svg", "circle_record.svg", m_recordBtnHover);
//    m_recordBtn->setFlat(false);
    m_recordBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    m_recordBtn->setIconSize(QSize(34, 34));
    m_recordBtn->setFixedSize(QSize(54, 54));
    QGridLayout *recordBtnHoverLayout = new QGridLayout;
    recordBtnHoverLayout->setContentsMargins(0, 8, 0, 0);
    recordBtnHoverLayout->addWidget(m_recordBtn, 0, 1);
    recordBtnHoverLayout->setColumnStretch(0, 1);
    recordBtnHoverLayout->setColumnStretch(1, 0);
    recordBtnHoverLayout->setColumnStretch(2, 1);
    m_recordBtnHover->setLayout(recordBtnHoverLayout);
    m_mainLayout->addWidget(m_recordBtnHover);

    m_playPanel = new VNotePlayWidget(this);
    m_mainLayout->addWidget(m_playPanel);
    onChangeTheme();
    m_mainLayout->setCurrentWidget(m_recordBtnHover);
}

/**
 * @brief VNoteRecordBar::initConnections
 */
void VNoteRecordBar::initConnections()
{
    //Install filter to filte MousePress
    //event.
    installEventFilter(this);
    m_recordBtn->installEventFilter(this);

    connect(m_recordBtn, &VNote2SIconButton::clicked, this, &VNoteRecordBar::onStartRecord);
    connect(m_recordPanel, SIGNAL(sigFinshRecord(const QString &, qint64)),
            this, SLOT(onFinshRecord(const QString &, qint64)));
    connect(m_playPanel, &VNotePlayWidget::sigWidgetClose,
            this, &VNoteRecordBar::onClosePlayWidget);
    connect(m_playPanel, SIGNAL(sigPlayVoice(VNVoiceBlock *)),
            this, SIGNAL(sigPlayVoice(VNVoiceBlock *)));
    connect(m_playPanel, SIGNAL(sigPauseVoice(VNVoiceBlock *)),
            this, SIGNAL(sigPauseVoice(VNVoiceBlock *)));
    connect(DApplicationHelper::instance(), &DApplicationHelper::themeTypeChanged,
            this, &VNoteRecordBar::onChangeTheme);
}

/**
 * @brief VNoteRecordBar::eventFilter
 * @param o
 * @param e
 * @return false 不过滤事件
 */
bool VNoteRecordBar::eventFilter(QObject *o, QEvent *e)
{
    Q_UNUSED(o);
    //Let NoteItem lost focus when click
    //outside of Note

    if (e->type() == QEvent::MouseButtonPress) {
        setFocus(Qt::MouseFocusReason);
    }

    return false;
}

/**
 * @brief VNoteRecordBar::startRecord
 */
void VNoteRecordBar::startRecord()
{
    m_mainLayout->setCurrentWidget(m_recordPanel);
    if (m_recordPanel->startRecord()) {
        m_recordPanel->setFocus();
        emit sigStartRecord(m_recordPanel->getRecordPath());
    }
}

/**
 * @brief VNoteRecordBar::onStartRecord
 */
void VNoteRecordBar::onStartRecord()
{
    if (this->isVisible()
        && m_mainLayout->currentWidget() == m_recordBtnHover
        && m_recordBtn->isEnabled()) {
        m_recordPanel->setAudioDevice(m_audioWatcher->getDeviceName(
            static_cast<AudioWatcher::AudioMode>(m_currentMode)));
        double volume = m_audioWatcher->getVolume(
            static_cast<AudioWatcher::AudioMode>(m_currentMode));
        m_showVolumeWanning = volumeToolow(volume);
        if (m_showVolumeWanning) {
            VNoteMessageDialog volumeLowDialog(VNoteMessageDialog::VolumeTooLow, this);
            connect(&volumeLowDialog, &VNoteMessageDialog::accepted, this, [this]() {
                //User confirmed record when volume too low
                //start recording anyway.
                startRecord();
            });

            volumeLowDialog.exec();
        } else {
            //Volume normal
            startRecord();
        }
    }
}

/**
 * @brief VNoteRecordBar::onFinshRecord
 * @param voicePath
 * @param voiceSize
 */
void VNoteRecordBar::onFinshRecord(const QString &voicePath, qint64 voiceSize)
{
    m_mainLayout->setCurrentWidget(m_recordBtnHover);
    emit sigFinshRecord(voicePath, voiceSize);
}

/**
 * @brief VNoteRecordBar::cancelRecord
 */
void VNoteRecordBar::stopRecord()
{
    m_recordPanel->stopRecord();
}

/**
 * @brief VNoteRecordBar::onClosePlayWidget
 * @param voiceData
 */
void VNoteRecordBar::onClosePlayWidget(VNVoiceBlock *voiceData)
{
    m_mainLayout->setCurrentWidget(m_recordBtnHover);
    emit sigWidgetClose(voiceData);
}

/**
 * @brief VNoteRecordBar::playVoice
 * @param voiceData
 * @param bIsSame
 */
void VNoteRecordBar::playVoice(VNVoiceBlock *voiceData, bool bIsSame)
{
    //焦点切换到当前窗口，响应播放快捷键
    setFocus();
    m_mainLayout->setCurrentWidget(m_playPanel);
    m_playPanel->playVoice(voiceData, bIsSame);
}

/**
 * @brief VNoteRecordBar::onAudioVolumeChange
 * @param mode
 */
void VNoteRecordBar::onAudioVolumeChange(int mode)
{
    static bool isShowDialog = false;
    if (m_mainLayout->currentWidget() == m_recordPanel
        && m_currentMode == mode) {
        double volume = m_audioWatcher->getVolume(
            static_cast<AudioWatcher::AudioMode>(mode));
        if (!m_showVolumeWanning) {
            m_showVolumeWanning = volumeToolow(volume);
            if (m_showVolumeWanning && isShowDialog == false) {
                VNoteMessageDialog volumeLowDialog(VNoteMessageDialog::VolumeTooLow, this);
                connect(&volumeLowDialog, &VNoteMessageDialog::rejected, this, [this]() {
                    stopRecord();
                });
                connect(&volumeLowDialog, &VNoteMessageDialog::finished, this, [ = ]() {
                    isShowDialog = false;
                });
                isShowDialog = true;
                volumeLowDialog.exec();
            }
        } else {
            if (!volumeToolow(volume)) {
                m_showVolumeWanning = false;
            }
        }
    }
}

/**
 * @brief VNoteRecordBar::onAudioDeviceChange
 * @param mode
 */
void VNoteRecordBar::onAudioDeviceChange(int mode)
{
    qDebug() <<"Current record auido mode(system:0,mic:1):" << m_currentMode <<  " Change record audio mode(system:0,mic:1): " << mode;
    if (m_currentMode == mode) {
        static bool msgshow = false;
        QString info = m_audioWatcher->getDeviceName(
            static_cast<AudioWatcher::AudioMode>(mode));
        qInfo() << "Current Audio Device Name: " << info;
        if (info.isEmpty()) { //切换后的设备异常
            qInfo() << __LINE__ << __func__ << "音频设备异常,已禁用录制按钮";
            qInfo() << "录制按钮是否可点击？" << false;
            m_recordBtn->setEnabled(false);
            m_recordBtn->setToolTip(
                DApplication::translate(
                    "VNoteRecordBar",
                    "No recording device detected"));
            if (m_mainLayout->currentWidget() == m_recordPanel) {
                stopRecord();
                if (!msgshow) {
                    emit sigDeviceExceptionMsgShow();
                    msgshow = true;
                }
            }
        } else { //切换后的设备正常
            if (msgshow) {
                emit sigDeviceExceptionMsgClose();
                msgshow = false;
            }
            //            m_recordBtn->setEnabled(true);
            //根据 系统>控制中心>声音>设备管理>输入设备 中的使能情况设置按钮是否可用
            bool isEnable = m_audioWatcher->getDeviceEnable(static_cast<AudioWatcher::AudioMode>(m_currentMode));
            qInfo() << "录制按钮是否可点击？" << isEnable;
            m_recordBtn->setEnabled(isEnable);
            m_recordBtn->setToolTip("");
            if (m_mainLayout->currentWidget() == m_recordPanel) {
                stopRecord();
            }
        }
    }
}

/**
 * @brief VNoteRecordBar::onAudioSelectChange
 * @param value
 */
void VNoteRecordBar::onAudioSelectChange(QVariant value)
{
    m_currentMode = value.toInt();
    onAudioDeviceChange(m_currentMode);
}

/**
 * @brief VNoteRecordBar::initSetting
 */
void VNoteRecordBar::initSetting()
{
    m_currentMode = setting::instance()->getOption(VNOTE_AUDIO_SELECT).toInt();
    auto option = setting::instance()->getSetting()->option(VNOTE_AUDIO_SELECT);
    connect(option, &DSettingsOption::valueChanged,
            this, &VNoteRecordBar::onAudioSelectChange);
}

/**
 * @brief VNoteRecordBar::initAudioWatcher
 */
void VNoteRecordBar::initAudioWatcher()
{
    m_audioWatcher = new AudioWatcher(this);
    connect(m_audioWatcher, &AudioWatcher::sigDeviceChange,
            this, &VNoteRecordBar::onAudioDeviceChange);
    connect(m_audioWatcher, &AudioWatcher::sigVolumeChange,
            this, &VNoteRecordBar::onAudioVolumeChange);
    connect(m_audioWatcher, &AudioWatcher::sigDeviceEnableChanged,
                this, &VNoteRecordBar::onDeviceEnableChanged);
}

/**
 * @brief VNoteRecordBar::volumeToolow
 * @param volume
 * @return true 录音音量低
 */
bool VNoteRecordBar::volumeToolow(const double &volume)
{
    return (volume - 0.2 < 0.0) ? true : false;
}

void VNoteRecordBar::onChangeTheme()
{
    DPalette palette = DApplicationHelper::instance()->palette(m_recordBtn);
    DGuiApplicationHelper::ColorType theme =
        DGuiApplicationHelper::instance()->themeType();
    QColor highColor = theme == DGuiApplicationHelper::DarkType ? "#B82222" : "#F45959";
    palette.setBrush(DPalette::Highlight, highColor);
    highColor.setAlphaF(0.4);
    palette.setColor(DPalette::Disabled, DPalette::Highlight, highColor);
    m_recordBtn->setPalette(palette);
}

/**
 * @brief VNoteRecordBar::onDeviceEnableChanged
 * 根据 系统>控制中心>声音>设备管理>输入设备 中的使能情况设置按钮是否可用
 * @param mode
 * @param enabled
 */
void VNoteRecordBar::onDeviceEnableChanged(int mode, bool enabled)
{
    qInfo() << "通过控制中心改变默认设备 "<< mode << "（输出设备:0 输入设备:1）使能状态！enalbed:" <<enabled;
    if (1 == mode) {
        qInfo() << "录制按钮是否可点击？" << enabled;
        m_recordBtn->setEnabled(enabled);
    }
}

void VNoteRecordBar::stopPlay()
{
    m_playPanel->onCloseBtnClicked();
}
