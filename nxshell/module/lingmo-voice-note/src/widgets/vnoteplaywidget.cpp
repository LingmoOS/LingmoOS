// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vnoteplaywidget.h"
#include "vnote2siconbutton.h"
#include "common/vnoteitem.h"
#include "common/utils.h"

#include <DDialogCloseButton>
#include <DFontSizeManager>
#include <DFloatingWidget>
#include <DBlurEffectWidget>
#include <QGridLayout>
#include <QDebug>

/**
 * @brief VNotePlayWidget::VNotePlayWidget
 * @param parent
 */
VNotePlayWidget::VNotePlayWidget(QWidget *parent)
    : DFloatingWidget(parent)
{
    //setFixedHeight(76);
    initUI();
    initPlayer();
    initConnection();
}

/**
 * @brief VNotePlayWidget::initUI
 */
void VNotePlayWidget::initUI()
{
    m_playerBtn = new DIconButton(this);
//    m_playerBtn->setIcon(Utils::loadSVG("play.svg", true));
    m_playerBtn->setIcon(QIcon::fromTheme("dvn_play"));
    m_playerBtn->setIconSize(QSize(36, 36));
    m_playerBtn->setFixedSize(QSize(36, 36));
    m_playerBtn->setFlat(true);
    m_playerBtn->setEnabledCircle(true);

    m_timeLab = new DLabel(this);
    QFont timeLabFont;
    timeLabFont.setPixelSize(10);
    m_timeLab->setFont(timeLabFont);
    m_timeLab->setText("00:00/00:00");
    m_timeLab->setFixedHeight(15);

    m_sliderHover = new DFrame(this);
    DPalette pa = DApplicationHelper::instance()->palette(m_sliderHover);
    QColor splitColor(0,0,0,13);
    pa.setColor(DPalette::Base, splitColor);
    m_sliderHover->setPalette(pa);
    m_sliderHover->setFixedHeight(36);

    m_nameLab = new DLabel(this);
    QFont nameLabFont;
    nameLabFont.setPixelSize(12);
    m_nameLab->setFont(nameLabFont);
    m_nameLab->setFixedHeight(17);

    QHBoxLayout *t_blurAreaLayout = new QHBoxLayout(this);
    t_blurAreaLayout->setContentsMargins(0, 0, 0, 0);
    m_slider = new DSlider(Qt::Horizontal);

    m_slider->setFixedHeight(40);
    m_slider->setMinimum(0);
    m_slider->setValue(0);
    t_blurAreaLayout->addWidget(m_slider, 0, Qt::AlignVCenter);
    m_sliderHover->setLayout(t_blurAreaLayout);

    m_closeBtn = new DIconButton(this);
    //m_closeBtn->setIcon(Utils::loadSVG("clear.svg", true));
    m_closeBtn->setIcon(QIcon::fromTheme("dvn_clear"));
    m_closeBtn->setIconSize(QSize(22, 22));
    m_closeBtn->setFixedSize(QSize(22, 22));
    m_closeBtn->setFlat(true);
    m_closeBtn->setEnabledCircle(true);

    QHBoxLayout *labLayout = new QHBoxLayout;
    labLayout->setContentsMargins(0, 0, 0, 0);
    labLayout->addWidget(m_nameLab, 0, Qt::AlignLeft | Qt::AlignVCenter);
    labLayout->addStretch();
    labLayout->addWidget(m_timeLab, 0, Qt::AlignRight| Qt::AlignVCenter);

    QVBoxLayout *sliderLayout = new QVBoxLayout;
    //语音名和进度条上下边距
    sliderLayout->setContentsMargins(0, 0, 0, 0);
    sliderLayout->addSpacing(2);
    sliderLayout->addWidget(m_sliderHover, Qt::AlignTop);
    sliderLayout->addLayout(labLayout, Qt::AlignBottom);

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->setContentsMargins(20, 9, 20, 7);
    mainLayout->setSpacing(20);
    mainLayout->addWidget(m_playerBtn, 0, Qt::AlignVCenter);
    mainLayout->addLayout(sliderLayout);
    mainLayout->addWidget(m_closeBtn, 0, Qt::AlignVCenter);
    this->setLayout(mainLayout);
    m_sliderHover->installEventFilter(this);
    m_slider->installEventFilter(this);
}

/**
 * @brief VNotePlayWidget::initPlayer
 */
void VNotePlayWidget::initPlayer()
{
    m_player = new VlcPalyer(this);
}

/**
 * @brief VNotePlayWidget::initConnection
 */
void VNotePlayWidget::initConnection()
{
    connect(m_player, &VlcPalyer::positionChanged,
            this, &VNotePlayWidget::onVoicePlayPosChange, Qt::QueuedConnection);
    connect(m_player, &VlcPalyer::durationChanged,
            this, &VNotePlayWidget::onDurationChanged, Qt::QueuedConnection);
    connect(m_player, &VlcPalyer::playEnd,
            this, &VNotePlayWidget::onCloseBtnClicked, Qt::QueuedConnection);

    connect(m_playerBtn, &DIconButton::clicked,
            this, &VNotePlayWidget::onPlayerBtnClicked);

    connect(m_closeBtn, &DIconButton::clicked,
            this, &VNotePlayWidget::onCloseBtnClicked);

    connect(m_slider, &DSlider::sliderPressed,
            this, &VNotePlayWidget::onSliderPressed);
    connect(m_slider, &DSlider::sliderReleased,
            this, &VNotePlayWidget::onSliderReleased);
    connect(m_slider, &DSlider::sliderMoved,
            this, &VNotePlayWidget::onSliderMove);
}

/**
 * @brief VNotePlayWidget::onVoicePlayPosChange
 * @param pos
 */
void VNotePlayWidget::onVoicePlayPosChange(qint64 pos)
{
    if (m_sliderReleased == true) {
        onSliderMove(static_cast<int>(pos));
    }
}

/**
 * @brief VNotePlayWidget::onCloseBtnClicked
 */
void VNotePlayWidget::onCloseBtnClicked()
{
    qDebug() << "Click close button!";
    m_slider->setValue(0);
    m_player->stop();
    m_sliderReleased = true;
    emit sigWidgetClose(m_voiceBlock);
}

/**
 * @brief VNotePlayWidget::onSliderPressed
 */
void VNotePlayWidget::onSliderPressed()
{
    m_sliderReleased = false;
}

/**
 * @brief VNotePlayWidget::onSliderReleased
 */
void VNotePlayWidget::onSliderReleased()
{
    m_sliderReleased = true;
    VlcPalyer::VlcState state = m_player->getState();
    if (state == VlcPalyer::Playing || state == VlcPalyer::Paused) {
        int pos = m_slider->value();
        if (pos >= m_slider->maximum()) {
            onCloseBtnClicked();
        } else {
            if (m_player->getState() == VlcPalyer::Playing) {
                m_player->setPosition(pos);
            }
        }
    }
}

/**
 * @brief VNotePlayWidget::onSliderMove
 * @param pos
 */
void VNotePlayWidget::onSliderMove(int pos)
{
    if (m_voiceBlock) {
#ifdef MPV_PLAYENGINE
        qint64 tmpPos = pos*1000 > m_voiceBlock->voiceSize ? m_voiceBlock->voiceSize : pos*1000;
#else
        qint64 tmpPos = pos > m_voiceBlock->voiceSize ? m_voiceBlock->voiceSize : pos;
#endif
        m_timeLab->setText(Utils::formatMillisecond(tmpPos, 0) + "/" + Utils::formatMillisecond(m_voiceBlock->voiceSize));
        //qDebug() << "Current play time: " << m_timeLab->text();
    }

    if (m_sliderReleased == true) {
        m_slider->setValue(pos);
    }
}

/**
 * @brief VNotePlayWidget::eventFilter
 * @param o
 * @param e
 * @return false 不过滤
 */
bool VNotePlayWidget::eventFilter(QObject *o, QEvent *e)
{
    if (o == m_slider && e->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent *>(e);
        if (keyEvent->key() == Qt::Key_Left) {
            int pos = m_slider->value() - 5000;
            if (pos < 0) {
                pos = 0;
            }
            if (m_player->getState() == VlcPalyer::Playing) {
                m_player->setPosition(pos);
            } else {
                onSliderMove(pos);
            }
        } else if (keyEvent->key() == Qt::Key_Right) {
            int pos = m_slider->value() + 5000;
            if (pos > m_slider->maximum()) {
                pos = m_slider->maximum();
            }
            if (m_player->getState() == VlcPalyer::Playing) {
                m_player->setPosition(pos);
            } else {
                onSliderMove(pos);
            }
        } else {
            return false;
        }
        return true;
    } else {
//        if (e->type() == QEvent::Enter) {
//            m_nameLab->setVisible(false);
//        } else if (e->type() == QEvent::Leave) {
//            m_nameLab->setVisible(true);
//        }
    }
    return false;
}

/**
 * @brief VNotePlayWidget::getPlayerStatus
 * @return 播放状态
 */
VlcPalyer::VlcState VNotePlayWidget::getPlayerStatus()
{
    return m_player->getState();
}

/**
 * @brief VNotePlayWidget::onDurationChanged
 * @param duration
 */
void VNotePlayWidget::onDurationChanged(qint64 duration)
{
    if (duration && m_slider->maximum() != duration) {
        qInfo() << "Get total audio duration changed! (duration: " << duration<< ")";
        m_slider->setMaximum(static_cast<int>(duration));
    }
}

void VNotePlayWidget::onPlayerBtnClicked()
{
    playVoice(m_voiceBlock, true);
}

void VNotePlayWidget::playVoice(VNVoiceBlock *voiceData, bool bIsSame)
{
    if (bIsSame && nullptr != m_voiceBlock) { //与上一次语音相同，执行继续/暂停操作
        qInfo() << "As with the last voice, continue/pause";
        VlcPalyer::VlcState status = getPlayerStatus();
        if (status == VlcPalyer::Playing) { //当前语音播放中则执行暂停操作
            m_player->pause();
            //m_playerBtn->setIcon(Utils::loadSVG("play.svg", true)); //当前语音暂停中则执行播放操作
            m_playerBtn->setIcon(QIcon::fromTheme("dvn_play")); //当前语音暂停中则执行播放操作
            emit sigPauseVoice(m_voiceBlock);
        } else if (status == VlcPalyer::Paused) {
            m_player->play();
            //m_playerBtn->setIcon(Utils::loadSVG("pause_play.svg", true));
            m_playerBtn->setIcon(QIcon::fromTheme("dvn_pauseplay"));
            emit sigPlayVoice(m_voiceBlock);
        }
    } else if (nullptr != voiceData) { //与上一次语音不相同，重新播放语音
        qInfo() << "Different from the last voice, play the voice again";
        m_slider->setValue(0);
        m_voiceBlock = voiceData;
        m_player->setChangePlayFile(true);
        m_player->setFilePath(m_voiceBlock->voicePath);
        m_nameLab->setText(voiceData->voiceTitle);
        m_timeLab->setText(Utils::formatMillisecond(0, 0) + "/" + Utils::formatMillisecond(voiceData->voiceSize));
        //m_playerBtn->setIcon(Utils::loadSVG("pause_play.svg", true));
        m_playerBtn->setIcon(QIcon::fromTheme("dvn_pauseplay"));
        m_player->play();
        emit sigPlayVoice(m_voiceBlock);
    } else {
        qInfo() << "paly voice param is error";
    }
}
