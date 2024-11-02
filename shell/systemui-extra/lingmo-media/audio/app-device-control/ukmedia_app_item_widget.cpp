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
#include "ukmedia_app_item_widget.h"

UkmediaAppItemWidget::UkmediaAppItemWidget(QWidget *parent)
    : QWidget(parent)
{
    if (QGSettings::isSchemaInstalled("org.lingmo.style")) {
        QGSettings *nThemeSetting = new QGSettings("org.lingmo.style");
        if (nThemeSetting->keys().contains("styleName")) {
            mThemeName = nThemeSetting->get("style-name").toString();
        }
    }

    initUI();

    onPaletteChanged();
}

void UkmediaAppItemWidget::initUI()
{
    this->setFixedSize(560,477);
    m_pAppLabel = new FixLabel(tr("Application"), this);
    m_pAppLabel->setContentsMargins(17,0,0,0);

    m_pAppWidget = new QFrame(this);
    m_pInputWidget = new QFrame(m_pAppWidget);
    m_pOutputWidget = new QFrame(m_pAppWidget);
    m_pVolumeWidget = new QFrame(m_pAppWidget);
    m_pCheckWidget = new QFrame(m_pAppWidget);

    m_pAppWidget->setFrameShape(QFrame::Shape::Box);
    m_pInputWidget->setFrameShape(QFrame::Shape::Box);
    m_pOutputWidget->setFrameShape(QFrame::Shape::Box);
    m_pVolumeWidget->setFrameShape(QFrame::Shape::Box);
    m_pCheckWidget->setFrameShape(QFrame::Shape::Box);

    m_pInputWidget->setObjectName("InputWidget");
    m_pOutputWidget->setObjectName("OutputWidget");
    m_pVolumeWidget->setObjectName("VolumeWidget");

    m_pAppWidget->setFixedSize(512, 415);
    m_pInputWidget->setFixedSize(512, 90);
    m_pOutputWidget->setFixedSize(512, 90);
    m_pVolumeWidget->setFixedSize(512, 60);
    m_pCheckWidget->setFixedSize(512, 36);

    //应用音量布局
    m_pVolumeLabel = new FixLabel(tr("Output Volume"), m_pVolumeWidget);
    m_pVolumeBtn = new LingmoUIButtonDrawSvg(m_pVolumeWidget);
    m_pVolumeSlider = new KSlider(m_pVolumeWidget);
    m_pVolumeSlider->setRange(0,100);
    m_pVolumeSlider->setSliderType(SmoothSlider);
    m_pVolumeSlider->setNodeVisible(false);
    m_pVolumeNumLabel = new FixLabel("0%", m_pVolumeWidget);

    m_pVolumeBtn->setFixedSize(24,24);
    m_pVolumeSlider->setFixedSize(260,55);
    m_pVolumeLabel->setFixedSize(120,20);
    m_pVolumeNumLabel->setFixedSize(55,20);
    m_pVolumeSlider->setOrientation(Qt::Horizontal);
    m_pVolumeNumLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    QHBoxLayout *volumeLayout = new QHBoxLayout(m_pVolumeWidget);
    volumeLayout->addItem(new QSpacerItem(17,20,QSizePolicy::Fixed));
    volumeLayout->addWidget(m_pVolumeLabel);
    volumeLayout->addItem(new QSpacerItem(12,20,QSizePolicy::Expanding));
    volumeLayout->addWidget(m_pVolumeBtn);
    volumeLayout->addItem(new QSpacerItem(10,20,QSizePolicy::Fixed));
    volumeLayout->addWidget(m_pVolumeSlider);
//    volumeLayout->addItem(new QSpacerItem(14,20,QSizePolicy::Fixed));
    volumeLayout->addWidget(m_pVolumeNumLabel);
    volumeLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    volumeLayout->setSpacing(0);
    m_pVolumeWidget->setLayout(volumeLayout);
    m_pVolumeWidget->layout()->setContentsMargins(0,0,0,0);

    //输入设备布局
    m_pInputDevLabel = new FixLabel(tr("Input Device"), m_pInputWidget);
    m_pInputCombobox = new QComboBox(m_pInputWidget);
    m_pInputDevLabel->setFixedSize(120, 36);
    m_pInputCombobox->setFixedSize(347, 36);
    m_pInputHintIcon = new QLabel(m_pInputWidget);
    m_pInputHintIcon->setFixedSize(16, 16);
    m_pInputHintIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16, 16)));
    m_pInputHintLabel = new FixLabel(tr("This case does not support setting the input device"), m_pInputWidget);

    m_pInputHintWidget = new QFrame(m_pInputWidget);
    QHBoxLayout *lay = new QHBoxLayout(m_pInputHintWidget);
    lay->addWidget(m_pInputHintIcon);
    lay->addWidget(m_pInputHintLabel);
    lay->setSpacing(3);
    m_pInputHintWidget->setLayout(lay);
    m_pInputHintWidget->layout()->setContentsMargins(0, 0, 0, 0);

    QGridLayout *gridLayout = new QGridLayout(m_pInputWidget);
    gridLayout->addWidget(m_pInputDevLabel, 0, 0);
    gridLayout->addWidget(m_pInputCombobox, 0, 1);
    gridLayout->addItem(new QSpacerItem(132, 20, QSizePolicy::Fixed), 1, 0);
    gridLayout->addWidget(m_pInputHintWidget);
    gridLayout->setSpacing(0);
    m_pInputWidget->setLayout(gridLayout);
    m_pInputWidget->layout()->setContentsMargins(17, 11, 16, 0);

    //输出设备布局
    m_pOutputDevLabel = new FixLabel(tr("Output Device"), m_pOutputWidget);
    m_pOutputCombobox = new QComboBox(m_pOutputWidget);
    m_pOutputDevLabel->setFixedSize(120, 36);
    m_pOutputCombobox->setFixedSize(347, 36);
    m_pOutputHintIcon = new QLabel(m_pOutputWidget);
    m_pOutputHintIcon->setFixedSize(16, 16);
    m_pOutputHintIcon->setPixmap(QIcon::fromTheme("dialog-warning").pixmap(QSize(16, 16)));
    m_pOutputHintLabel = new FixLabel(tr("This case does not support setting the output device"), m_pOutputWidget);

    m_pOutputHintWidget = new QFrame(m_pOutputWidget);
    QHBoxLayout *outputLay = new QHBoxLayout(m_pOutputHintWidget);
    outputLay->addWidget(m_pOutputHintIcon);
    outputLay->addWidget(m_pOutputHintLabel);
    outputLay->setSpacing(3);
    m_pOutputHintWidget->setLayout(outputLay);
    m_pOutputHintWidget->layout()->setContentsMargins(0, 0, 0, 0);

    QGridLayout *outputGridLayout = new QGridLayout(m_pOutputWidget);
    outputGridLayout->addWidget(m_pOutputDevLabel, 0, 0);
    outputGridLayout->addWidget(m_pOutputCombobox, 0, 1);
    outputGridLayout->addItem(new QSpacerItem(132, 20, QSizePolicy::Fixed), 1, 0);
    outputGridLayout->addWidget(m_pOutputHintWidget);
    outputGridLayout->setSpacing(0);
    m_pOutputWidget->setLayout(outputGridLayout);
    m_pOutputWidget->layout()->setContentsMargins(17, 11, 16, 0);

    // check Btn
    m_pConfirmBtn = new QPushButton(tr("Confirm"), m_pCheckWidget);
    m_pConfirmBtn->setFixedSize(96, 36);
    m_pConfirmBtn->setProperty("isImportant", true);

    QHBoxLayout *checkLayout = new QHBoxLayout(m_pCheckWidget);
    checkLayout->addStretch();
    checkLayout->addWidget(m_pConfirmBtn);
    checkLayout->setSpacing(0);
    m_pCheckWidget->setLayout(checkLayout);
    m_pCheckWidget->layout()->setContentsMargins(0,0,0,0);

//    line1 = myLine();
//    line2 = myLine();

    QVBoxLayout *vlayout = new QVBoxLayout(m_pAppWidget);
    vlayout->addWidget(m_pVolumeWidget);
    vlayout->addSpacing(1);
    vlayout->addWidget(m_pOutputWidget);
    vlayout->addSpacing(1);
    vlayout->addWidget(m_pInputWidget);
    vlayout->addItem(new QSpacerItem(16,197,QSizePolicy::Fixed));
    vlayout->addStretch();
    vlayout->addWidget(m_pCheckWidget);
    vlayout->setSpacing(0);
    vlayout->setContentsMargins(0,0,0,0);

    //整体布局
    QVBoxLayout *appLayout = new QVBoxLayout(this);
    appLayout->addItem(new QSpacerItem(16,8,QSizePolicy::Fixed));
    appLayout->addWidget(m_pAppLabel);
    appLayout->addItem(new QSpacerItem(16,8,QSizePolicy::Fixed));
    appLayout->addWidget(m_pAppWidget);
    appLayout->addItem(new QSpacerItem(16,24,QSizePolicy::Fixed));
    appLayout->addStretch();

    appLayout->setSpacing(0);
    this->setLayout(appLayout);
    this->layout()->setContentsMargins(25,0,23,0);
}

void UkmediaAppItemWidget::setInputHintWidgetShow(bool status)
{
    if (status) {
        m_pInputHintWidget->hide();
        m_pInputWidget->setFixedSize(512, 60);
        m_pInputCombobox->setDisabled(false);
    } else {
        m_pInputHintWidget->show();
        m_pInputWidget->setFixedSize(512, 90);
        m_pInputCombobox->setDisabled(true);
    }
}

void UkmediaAppItemWidget::setOutputHintWidgetShow(bool status)
{
    if (status) {
        m_pOutputHintWidget->hide();
        m_pOutputWidget->setFixedSize(512, 60);
        m_pOutputCombobox->setDisabled(false);
    } else {
        m_pOutputHintWidget->show();
        m_pOutputWidget->setFixedSize(512, 90);
        m_pOutputCombobox->setDisabled(true);
    }
}

//  设置窗口应用标题
void UkmediaAppItemWidget::setTitleName(QString name)
{
    m_pAppLabel->setText(name);
}

void UkmediaAppItemWidget::setChildObjectName(QString name)
{
    m_pVolumeNumLabel->setObjectName(name+"-label");
    m_pVolumeBtn->setObjectName(name);
    m_pVolumeSlider->setObjectName(name);
    m_pInputCombobox->setObjectName(name+"-input");
    m_pOutputCombobox->setObjectName(name+"-output");
    this->setObjectName(name);
}

void UkmediaAppItemWidget::setSliderValue(int value)
{
    if (m_pVolumeSlider->objectName() == "lingmo-settings-system") {
        if (QGSettings::isSchemaInstalled("org.lingmo.sound")) {
            QGSettings *nThemeSetting = new QGSettings("org.lingmo.sound");
            if (nThemeSetting->keys().contains("volumeIncrease")) {
                if (nThemeSetting->get("volumeIncrease").toBool())
                    m_pVolumeSlider->setRange(0, 125);
            }
        }
    }

    m_pVolumeSlider->blockSignals(true);
    m_pVolumeSlider->setValue(value);
    m_pVolumeSlider->blockSignals(false);

    QString percent = QString::number(value);
    m_pVolumeNumLabel->setText(percent+"%");
}

void UkmediaAppItemWidget::addInputCombobox(QString port)
{
    m_pInputCombobox->blockSignals(true);
    m_pInputCombobox->addItem(port);
    m_pInputCombobox->blockSignals(false);
}

void UkmediaAppItemWidget::addOutputCombobox(QString port)
{
    m_pOutputCombobox->blockSignals(true);
    m_pOutputCombobox->addItem(port);
    m_pOutputCombobox->blockSignals(false);
}

void UkmediaAppItemWidget::onPaletteChanged()
{
    QPalette palette = m_pInputHintLabel->palette();
    QColor color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::Text, color);
    m_pInputHintLabel->setPalette(palette);

    palette = m_pOutputHintLabel->palette();
    color = palette.color(QPalette::PlaceholderText);
    palette.setColor(QPalette::Text, color);
    m_pOutputHintLabel->setPalette(palette);

    if (mThemeName == LINGMO_THEME_LIGHT || mThemeName == LINGMO_THEME_DEFAULT) {
        m_pInputWidget->setStyleSheet("#InputWidget{border-radius: 6px; background-color: #F5F5F5;}");
        m_pOutputWidget->setStyleSheet("#OutputWidget{border-radius: 6px; background-color: #F5F5F5;}");
        m_pVolumeWidget->setStyleSheet("#VolumeWidget{border-radius: 6px; background-color: #F5F5F5;}");
    }
    else if (mThemeName == LINGMO_THEME_DARK) {
        m_pInputWidget->setStyleSheet("#InputWidget{border-radius: 6px; background-color: #333333;}");
        m_pOutputWidget->setStyleSheet("#OutputWidget{border-radius: 6px; background-color: #333333;}");
        m_pVolumeWidget->setStyleSheet("#VolumeWidget{border-radius: 6px; background-color: #333333;}");
    }
}

//  更新音量图标
void UkmediaAppItemWidget::outputVolumeDarkThemeImage(int value, bool status)
{
    QImage image;

    QColor color = QColor(0,0,0,216);
    if (mThemeName == LINGMO_THEME_LIGHT || mThemeName == LINGMO_THEME_DEFAULT)
        color = QColor(0,0,0,216);
    else if (mThemeName == LINGMO_THEME_DARK)
        color = QColor(255,255,255,216);

    if (status) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value <= 0) {
        image = QIcon::fromTheme("audio-volume-muted-symbolic").pixmap(24,24).toImage();
    }
    else if (value > 0 && value <= 33) {
        image = QIcon::fromTheme("audio-volume-low-symbolic").pixmap(24,24).toImage();
    }
    else if (value >33 && value <= 66) {
        image = QIcon::fromTheme("audio-volume-medium-symbolic").pixmap(24,24).toImage();
    }
    else {
        image = QIcon::fromTheme("audio-volume-high-symbolic").pixmap(24,24).toImage();
    }

    m_pVolumeBtn->refreshIcon(image, color);
}

QFrame* UkmediaAppItemWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setFixedSize(512, 1);
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    return line;
}

UkmediaAppItemWidget::~UkmediaAppItemWidget()
{

}
