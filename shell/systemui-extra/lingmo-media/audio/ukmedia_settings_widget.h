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
#ifndef UKMEDIA_SETTINGS_WIDGET_H
#define UKMEDIA_SETTINGS_WIDGET_H

#include <QWidget>
#include <QComboBox>
#include <QLabel>
#include <QHBoxLayout>
#include "lingmo_custom_style.h"
#include "kswitchbutton.h"

using namespace kdk;

class UkmediaSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaSettingsWidget(QWidget *parent = nullptr);
    ~UkmediaSettingsWidget();
    friend class UkmediaMainWidget;

private:
    QFrame *myLine();

private:
    QFrame *m_pAdvancedSettingsWidget;  //用于存放声音设备管理、应用声音的窗口的窗口
    QFrame *m_pEquipmentControlWidget;  //声音设备管理窗口
    QFrame *m_pAppSoundCtrlWidget;      //应用声音窗口

    TitleLabel *m_pAdvancedSettingsLabel;

    QLabel *m_pAppSoundLabel;
    QLabel *m_pEquipmentControlLabel;
    QPushButton *m_pAppSoundDetailsBtn;
    QPushButton *m_pDevControlDetailsBtn;
};

#endif // UKMEDIA_SETTINGS_WIDGET_H
