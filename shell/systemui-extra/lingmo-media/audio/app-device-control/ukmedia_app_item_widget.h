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
#ifndef UKMEDIA_ITEM_WIDGET_H
#define UKMEDIA_ITEM_WIDGET_H

#include <QLabel>
#include <QSlider>
#include <QWidget>
#include <QComboBox>
#include <QBoxLayout>
#include <QDBusError>
#include <QDBusReply>
#include <QGSettings>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingReply>
#include <QDBusPendingCallWatcher>
#include "kslider.h"
#include "../lingmo_custom_style.h"
#include "../common/ukmedia_common.h"

using namespace kdk;

class UkmediaAppItemWidget : public QWidget
{
    Q_OBJECT
public:
    UkmediaAppItemWidget(QWidget *parent = nullptr);
    ~UkmediaAppItemWidget();
    friend class UkmediaAppCtrlWidget;

    void initUI();
    void addInputCombobox(QString port);
    void addOutputCombobox(QString port);
    void setSliderValue(int value);
    void outputVolumeDarkThemeImage(int value, bool status);
    void setTitleName(QString name);        //设置应用标题名
    void setChildObjectName(QString name);  //设置控件object name
    void setInputHintWidgetShow(bool status);
    void setOutputHintWidgetShow(bool status);

private:
    QFrame* myLine();
//    QFrame *line1;
//    QFrame *line2;
    QFrame *m_pAppWidget;
    QFrame *m_pInputWidget;
    QFrame *m_pOutputWidget;
    QFrame *m_pVolumeWidget;
    QFrame *m_pCheckWidget;

    FixLabel *m_pAppLabel;
    FixLabel *m_pVolumeLabel;
    FixLabel *m_pInputDevLabel;
    FixLabel *m_pOutputDevLabel;
    FixLabel *m_pVolumeNumLabel;

    QLabel *m_pInputHintIcon;
    QLabel *m_pOutputHintIcon;
    QFrame *m_pInputHintWidget;
    QFrame *m_pOutputHintWidget;
    FixLabel *m_pInputHintLabel;
    FixLabel *m_pOutputHintLabel;

    KSlider *m_pVolumeSlider;
    QComboBox *m_pInputCombobox;
    QComboBox *m_pOutputCombobox;
    QPushButton *m_pConfirmBtn;
    LingmoUIButtonDrawSvg *m_pVolumeBtn;

    QString mThemeName;
    void onPaletteChanged();
};

#endif //UKMEDIA_ITEM_WIDGET_H
