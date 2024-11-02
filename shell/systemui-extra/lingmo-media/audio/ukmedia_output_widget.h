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
#ifndef UKMEDIAOUTPUTWIDGET_H
#define UKMEDIAOUTPUTWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QEvent>
#include <QMouseEvent>
#include <QPushButton>
#include <QCoreApplication>
#include <QVBoxLayout>
#include "lingmo_custom_style.h"
#include "lingmo_list_widget_item.h"
#include "customstyle.h"
#include <QListWidget>
#include <QComboBox>
#include "kswitchbutton.h"
#include <QApplication>

using namespace kdk;

class UkmediaOutputWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaOutputWidget(QWidget *parent = nullptr);
    ~UkmediaOutputWidget();

    void setVolumeSliderRange(bool status);
    friend class UkmediaMainWidget;
Q_SIGNALS:

public Q_SLOTS:
    void onPaletteChanged();

private:
    void setLabelAlignment(Qt::Alignment alignment);
    QFrame* myLine();

private:
    QFrame *m_pOutputWidget;
    QFrame *m_pMasterVolumeWidget;
    QFrame *m_pVolumeIncreaseWidget;
    QWidget *VolumeIncreaseTipsWidget;

    QFrame *m_pAudioBlanceWidget;       //monoAudio
    QFrame *m_pMonoAudioWidget;
    QFrame *m_pChannelBalanceWidget;
    QFrame *m_pMonoLine;

    QWidget *MonoAudioTipsWidget;
    FixLabel *m_pMonoAudioLabel;
    FixLabel *MonoAudioTipsLabel;
    KSwitchButton *m_pMonoAudioButton;

    TitleLabel *m_pOutputLabel;
    FixLabel *m_pOutputDeviceLabel;
    FixLabel *m_pOpVolumeLabel;
    QLabel *m_pOpVolumePercentLabel;
    FixLabel *m_pOpBalanceLabel;
    FixLabel *VolumeIncreaseTipsLabel;
    FixLabel *m_pLeftBalanceLabel;
    FixLabel *m_pRightBalanceLabel;
    FixLabel *m_pVolumeIncreaseLabel;

    QComboBox *m_pDeviceSelectBox;
    QFrame *m_pOutputSlectWidget;

    LingmoUIButtonDrawSvg *m_pOutputIconBtn;
    AudioSlider *m_pOpVolumeSlider;
    UkmediaVolumeSlider *m_pOpBalanceSlider;
    KSwitchButton *m_pVolumeIncreaseButton;

    QVBoxLayout *m_pVlayout;
    QString sliderQss;
};

#endif // UKMEDIAOUTPUTWIDGET_H
