/*
 * Copyright (C) 2019 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */
#ifndef UKMEDIASYSTEMVOLUMEWIDGET_H
#define UKMEDIASYSTEMVOLUMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QListWidget>
#include <QApplication>
#include "ukmedia_custom_class.h"
#include "../common/ukmedia_common.h"

class UkmediaSystemVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    explicit UkmediaSystemVolumeWidget(QWidget *parent = nullptr);
    friend class UkmediaMainWidget;

    void setVolumeSliderRange(bool status);

private:
    void setLabelAlignment(Qt::Alignment alignment);

Q_SIGNALS:

private:
    QListWidget *m_pInputListWidget;
    QListWidget *m_pOutputListWidget;
    QLabel *m_pOutputLabel;
    QWidget     *m_pSysSliderWidget;
    QLabel      *m_pSystemVolumeLabel;
    AudioSlider     *m_pSystemVolumeSlider;
    QLabel      *m_pSystemVolumeSliderLabel;

    QFrame *volumeSettingFrame;
    LingmoUISettingButton *volumeSettingButton;

    QPushButton *m_pSystemVolumeBtn;

public Q_SLOTS:
    void onPaletteChanged();

};

#endif // UKMEDIASYSTEMVOLUMEWIDGET_H
