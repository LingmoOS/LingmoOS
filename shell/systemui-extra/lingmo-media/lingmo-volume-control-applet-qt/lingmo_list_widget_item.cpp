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
#include "lingmo_list_widget_item.h"

bool isCheckBluetoothInput;

LingmoUIListWidgetItem::LingmoUIListWidgetItem(QWidget *parent) :
    QWidget(parent)
{
    this->setFixedSize(404,48);

    hLayout      = new QHBoxLayout(this);
    portLabel    = new FixLabel(this);
    deviceLabel  = new QLabel(this);
    deviceButton = new QPushButton(this);

    portLabel->setFixedSize(326,24);
    deviceLabel->setFixedSize(326,24);
    deviceLabel->hide(); // 记录cardname

    deviceButton->setFixedSize(36,36);
    deviceButton->setCheckable(true);
    deviceButton->setProperty("isRoundButton", true);
    deviceButton->setProperty("useButtonPalette", true);
    deviceButton->setProperty("needTranslucent", true);

    hLayout->addWidget(deviceButton);
    hLayout->addSpacing(12);
    hLayout->addWidget(portLabel);
    hLayout->setSpacing(0);
    this->setLayout(hLayout);
    hLayout->setContentsMargins(14,6,24,6);

    this->show();
}

void LingmoUIListWidgetItem::setButtonIcon(QString iconName)
{
    QString iconStr;

    if (iconName.contains("headphone", Qt::CaseInsensitive))
        iconStr = "audio-headphones-symbolic";
    else if (iconName.contains("headset", Qt::CaseInsensitive))
        iconStr = "audio-headset-symbolic";
    else
        iconStr = "audio-volume-high-symbolic";

    deviceButton->setIcon(QIcon::fromTheme(iconStr));
}

void LingmoUIListWidgetItem::setLabelText(QString portLabel, QString deviceLabel)
{
    this->portLabel->setText(portLabel,true);
    this->deviceLabel->setText(deviceLabel);
}

void LingmoUIListWidgetItem::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    qDebug() << "Mouse Press Event" << this->portLabel->fullText << this->deviceLabel->text();
    //蓝牙输入去除勾选
    if (this->deviceLabel->text().contains("bluez_card")) {
        if (isCheckBluetoothInput == false)
            isCheckBluetoothInput = true;
        else  {
            isCheckBluetoothInput = false;
            QString cmd = "pactl set-card-profile "+this->deviceLabel->text()+" a2dp_sink";
            system(cmd.toLocal8Bit().data());
        }
    }
}

LingmoUIListWidgetItem::~LingmoUIListWidgetItem()
{
}
