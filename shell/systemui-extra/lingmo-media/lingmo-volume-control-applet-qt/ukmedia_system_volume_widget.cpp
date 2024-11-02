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
#include "ukmedia_system_volume_widget.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>

UkmediaSystemVolumeWidget::UkmediaSystemVolumeWidget(QWidget *parent) : QWidget(parent)
{
    connect(qApp, &QApplication::paletteChanged, this, &UkmediaSystemVolumeWidget::onPaletteChanged);
    m_pOutputListWidget = new QListWidget(this);
    m_pInputListWidget = new QListWidget(this);
    m_pSysSliderWidget = new QWidget(this);

    m_pOutputListWidget->setFixedSize(412,245);
    m_pOutputListWidget->setFrameShape(QFrame::Shape::NoFrame);

    QPalette pal = m_pOutputListWidget->palette();
    pal.setBrush(QPalette::Base, QColor(0,0,0,0));        //背景透明
    m_pOutputListWidget->setPalette(pal);
    m_pOutputListWidget->setProperty("needTranslucent", true);
    m_pOutputListWidget->setSelectionMode(QAbstractItemView:: NoSelection);

    QWidget *outputWidget = new QWidget(this);
    outputWidget->setFixedSize(412,245);
    QHBoxLayout *outputWidgetLayout = new QHBoxLayout;
    outputWidgetLayout->addWidget(m_pOutputListWidget);
    outputWidgetLayout->setSpacing(0);
    outputWidget->setLayout(outputWidgetLayout);
    if (UkmediaCommon::getInstance().isHWKLanguageEnv(getenv("LANGUAGE"))) {
        outputWidgetLayout->setContentsMargins(5,0,0,0);
    } else {
        outputWidgetLayout->setContentsMargins(0,0,8,0);
    }

    m_pInputListWidget->hide();
    m_pSysSliderWidget->setFixedSize(412,48);
    m_pSystemVolumeBtn = new QPushButton(m_pSysSliderWidget);
    m_pSystemVolumeBtn->setFixedSize(36,36);
    m_pSystemVolumeBtn->setCheckable(true);
    m_pSystemVolumeBtn->setProperty("isRoundButton",true);      //圆形按钮
    m_pSystemVolumeBtn->setProperty("useButtonPalette",true);   //灰色按钮
    m_pSystemVolumeBtn->setProperty("needTranslucent", true);   //灰色半透明按钮

    m_pSystemVolumeSlider = new AudioSlider(m_pSysSliderWidget);
    m_pSystemVolumeSlider->setOrientation(Qt::Horizontal);
    m_pSystemVolumeSlider->setFocusPolicy(Qt::StrongFocus);
    m_pSystemVolumeSlider->setProperty("needTranslucent", true); // Increase translucent effect
    m_pSystemVolumeSlider->setFixedSize(286,48);
    m_pSystemVolumeSlider->setRange(0,100);

    m_pSystemVolumeSliderLabel = new QLabel(m_pSysSliderWidget);
    m_pSystemVolumeSliderLabel->setText("0%");
    m_pSystemVolumeSliderLabel->setFixedSize(52,48);

    //音量label布局
    m_pSystemVolumeLabel = new QLabel(this);
    m_pSystemVolumeLabel->setText(tr("Volume"));
    m_pSystemVolumeLabel->setFixedSize(372,29);//防止字体显示不全
    QWidget *systemVolumeLabelWidget = new QWidget(this);
    systemVolumeLabelWidget->setFixedSize(412,29);
    QHBoxLayout *systemVolumeLabelLayout = new QHBoxLayout;
    systemVolumeLabelLayout->addWidget(m_pSystemVolumeLabel);
    systemVolumeLabelWidget->setLayout(systemVolumeLabelLayout);
    systemVolumeLabelLayout->setContentsMargins(16,0,24,0);

    //输出label布局
    m_pOutputLabel = new QLabel(this);
    m_pOutputLabel->setText(tr("Output"));
    m_pOutputLabel->setFixedSize(372,29);
    QWidget *outputLabelWidget = new QWidget(this);
    outputLabelWidget->setFixedSize(412,29);
    QHBoxLayout *outputLabelLayout = new QHBoxLayout;
    outputLabelLayout->addWidget(m_pOutputLabel);
    outputLabelWidget->setLayout(outputLabelLayout);
    outputLabelLayout->setContentsMargins(16,0,24,0);

    QPalette palette = m_pSystemVolumeLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    m_pSystemVolumeLabel->setPalette(palette);
    m_pOutputLabel->setPalette(palette);

    volumeSettingButton = new LingmoUISettingButton(volumeSettingFrame);
    volumeSettingButton->setText(tr("Sound Settings"));
    volumeSettingButton->setCursor(Qt::PointingHandCursor);
    volumeSettingButton->setScaledContents(true);

    volumeSettingFrame = new QFrame(this);
    volumeSettingFrame->setFixedHeight(52);
    QHBoxLayout *volumeSettingLayout = new QHBoxLayout;
    volumeSettingLayout->addWidget(volumeSettingButton);
    volumeSettingLayout->addStretch();
    volumeSettingFrame->setLayout(volumeSettingLayout);
    volumeSettingLayout->setContentsMargins(16,0,24,0);

    //系统音量滑动条布局
    QHBoxLayout *sysVolumeLay = new QHBoxLayout();
    sysVolumeLay->addWidget(m_pSystemVolumeBtn);
    sysVolumeLay->addSpacing(3);
    sysVolumeLay->addWidget(m_pSystemVolumeSlider);
    sysVolumeLay->addWidget(m_pSystemVolumeSliderLabel);
    sysVolumeLay->setSpacing(0);
    m_pSysSliderWidget->setLayout(sysVolumeLay);
    sysVolumeLay->setContentsMargins(11,0,24,0);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(systemVolumeLabelWidget);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(m_pSysSliderWidget);
    mainLayout->addSpacing(13);
    mainLayout->addWidget(outputLabelWidget);
    mainLayout->addSpacing(3);
    mainLayout->addWidget(outputWidget);
    mainLayout->addSpacing(2);
    mainLayout->addWidget(volumeSettingFrame);
    mainLayout->setSpacing(0);
    this->setLayout(mainLayout);
    mainLayout->setContentsMargins(8,12,0,0);
    this->setFixedSize(420,436);
}

void UkmediaSystemVolumeWidget::setLabelAlignment(Qt::Alignment alignment)
{
    m_pSystemVolumeSliderLabel->setAlignment(alignment);
}

void UkmediaSystemVolumeWidget::onPaletteChanged(){
    QPalette palette = m_pSystemVolumeLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::WindowText,color);
    m_pSystemVolumeLabel->setPalette(palette);
    m_pOutputLabel->setPalette(palette);
}

void UkmediaSystemVolumeWidget::setVolumeSliderRange(bool status)
{
    int maxValue = (status) ? 125 : 100;
    m_pSystemVolumeSlider->blockSignals(true);
    m_pSystemVolumeSlider->setRange(0, maxValue);
    m_pSystemVolumeSlider->blockSignals(false);
}
