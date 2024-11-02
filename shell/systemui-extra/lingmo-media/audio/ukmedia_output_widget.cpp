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
#include "ukmedia_output_widget.h"
#include "lingmo_list_widget_item.h"
#include <QHBoxLayout>
#include <QDebug>

UkmediaOutputWidget::UkmediaOutputWidget(QWidget *parent) : QWidget(parent)
{
    connect(qApp, &QApplication::paletteChanged, this, &UkmediaOutputWidget::onPaletteChanged);
    //加载qss样式文件
    QFile QssFile("://combox.qss");
    QssFile.open(QFile::ReadOnly);

    if (QssFile.isOpen()){
        sliderQss = QLatin1String(QssFile.readAll());
        QssFile.close();
    } else {
        qDebug()<<"combox.qss is not found"<<endl;
    }

    m_pOutputWidget         = new QFrame(this);
    m_pMasterVolumeWidget   = new QFrame(m_pOutputWidget);
    m_pOutputSlectWidget    = new QFrame(m_pOutputWidget);
    m_pVolumeIncreaseWidget = new QFrame(m_pOutputWidget);

    m_pAudioBlanceWidget = new QFrame(this);                        //MonoAudio
    m_pChannelBalanceWidget = new QFrame(m_pAudioBlanceWidget);
    m_pMonoAudioWidget = new QFrame(m_pAudioBlanceWidget);

    m_pOutputWidget->setFrameShape(QFrame::Shape::Box);
    m_pMasterVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pOutputSlectWidget->setFrameShape(QFrame::Shape::Box);
    m_pVolumeIncreaseWidget->setFrameShape(QFrame::Shape::Box);

    m_pAudioBlanceWidget->setFrameShape(QFrame::Shape::Box);        //MonoAudio
    m_pChannelBalanceWidget->setFrameShape(QFrame::Shape::Box);
    m_pMonoAudioWidget->setFrameShape(QFrame::Shape::Box);

    //设置大小
    m_pOutputWidget->setMinimumSize(550,0);
    m_pOutputWidget->setMaximumSize(16777215,360);
    m_pMasterVolumeWidget->setMinimumSize(550,60);
    m_pMasterVolumeWidget->setMaximumSize(16777215,60);
    m_pOutputSlectWidget->setMinimumSize(550,60);
    m_pOutputSlectWidget->setMaximumSize(16777215,60);
    m_pVolumeIncreaseWidget->setMinimumSize(550,90);
    m_pVolumeIncreaseWidget->setMaximumSize(16777215,90);

    m_pChannelBalanceWidget->setMinimumSize(550,60);                  //MonoAudio
    m_pChannelBalanceWidget->setMaximumSize(16777215,60);
    m_pMonoAudioWidget->setMinimumSize(550,90);
    m_pMonoAudioWidget->setMaximumSize(16777215,90);

    m_pOutputLabel = new TitleLabel(this);
    //~ contents_path /Audio/Output
    m_pOutputLabel->setText(tr("Output"));
    m_pOutputLabel->setContentsMargins(16,0,16,0);
    m_pOutputLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    //~ contents_path /Audio/Output Device
    m_pOutputDeviceLabel = new FixLabel(tr("Output Device"),m_pOutputSlectWidget);
    m_pDeviceSelectBox = new QComboBox(m_pOutputSlectWidget);
    //~ contents_path /Audio/Master Volume
    m_pOpVolumeLabel = new FixLabel(tr("Master Volume"),m_pMasterVolumeWidget);
    m_pOutputIconBtn = new LingmoUIButtonDrawSvg(m_pMasterVolumeWidget);
    m_pOpVolumeSlider = new AudioSlider(m_pMasterVolumeWidget);
    m_pOpVolumePercentLabel = new QLabel(m_pMasterVolumeWidget);
    //~ contents_path /Audio/Balance
    m_pOpBalanceLabel = new FixLabel(tr("Balance"),m_pChannelBalanceWidget);
    m_pLeftBalanceLabel = new FixLabel(tr("Left"),m_pChannelBalanceWidget);
    m_pOpBalanceSlider = new UkmediaVolumeSlider(m_pChannelBalanceWidget);
    m_pRightBalanceLabel = new FixLabel(tr("Right"),m_pChannelBalanceWidget);

    //~ contents_path /Audio/Volume Increase
    VolumeIncreaseTipsWidget = new QWidget(m_pVolumeIncreaseWidget);
    m_pVolumeIncreaseLabel   = new FixLabel(tr("Volume Increase"),VolumeIncreaseTipsWidget);
    VolumeIncreaseTipsLabel  = new FixLabel(tr("Volume above 100% can cause sound distortion and damage your speakers."),VolumeIncreaseTipsWidget);
    m_pVolumeIncreaseButton  = new KSwitchButton();

    QPalette palette = VolumeIncreaseTipsLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    VolumeIncreaseTipsLabel->setPalette(palette);

    QVBoxLayout *vlay = new QVBoxLayout(this);
    vlay->addStretch();
    vlay->addWidget(m_pVolumeIncreaseLabel);
    vlay->addWidget(VolumeIncreaseTipsLabel);
    vlay->addStretch();
    vlay->setSpacing(0);

    VolumeIncreaseTipsWidget->setLayout(vlay);
    VolumeIncreaseTipsWidget->layout()->setContentsMargins(0,0,0,0);

    m_pOutputIconBtn->setFocusPolicy(Qt::NoFocus);

    //~ contents_path /Audio/Mono Audio
    MonoAudioTipsWidget = new QWidget(m_pMonoAudioWidget);
    m_pMonoAudioLabel   = new FixLabel(tr("Mono Audio"),MonoAudioTipsWidget);
    MonoAudioTipsLabel  = new FixLabel(tr("It merges the left and right channels into one channel."),MonoAudioTipsWidget);
    m_pMonoAudioButton  = new KSwitchButton();

    QPalette palette1 = MonoAudioTipsLabel->palette();
    QColor color1 = palette1.color(QPalette::PlaceholderText);
    palette1.setColor(QPalette::WindowText,color1);
    MonoAudioTipsLabel->setPalette(palette1);

    QVBoxLayout *vlay1 = new QVBoxLayout(this);
    vlay1->addStretch();
    vlay1->addWidget(m_pMonoAudioLabel);
    vlay1->addWidget(MonoAudioTipsLabel);
    vlay1->addStretch();
    vlay1->setSpacing(0);
    MonoAudioTipsWidget->setLayout(vlay1);
    MonoAudioTipsWidget->layout()->setContentsMargins(0,0,0,0);


    //选择输出设备框添加布局
    m_pOutputDeviceLabel->setFixedSize(140,20);
    m_pDeviceSelectBox->setFixedHeight(40);
    QHBoxLayout *deviceSlectLayout = new QHBoxLayout();
    deviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    deviceSlectLayout->addWidget(m_pOutputDeviceLabel);
    deviceSlectLayout->addItem(new QSpacerItem(93,20,QSizePolicy::Fixed));
    deviceSlectLayout->addWidget(m_pDeviceSelectBox);
    deviceSlectLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    deviceSlectLayout->setSpacing(0);
    m_pOutputSlectWidget->setLayout(deviceSlectLayout);
    m_pOutputSlectWidget->layout()->setContentsMargins(0,0,0,0);

    //主音量添加布局
    QHBoxLayout *masterLayout = new QHBoxLayout(m_pMasterVolumeWidget);
    m_pOpVolumeLabel->setFixedSize(110,40);
    m_pOutputIconBtn->setFixedSize(24,24);
    m_pOpVolumeSlider->setFixedHeight(55);
    m_pOpVolumePercentLabel->setFixedSize(55,20);

    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumeLabel);
    masterLayout->addItem(new QSpacerItem(123,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOutputIconBtn);
    masterLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumeSlider);
    masterLayout->addItem(new QSpacerItem(13,20,QSizePolicy::Fixed));
    masterLayout->addWidget(m_pOpVolumePercentLabel);
    masterLayout->addItem(new QSpacerItem(10,20,QSizePolicy::Fixed));
    masterLayout->setSpacing(0);
    m_pMasterVolumeWidget->setLayout(masterLayout);
    m_pMasterVolumeWidget->layout()->setContentsMargins(0,0,0,0);

    //声道平衡添加布局
    QHBoxLayout *soundLayout = new QHBoxLayout(m_pChannelBalanceWidget);
    m_pOpBalanceLabel->setFixedSize(110,20);
    m_pLeftBalanceLabel->setFixedSize(40,30);
    m_pOpBalanceSlider->setFixedHeight(55);
    m_pRightBalanceLabel->setFixedSize(55,30);

    soundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pOpBalanceLabel);
    soundLayout->addItem(new QSpacerItem(123,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pLeftBalanceLabel);
//    soundLayout->addItem(new QSpacerItem(26,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pOpBalanceSlider);
    soundLayout->addItem(new QSpacerItem(14,20,QSizePolicy::Fixed));
    soundLayout->addWidget(m_pRightBalanceLabel);
    soundLayout->addItem(new QSpacerItem(10,20,QSizePolicy::Fixed));
    soundLayout->setSpacing(0);
    m_pChannelBalanceWidget->setLayout(soundLayout);
    m_pChannelBalanceWidget->layout()->setContentsMargins(0,0,0,0);

    //音量增强
    QHBoxLayout *volumeIncreaseLayout = new QHBoxLayout(m_pVolumeIncreaseWidget);
    volumeIncreaseLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeIncreaseLayout->addWidget(VolumeIncreaseTipsWidget);
    volumeIncreaseLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    volumeIncreaseLayout->addWidget(m_pVolumeIncreaseButton);
    volumeIncreaseLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeIncreaseLayout->setSpacing(0);
    m_pVolumeIncreaseWidget->setLayout(volumeIncreaseLayout);
    m_pVolumeIncreaseWidget->layout()->setContentsMargins(0,0,0,0);

//    QFrame *outputselectAndVolume = myLine();
//    QFrame *volumeAndBalance = myLine();

    //单声道音频
    QHBoxLayout *monoAudioLayout = new QHBoxLayout(m_pMonoAudioWidget);
    monoAudioLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    monoAudioLayout->addWidget(MonoAudioTipsWidget);
    monoAudioLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    monoAudioLayout->addWidget(m_pMonoAudioButton);
    monoAudioLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    monoAudioLayout->setSpacing(0);
    m_pMonoAudioWidget->setLayout(monoAudioLayout);
    m_pMonoAudioWidget->layout()->setContentsMargins(0,0,0,0);


    //进行整体布局
    m_pVlayout = new QVBoxLayout(m_pOutputWidget);
    m_pVlayout->setContentsMargins(0,0,0,0);
    m_pVlayout->addWidget(m_pOutputSlectWidget);
    m_pVlayout->addWidget(myLine());
    m_pVlayout->addWidget(m_pMasterVolumeWidget);
    m_pVlayout->addWidget(myLine());
    m_pVlayout->addWidget(m_pVolumeIncreaseWidget);
    m_pVlayout->setSpacing(0);
//    m_pOutputWidget->setLayout(m_pVlayout);
//    m_pOutputWidget->layout()->setContentsMargins(0,0,0,0);

    m_pMonoLine = myLine();

    //进行monoAudio布局
    QVBoxLayout *m_pVlayout1 = new QVBoxLayout(m_pAudioBlanceWidget);
    m_pVlayout1->setContentsMargins(0,0,0,0);
    m_pVlayout1->addWidget(m_pChannelBalanceWidget);
    m_pVlayout1->addWidget(m_pMonoLine);
    m_pVlayout1->addWidget(m_pMonoAudioWidget);
    m_pVlayout1->setSpacing(0);

    QVBoxLayout *vLayout1 = new QVBoxLayout(this);
    vLayout1->addWidget(m_pOutputLabel);
    vLayout1->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    vLayout1->addWidget(m_pOutputWidget);
    vLayout1->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    vLayout1->addWidget(m_pAudioBlanceWidget);

    this->setLayout(vLayout1);
    this->layout()->setContentsMargins(0,0,0,0);

    m_pMasterVolumeWidget->setObjectName("masterVolumeWidget");
    //设置样式
    m_pOutputLabel->setObjectName("m_pOutputLabel");
}

void UkmediaOutputWidget::setVolumeSliderRange(bool status)
{
    int maxValue = (status) ? 125 : 100;
    m_pOpVolumeSlider->blockSignals(true);
    m_pOpVolumeSlider->setRange(0, maxValue);
    m_pOpVolumeSlider->blockSignals(false);
}

void UkmediaOutputWidget::setLabelAlignment(Qt::Alignment alignment)
{
    m_pOpVolumePercentLabel->setAlignment(alignment);
    m_pRightBalanceLabel->setAlignment(alignment);
}

QFrame* UkmediaOutputWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

void UkmediaOutputWidget::onPaletteChanged(){
    QPalette palette = VolumeIncreaseTipsLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    VolumeIncreaseTipsLabel->setPalette(palette);

    palette = MonoAudioTipsLabel->palette();
    color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    MonoAudioTipsLabel->setPalette(palette);
}

UkmediaOutputWidget::~UkmediaOutputWidget()
{

}
