/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#include "ukmedia_sound_effects_widget.h"
#include <QHBoxLayout>
#include <QFile>
#include <QVBoxLayout>
#include <QDebug>
#include <QGuiApplication>

LingmoUIMessageBox::LingmoUIMessageBox()
{
}

UkmediaSoundEffectsWidget::UkmediaSoundEffectsWidget(QWidget *parent) : QWidget(parent)
{
    m_pSystemSoundWidget = new QFrame(this);
    m_pStartupMusicWidget = new QFrame(m_pSystemSoundWidget);
    m_pPoweroffMusicWidget = new QFrame(m_pSystemSoundWidget);
    m_pLagoutWidget = new QFrame(m_pSystemSoundWidget);
    m_pWakeupMusicWidget = new QFrame(m_pSystemSoundWidget);

    m_pAlertSoundFrame = new QFrame(this);
    m_pAlertSoundSwitchWidget = new QFrame(m_pAlertSoundFrame);
    m_pThemeWidget = new QFrame(m_pAlertSoundSwitchWidget);
    m_pNotificationWidget = new QFrame(m_pAlertSoundSwitchWidget);
    m_pVolumeChangeWidget = new QFrame(m_pAlertSoundSwitchWidget);

    m_pSystemSoundWidget->setFrameShape(QFrame::Shape::Box);
    m_pStartupMusicWidget->setFrameShape(QFrame::Shape::Box);
    m_pPoweroffMusicWidget->setFrameShape(QFrame::Shape::Box);
    m_pLagoutWidget->setFrameShape(QFrame::Shape::Box);
    m_pWakeupMusicWidget->setFrameShape(QFrame::Shape::Box);

    m_pAlertSoundFrame->setFrameShape(QFrame::Shape::Box);
    m_pAlertSoundSwitchWidget->setFrameShape(QFrame::Shape::Box);
    m_pThemeWidget->setFrameShape(QFrame::Shape::Box);
    m_pNotificationWidget->setFrameShape(QFrame::Shape::Box);
    m_pVolumeChangeWidget->setFrameShape(QFrame::Shape::Box);

    //~ contents_path /Audio/System Sound   系统音效文本框
    m_pSoundEffectLabel = new TitleLabel(this);
    m_pSoundEffectLabel->setText(tr("System Sound"));
    m_pSoundEffectLabel->setContentsMargins(16,0,16,0);
    m_pSoundEffectLabel->setStyleSheet("QLabel{color: palette(windowText);}");
    //~ contents_path /Audio/Sound Theme
    m_pSoundThemeLabel = new FixLabel(tr("Sound Theme"),m_pThemeWidget);//提示音下的系统音效主题
    m_pSoundThemeCombobox = new QComboBox(m_pThemeWidget);
    //~ contents_path /Audio/Beep Switch
    m_pAlertSoundSwitchLabel = new FixLabel(tr("Beep Switch"),m_pAlertSoundSwitchWidget);//提示音
    //~ contents_path /Audio/Poweroff Music
    m_pPoweroffMusicLabel = new FixLabel(tr("Poweroff Music"),m_pPoweroffMusicWidget);//关机
    //~ contents_path /Audio/Startup Music
    m_pStartupMusicLabel = new FixLabel(tr("Startup Music"),m_pStartupMusicWidget);//开机
    //~ contents_path /Audio/Wakeup Music
    m_pWakeupMusicLabel = new FixLabel(tr("Wakeup Music"),m_pWakeupMusicWidget);//唤醒
    //~ contents_path /Audio/Logout Music
    m_pLagoutLabel = new FixLabel(tr("Logout Music"),m_pLagoutWidget);//注销
    //~ contents_path /Audio/Notification Sound
    m_pNotificationLabel = new FixLabel(tr("Notification Sound"),m_pNotificationWidget);//提示音下的通知提示
    m_pNotificationCombobox = new QComboBox(m_pNotificationWidget);
    m_pNotificationCombobox->setObjectName("notifyCbox");
    //~ contents_path /Audio/Volume Control Sound
    m_pVolumeChangeLabel = new FixLabel(tr("Volume Control Sound"),m_pVolumeChangeWidget);//音量调节
    m_pVolumeChangeCombobox = new QComboBox(m_pVolumeChangeWidget);
    m_pVolumeChangeCombobox->setObjectName("volChangeCbox");

    m_pStartupButton = new KSwitchButton(m_pStartupMusicWidget);
    m_pLogoutButton = new KSwitchButton(m_pLagoutWidget);
    m_pWakeupMusicButton = new KSwitchButton(m_pWakeupMusicWidget);
    m_pPoweroffButton = new KSwitchButton(m_pPoweroffMusicWidget);
    m_pAlertSoundSwitchButton = new KSwitchButton(m_pAlertSoundSwitchWidget);

    //设置大小
    m_pSystemSoundWidget->setMinimumSize(550,0);
    m_pSystemSoundWidget->setMaximumSize(16777215,240);
    m_pStartupMusicWidget->setMinimumSize(550,60);
    m_pStartupMusicWidget->setMaximumSize(16777215,60);
    m_pPoweroffMusicWidget->setMinimumSize(550,60);
    m_pPoweroffMusicWidget->setMaximumSize(16777215,60);
    m_pLagoutWidget->setMinimumSize(550,60);
    m_pLagoutWidget->setMaximumSize(16777215,60);
    m_pWakeupMusicWidget->setMinimumSize(550,60);
    m_pWakeupMusicWidget->setMaximumSize(16777215,60);

    m_pAlertSoundSwitchWidget->setMinimumSize(550,60);
    m_pAlertSoundSwitchWidget->setMaximumSize(16777215,60);
    m_pThemeWidget->setMinimumSize(550,60);
    m_pThemeWidget->setMaximumSize(16777215,60);

    m_pSoundEffectLabel->setFixedSize(150,32);
    m_pSoundThemeLabel->setFixedSize(150,40);
    m_pPoweroffMusicLabel->setFixedSize(150,32);
    m_pSoundThemeCombobox->setFixedHeight(40);

    m_pNotificationWidget->setMinimumSize(550,60);
    m_pNotificationWidget->setMaximumSize(16777215,60);
    m_pVolumeChangeWidget->setMinimumSize(550,60);
    m_pVolumeChangeWidget->setMaximumSize(16777215,60);
    m_pNotificationCombobox->setFixedHeight(40);
    m_pVolumeChangeCombobox->setFixedHeight(40);
    m_pVolumeChangeLabel->setFixedSize(220,40);
    m_pNotificationLabel->setFixedSize(220,40);

    //添加布局
    QHBoxLayout *themeLayout = new QHBoxLayout(m_pThemeWidget);
    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    themeLayout->addWidget(m_pSoundThemeLabel);
    themeLayout->addItem(new QSpacerItem(83,20,QSizePolicy::Fixed));
    themeLayout->addWidget(m_pSoundThemeCombobox);
    themeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    themeLayout->setSpacing(0);
    m_pThemeWidget->setLayout(themeLayout);
    m_pThemeWidget->layout()->setContentsMargins(0,0,0,0);

    QHBoxLayout *NotificationLayout = new QHBoxLayout(m_pNotificationWidget);
    NotificationLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    NotificationLayout->addWidget(m_pNotificationLabel);
    NotificationLayout->addItem(new QSpacerItem(13,20,QSizePolicy::Fixed));
    NotificationLayout->addWidget(m_pNotificationCombobox);
    NotificationLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    NotificationLayout->setSpacing(0);
    m_pNotificationWidget->setLayout(NotificationLayout);
    NotificationLayout->layout()->setContentsMargins(0,0,0,0);

    //开机音乐设置开关
    QHBoxLayout *startupLayout = new QHBoxLayout(m_pStartupMusicWidget);
    startupLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    startupLayout->addWidget(m_pStartupMusicLabel);
    startupLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    startupLayout->addWidget(m_pStartupButton);
    startupLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    startupLayout->setSpacing(0);
    m_pStartupMusicWidget->setLayout(startupLayout);
    m_pStartupMusicWidget->layout()->setContentsMargins(0,0,0,0);

    //注销提示音布局
    QHBoxLayout *lagoutLayout = new QHBoxLayout(m_pLagoutWidget);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    lagoutLayout->addWidget(m_pLagoutLabel);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    lagoutLayout->addWidget(m_pLogoutButton);
    lagoutLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    lagoutLayout->setSpacing(0);
    m_pLagoutWidget->setLayout(lagoutLayout);
    m_pLagoutWidget->layout()->setContentsMargins(0,0,0,0);
//    m_pLagoutCombobox->setVisible(false);

    //提示音开关布局
    QHBoxLayout *alertSoundLayout = new QHBoxLayout(m_pAlertSoundSwitchWidget);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundLayout->addWidget(m_pAlertSoundSwitchLabel);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    alertSoundLayout->addWidget(m_pAlertSoundSwitchButton);
    alertSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    alertSoundLayout->setSpacing(0);
    m_pAlertSoundSwitchWidget->setLayout(alertSoundLayout);
    m_pAlertSoundSwitchWidget->layout()->setContentsMargins(0,0,0,0);

    //窗口关闭提示音
    QHBoxLayout *wakeupMusicLayout = new QHBoxLayout(m_pWakeupMusicWidget);
    wakeupMusicLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    wakeupMusicLayout->addWidget(m_pWakeupMusicLabel);
    wakeupMusicLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    wakeupMusicLayout->addWidget(m_pWakeupMusicButton);
    wakeupMusicLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    wakeupMusicLayout->setSpacing(0);
    m_pWakeupMusicWidget->setLayout(wakeupMusicLayout);
    m_pWakeupMusicWidget->layout()->setContentsMargins(0,0,0,0);

    //音量改变提示音
    QHBoxLayout *volumeChangedLayout = new QHBoxLayout(m_pVolumeChangeWidget);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->addWidget(m_pVolumeChangeLabel);
    volumeChangedLayout->addItem(new QSpacerItem(13,20,QSizePolicy::Fixed));
    volumeChangedLayout->addWidget(m_pVolumeChangeCombobox);
    volumeChangedLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeChangedLayout->setSpacing(0);
    m_pVolumeChangeWidget->setLayout(volumeChangedLayout);
    m_pVolumeChangeWidget->layout()->setContentsMargins(0,0,0,0);

    //关机提示音
    QHBoxLayout *poweroffLayout = new QHBoxLayout(m_pPoweroffMusicWidget);
    poweroffLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    poweroffLayout->addWidget(m_pPoweroffMusicLabel);
    poweroffLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    poweroffLayout->addWidget(m_pPoweroffButton);
    poweroffLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    poweroffLayout->setSpacing(0);
    m_pPoweroffMusicWidget->setLayout(poweroffLayout);
    m_pPoweroffMusicWidget->layout()->setContentsMargins(0,0,0,0);

    QFrame *startupAndPoweroff = myLine();
    QFrame *poweroffAndLagout = myLine();
    QFrame *lagoutAndWakeup = myLine();
    line1 = myLine();
    line2 = myLine();
    line3 = myLine();

    QVBoxLayout *systemSoundLayout = new QVBoxLayout(m_pSystemSoundWidget);
    systemSoundLayout->setContentsMargins(0,0,0,0);
    systemSoundLayout->addWidget(m_pStartupMusicWidget);
    systemSoundLayout->addWidget(startupAndPoweroff);
    systemSoundLayout->addWidget(m_pPoweroffMusicWidget);
    systemSoundLayout->addWidget(poweroffAndLagout);
    systemSoundLayout->addWidget(m_pLagoutWidget);
    systemSoundLayout->addWidget(lagoutAndWakeup);
    systemSoundLayout->addWidget(m_pWakeupMusicWidget);
    systemSoundLayout->setSpacing(0);
    m_pSystemSoundWidget->setLayout(systemSoundLayout);

    QVBoxLayout *SoundLayout = new QVBoxLayout(m_pAlertSoundFrame);
    SoundLayout->setContentsMargins(0,0,0,0);
    SoundLayout->addWidget(m_pAlertSoundSwitchWidget);
    SoundLayout->addWidget(line1);
    SoundLayout->addWidget(m_pThemeWidget);
    SoundLayout->addWidget(line2);
    SoundLayout->addWidget(m_pVolumeChangeWidget);
    SoundLayout->addWidget(line3);
    SoundLayout->addWidget(m_pNotificationWidget);
    SoundLayout->setSpacing(0);
    m_pAlertSoundFrame->setLayout(SoundLayout);

    //进行整体布局
    m_pSoundLayout = new QVBoxLayout(this);
    m_pSoundLayout->addWidget(m_pSoundEffectLabel);
    m_pSoundLayout->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pSystemSoundWidget);
    m_pSoundLayout->addItem(new QSpacerItem(16,8,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pAlertSoundFrame);

    this->setLayout(m_pSoundLayout);
    m_pSoundLayout->setSpacing(0);
    this->layout()->setContentsMargins(0,0,0,0);
}

void UkmediaSoundEffectsWidget::alertSoundWidgetShow(bool status)
{
    if (status) {
        m_pThemeWidget->show();
        m_pVolumeChangeWidget->show();
        m_pNotificationWidget->show();
        line1->show();
        line2->show();
        line3->show();
    } else {
        m_pThemeWidget->hide();
        m_pVolumeChangeWidget->hide();
        m_pNotificationWidget->hide();
        line1->hide();
        line2->hide();
        line3->hide();
    }
}

QFrame* UkmediaSoundEffectsWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

UkmediaSoundEffectsWidget::~UkmediaSoundEffectsWidget()
{

}
