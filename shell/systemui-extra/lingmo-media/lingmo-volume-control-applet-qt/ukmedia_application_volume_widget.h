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
#ifndef APPLICATIONVOLUMEWIDGET_H
#define APPLICATIONVOLUMEWIDGET_H
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QAudioInput>
#include <QAudioOutput>
#include <QApplication>
#include "ukmedia_custom_class.h"

class ApplicationVolumeWidget : public QWidget
{
    Q_OBJECT
public:
    ApplicationVolumeWidget(QWidget *parent = nullptr);
    ~ApplicationVolumeWidget();
    void fullushBlueRecordStream();
    void deleteBlueRecordStream();
    void setVolumeSliderRange(bool status);
    friend class DeviceSwitchWidget;
    friend class UkmediaMainWidget;

private:
    void setLabelAlignment(Qt::Alignment alignment);

private:
    QLabel *applicationLabel;
    QLabel *systemVolumeLabel;
    QPushButton *systemVolumeBtn;
    AudioSlider *systemVolumeSlider;
    QLabel *systemVolumeDisplayLabel;

    QWidget *systemVolumeSliderWidget;
    QWidget *displayAppVolumeWidget;
    QStringList *app_volume_list;
    QLabel *appVolumeLabel;
    QPushButton *appMuteBtn;
    QPushButton *appIconBtn;
    QSlider *appSlider;
    QScrollArea *appArea;
    QVBoxLayout *m_pVlayout;

    QFrame * volumeSettingFrame;
    LingmoUISettingButton *volumeSettingButton;

    QFile outputFile;   // class member.
    QAudioInput* audio; // class member.
    QAudioOutput *outAudio;
    bool isRecording = false;

protected:
//    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void onPaletteChanged();

};

#endif // APPLICATIONVOLUMEWIDGET_H
