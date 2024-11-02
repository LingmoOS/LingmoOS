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
#include "ukmedia_input_widget.h"
#include <QDebug>
#include <QFile>
#include <QApplication>

UkmediaInputWidget::UkmediaInputWidget(QWidget *parent) : QWidget(parent)
{
    connect(qApp, &QApplication::paletteChanged, this, &UkmediaInputWidget::onPaletteChanged);

    m_pInputWidget      = new QFrame(this);
    m_pVolumeWidget     = new QFrame(m_pInputWidget);
    m_pInputLevelWidget = new QFrame(m_pInputWidget);
    m_pInputSlectWidget = new QFrame(m_pInputWidget);
    m_pNoiseReducteWidget = new QFrame(m_pInputWidget);
    m_pLoopBackWidget     = new QFrame(m_pInputWidget);

    m_pInputWidget->setFrameShape(QFrame::Shape::Box);
    m_pVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputLevelWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputSlectWidget->setFrameShape(QFrame::Shape::Box);
    m_pNoiseReducteWidget->setFrameShape(QFrame::Shape::Box);
    m_pLoopBackWidget->setFrameShape(QFrame::Shape::Box);

    //设置大小
    m_pVolumeWidget->setMinimumSize(550,60);
    m_pVolumeWidget->setMaximumSize(16777215,60);
    m_pInputLevelWidget->setMinimumSize(550,60);
    m_pInputLevelWidget->setMaximumSize(16777215,60);
    m_pInputSlectWidget->setMinimumSize(550,60);
    m_pInputSlectWidget->setMaximumSize(16777215,60);
    m_pNoiseReducteWidget->setMinimumSize(550,60);
    m_pNoiseReducteWidget->setMaximumSize(16777215,60);
    m_pLoopBackWidget->setMinimumSize(550,90);
    m_pLoopBackWidget->setMaximumSize(16777215,90);

    m_pInputLabel = new TitleLabel(this);
    //~ contents_path /Audio/Input
    m_pInputLabel->setText(tr("Input"));
    m_pInputLabel->setContentsMargins(16,0,16,0);
    m_pInputLabel->setStyleSheet("color: palette(windowText);}");


    //~ contents_path /Audio/Input Device
    m_pInputDeviceLabel = new FixLabel(tr("Input Device"),m_pInputSlectWidget);
    m_pInputDeviceSelectBox = new QComboBox(m_pInputSlectWidget);
    //~ contents_path /Audio/Volume
    m_pIpVolumeLabel = new FixLabel(tr("Volume"),m_pVolumeWidget);
    m_pInputIconBtn = new LingmoUIButtonDrawSvg(m_pVolumeWidget);
    m_pIpVolumeSlider = new AudioSlider();
    m_pIpVolumePercentLabel = new QLabel(m_pVolumeWidget);
    //~ contents_path /Audio/Input Level
    m_pInputLevelLabel = new FixLabel(tr("Input Level"),m_pInputLevelWidget);
    m_pInputLevelProgressBar = new QProgressBar(m_pInputLevelWidget);

    m_pInputLevelProgressBar->setStyle(new CustomStyle);
    m_pInputLevelProgressBar->setTextVisible(false);

    m_pInputIconBtn->setFocusPolicy(Qt::NoFocus);

    //~ contents_path /Audio/Noise Reduction
     m_pNoiseReducteLabel = new FixLabel(tr("Noise Reduction"));
     m_pNoiseReducteButton = new KSwitchButton();

     //~ contents_path /audio/Voice Monitor
    m_pLoopBackLabel = new FixLabel(tr("Voice Monitor"),m_pLoopBackWidget);//add 侦听功能
    m_pLoopBackTipsLabel  = new FixLabel(tr("You can hear your voice in the output device of your choice"),m_pLoopBackWidget);
    m_pLoopBackButton = new KSwitchButton();

    QPalette palette = m_pLoopBackTipsLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    m_pLoopBackTipsLabel->setPalette(palette);

    QPalette pe;
    pe.setColor(QPalette::WindowText,Qt::red);

    //输入设备添加布局
    m_pInputDeviceLabel->setFixedSize(120,40);
    m_pInputDeviceSelectBox->setFixedHeight(40);
    QHBoxLayout *inputdeviceSlectLayout = new QHBoxLayout();
    inputdeviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    inputdeviceSlectLayout->addWidget(m_pInputDeviceLabel);
    inputdeviceSlectLayout->addItem(new QSpacerItem(113,20,QSizePolicy::Fixed));
    inputdeviceSlectLayout->addWidget(m_pInputDeviceSelectBox);
    inputdeviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    inputdeviceSlectLayout->setSpacing(0);
    m_pInputSlectWidget->setLayout(inputdeviceSlectLayout);
    m_pInputSlectWidget->layout()->setContentsMargins(0,0,0,0);

    //主音量添加布局
    QHBoxLayout *m_pMasterLayout = new QHBoxLayout(m_pVolumeWidget);
    m_pIpVolumeLabel->setFixedSize(110,40);
    m_pInputIconBtn->setFixedSize(24,24);
    m_pIpVolumeSlider->setFixedHeight(55);
    m_pIpVolumePercentLabel->setFixedSize(55,20);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeLabel);
    m_pMasterLayout->addItem(new QSpacerItem(123,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pInputIconBtn);
    m_pMasterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pMasterLayout->addWidget(m_pIpVolumeSlider);
    m_pMasterLayout->addItem(new QSpacerItem(13,20,QSizePolicy::Maximum));
    m_pMasterLayout->addWidget(m_pIpVolumePercentLabel);
    m_pMasterLayout->addItem(new QSpacerItem(10,20,QSizePolicy::Maximum));
    m_pMasterLayout->setSpacing(0);
    m_pVolumeWidget->setLayout(m_pMasterLayout);
    m_pVolumeWidget->layout()->setContentsMargins(0,0,0,0);

    //输入反馈添加布局
    QHBoxLayout *m_pSoundLayout = new QHBoxLayout(m_pInputLevelWidget);
    m_pInputLevelLabel->setFixedSize(110,40);
    m_pSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelLabel);
    m_pSoundLayout->addItem(new QSpacerItem(123,20,QSizePolicy::Fixed));
    m_pSoundLayout->addWidget(m_pInputLevelProgressBar);
    m_pSoundLayout->addItem(new QSpacerItem(5,20,QSizePolicy::Fixed));
    m_pSoundLayout->setSpacing(0);
    m_pInputLevelWidget->setLayout(m_pSoundLayout);
    m_pInputLevelWidget->layout()->setContentsMargins(0,0,0,0);

    //智能降噪
    QHBoxLayout *noiseReducteLayout = new QHBoxLayout(m_pNoiseReducteWidget);
    noiseReducteLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    noiseReducteLayout->addWidget(m_pNoiseReducteLabel);
    noiseReducteLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    noiseReducteLayout->addWidget(m_pNoiseReducteButton);
    noiseReducteLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    noiseReducteLayout->setSpacing(0);
    m_pNoiseReducteWidget->setLayout(noiseReducteLayout);
    m_pNoiseReducteWidget->layout()->setContentsMargins(0,0,0,0);

    //侦听功能添加布局
    QGridLayout *gridLayout = new QGridLayout(m_pLoopBackWidget);
    gridLayout->addWidget(m_pLoopBackLabel,0,0);
    QHBoxLayout *spacer = new QHBoxLayout(m_pLoopBackWidget);
    spacer->addStretch();
    gridLayout->addLayout(spacer,0,2);
    gridLayout->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed),1,0);
    gridLayout->addWidget(m_pLoopBackTipsLabel,2,0,1,3);
    gridLayout->addWidget(m_pLoopBackButton,0,3,3,1);
    gridLayout->setSpacing(0);
    m_pLoopBackWidget->setLayout(gridLayout);
    m_pLoopBackWidget->layout()->setContentsMargins(16,15,16,15);

    m_pLoopbackLine = myLine();

    //进行整体布局
    m_pVlayout = new QVBoxLayout(m_pInputWidget);
    m_pVlayout->addWidget(m_pInputSlectWidget);
    m_pVlayout->addWidget(myLine());
    m_pVlayout->addWidget(m_pVolumeWidget);
    m_pVlayout->addWidget(myLine());
    m_pVlayout->addWidget(m_pInputLevelWidget);
    m_pVlayout->addWidget(myLine());
    m_pVlayout->addWidget(m_pNoiseReducteWidget);
    m_pVlayout->addWidget(m_pLoopbackLine);
    m_pVlayout->addWidget(m_pLoopBackWidget);
    m_pVlayout->setSpacing(0);
    m_pInputWidget->setLayout(m_pVlayout);
    m_pInputWidget->layout()->setContentsMargins(0,0,0,0);

    QVBoxLayout *m_pVlayout1 = new QVBoxLayout(this);
    m_pVlayout1->addWidget(m_pInputLabel);
    m_pVlayout1->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    m_pVlayout1->addWidget(m_pInputWidget);
    this->setLayout(m_pVlayout1);
    this->layout()->setContentsMargins(0,0,0,0);
}

void UkmediaInputWidget::setLabelAlignment(Qt::Alignment alignment)
{
      m_pIpVolumePercentLabel->setAlignment(alignment);
}

QFrame* UkmediaInputWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void UkmediaInputWidget::onPaletteChanged(){
    QPalette palette = m_pLoopBackTipsLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    m_pLoopBackTipsLabel->setPalette(palette);
}

UkmediaInputWidget::~UkmediaInputWidget()
{

}
