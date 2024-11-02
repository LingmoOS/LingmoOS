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
#include "ukmedia_application_volume_widget.h"
#include <QDebug>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QStyleOption>
#include <QPainter>
ApplicationVolumeWidget::ApplicationVolumeWidget(QWidget *parent) : QWidget (parent)
{
    connect(qApp, &QApplication::paletteChanged, this, &ApplicationVolumeWidget::onPaletteChanged);

    app_volume_list = new QStringList;

    //系统音量label
    systemVolumeLabel = new QLabel(tr("System Volume"),this);
    QWidget *systemVolumeLabelWidget = new QWidget(this);
    systemVolumeLabelWidget->setFixedSize(412,29);
    systemVolumeLabel->setFixedSize(372,29);
    QHBoxLayout *systemVolumeLabelLayout = new QHBoxLayout;
    systemVolumeLabelLayout->addWidget(systemVolumeLabel);
    systemVolumeLabelWidget->setLayout(systemVolumeLabelLayout);
    systemVolumeLabelLayout->setContentsMargins(16,0,24,0);

    //应用label
    applicationLabel = new QLabel(tr("Application Volume"),this);
    QWidget *applicationLabelWidget = new QWidget(this);
    applicationLabelWidget->setFixedSize(412,29);
    applicationLabel->setFixedSize(372,29);
    QHBoxLayout *applicationLabelLayout = new QHBoxLayout;
    applicationLabelLayout->addWidget(applicationLabel);
    applicationLabelWidget->setLayout(applicationLabelLayout);
    applicationLabelLayout->setContentsMargins(16,0,24,0);

    //文本颜色置灰
    QPalette palette = applicationLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    applicationLabel->setPalette(palette);
    systemVolumeLabel->setPalette(palette);

    //音量滑动条
    systemVolumeSliderWidget = new QWidget(this);
    systemVolumeSliderWidget->setFixedSize(412,48);

    systemVolumeBtn = new QPushButton(systemVolumeSliderWidget);
    systemVolumeBtn->setFixedSize(36,36);
    systemVolumeBtn->setCheckable(true);
    systemVolumeBtn->setProperty("isRoundButton",true);      //圆形按钮
    systemVolumeBtn->setProperty("useButtonPalette",true);   //灰色按钮
    systemVolumeBtn->setProperty("needTranslucent", true);   //灰色半透明按钮

    systemVolumeSlider = new AudioSlider(systemVolumeSliderWidget);
    systemVolumeSlider->setOrientation(Qt::Horizontal);
    systemVolumeSlider->setFocusPolicy(Qt::StrongFocus);
    systemVolumeSlider->setProperty("needTranslucent", true); // Increase translucent effect
    systemVolumeSlider->setRange(0,100);
    systemVolumeSlider->setFixedSize(286,48);

    systemVolumeDisplayLabel = new QLabel(systemVolumeSliderWidget);
    systemVolumeDisplayLabel->setText("0%");
    systemVolumeDisplayLabel->setFixedSize(52,48);

    //系统音量滑动条布局
    QHBoxLayout *systemVolumeSliderLayout = new QHBoxLayout();
    systemVolumeSliderLayout->addWidget(systemVolumeBtn);
    systemVolumeSliderLayout->addSpacing(0);
    systemVolumeSliderLayout->addWidget(systemVolumeSlider);
    systemVolumeSliderLayout->addWidget(systemVolumeDisplayLabel);
    systemVolumeSliderLayout->setSpacing(0);
    systemVolumeSliderWidget->setLayout(systemVolumeSliderLayout);
    systemVolumeSliderLayout->setContentsMargins(16,0,24,0);
    systemVolumeSlider->setObjectName("systemVolumeSlider");

    //声音设置布局
    volumeSettingButton = new LingmoUISettingButton(volumeSettingFrame);
    volumeSettingButton->setText(tr("Sound Settings"));
    volumeSettingButton->setCursor(Qt::PointingHandCursor);
    volumeSettingButton->setScaledContents(true);
    volumeSettingFrame = new QFrame(this);
    volumeSettingFrame->setFixedHeight(52);
    QHBoxLayout *volumeSettingLayout = new QHBoxLayout(volumeSettingFrame);
    volumeSettingLayout->addWidget(volumeSettingButton);
    volumeSettingLayout->addStretch();
    volumeSettingFrame->setLayout(volumeSettingLayout);
    volumeSettingLayout->setContentsMargins(16,0,24,0);

    //应用音量widget
    appArea = new QScrollArea(this);
    appArea->setFixedSize(412,245);
    appArea->setFrameShape(QFrame::NoFrame);//bjc去掉appArea的边框
    appArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    appArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);//滚动条背景透明

    QPalette pal = appArea->palette();
    pal.setColor(QPalette::Window, QColor(0x00,0xff,0x00,0x00));  //改变appArea背景色透明
    appArea->setPalette(pal);

    displayAppVolumeWidget = new LingmoUIApplicationWidget(appArea);
    displayAppVolumeWidget->setFixedWidth(412);
    displayAppVolumeWidget->setAttribute(Qt::WA_TranslucentBackground);
    appArea->setWidget(displayAppVolumeWidget);

    m_pVlayout = new QVBoxLayout(displayAppVolumeWidget);
    displayAppVolumeWidget->setLayout(m_pVlayout);

//    this->setObjectName("mainWidget");
//    setObjectName("appWidget");

    //整体布局
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(systemVolumeLabelWidget);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(systemVolumeSliderWidget);
    mainLayout->addSpacing(13);
    mainLayout->addWidget(applicationLabelWidget);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(appArea);
    mainLayout->addSpacing(2);
    mainLayout->addWidget(volumeSettingFrame);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(8,12,0,0);
    this->setFixedSize(420,436);
}

void ApplicationVolumeWidget::fullushBlueRecordStream()
{
    isRecording = true;
    outputFile.setFileName("/tmp/test.raw");
    outputFile.open( QIODevice::WriteOnly | QIODevice::Truncate );
    QAudioFormat format;

    format.setSampleRate(8000);
    format.setChannelCount(1);
    format.setSampleSize(8);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);

    format.setSampleType(QAudioFormat::UnSignedInt);
    QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();
    qDebug() << "input device" << info.deviceName();
    if (!info.isFormatSupported(format))
    {
       qWarning()<<"default format not supported try to use nearest";
       format = info.nearestFormat(format);
    }
    qDebug() << "准备蓝牙录音-------";
    audio = new QAudioInput(info, format, this);
    audio->start(&outputFile);
}

void ApplicationVolumeWidget::deleteBlueRecordStream()
{
    isRecording = false;
    qDebug() << "停止录制-------";
    audio->stop();
    outputFile.close();
    system("rm /tmp/test.raw");
    delete audio;
}

void ApplicationVolumeWidget::setLabelAlignment(Qt::Alignment alignment)
{
    systemVolumeDisplayLabel->setAlignment(alignment);
}

void ApplicationVolumeWidget::onPaletteChanged(){
    QPalette palette = applicationLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    applicationLabel->setPalette(palette);
    systemVolumeLabel->setPalette(palette);
}

void ApplicationVolumeWidget::setVolumeSliderRange(bool status)
{
    int maxValue = (status) ? 125 : 100;
    systemVolumeSlider->blockSignals(true);
    systemVolumeSlider->setRange(0, maxValue);
    systemVolumeSlider->blockSignals(false);
}

ApplicationVolumeWidget::~ApplicationVolumeWidget()
{
//    delete  app_volume_list;
//    delete  appIconBtn;
}

