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
#ifndef UKMEDIASOUNDEFFECTSWIDGET_H
#define UKMEDIASOUNDEFFECTSWIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QMessageBox>
#include <QHBoxLayout>
#include <QStyledItemDelegate>
#include "lingmo_custom_style.h"
#include "kswitchbutton.h"

using namespace kdk;


class LingmoUIMessageBox : public QMessageBox
{
public:
    explicit LingmoUIMessageBox();
};

class UkmediaSoundEffectsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaSoundEffectsWidget(QWidget *parent = nullptr);
    ~UkmediaSoundEffectsWidget();
    friend class UkmediaMainWidget;
    void alertSoundWidgetShow(bool status);

Q_SIGNALS:

public Q_SLOTS:

private:
    QFrame *myLine();

private:
    QFrame *m_pSystemSoundWidget;//存放注销、唤醒、开机、关机提示音的窗口
    QFrame *m_pLagoutWidget;//注销提示音窗口
    QFrame *m_pWakeupMusicWidget;//唤醒音乐窗口
    QFrame *m_pStartupMusicWidget;//设置开机音乐窗口
    QFrame *m_pPoweroffMusicWidget;//关机提示音

    QFrame *m_pAlertSoundFrame;//存放注销、唤醒、开机、关机提示音的窗口
    QFrame *m_pAlertSoundSwitchWidget;//提示音开关窗口
    QFrame *m_pThemeWidget;//提示音下的系统音效主题

    QString qss;
    QStyledItemDelegate *itemDelegate;
    TitleLabel *m_pSoundEffectLabel;
    FixLabel *m_pSoundThemeLabel;
    FixLabel *m_pLagoutLabel;
    FixLabel *m_pAlertSoundSwitchLabel;
    FixLabel *m_pWakeupMusicLabel;
    FixLabel *m_pPoweroffMusicLabel;
    FixLabel *m_pStartupMusicLabel;

    QComboBox *m_pSoundThemeCombobox;
    QVBoxLayout *m_pSoundLayout;

    KSwitchButton *m_pStartupButton;
    KSwitchButton *m_pPoweroffButton;
    KSwitchButton *m_pLogoutButton;
    KSwitchButton *m_pAlertSoundSwitchButton;
    KSwitchButton *m_pWakeupMusicButton;

    FixLabel *m_pVolumeChangeLabel;
    FixLabel *m_pNotificationLabel;
    QComboBox *m_pVolumeChangeCombobox;
    QComboBox *m_pNotificationCombobox;
    QFrame *m_pVolumeChangeWidget;//提示音下的音量调节
    QFrame *m_pNotificationWidget;//提示音下的通知提示
    QFrame *line1;
    QFrame *line2;
    QFrame *line3;


//    UkmediaVolumeSlider *m_pAlertSlider;
//    LingmoUIButtonDrawSvg *m_pAlertIconBtn;
};

#endif // UKMEDIASOUNDEFFECTSWIDGET_H
