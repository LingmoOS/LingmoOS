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
#include "ukmedia_settings_widget.h"
#include <QGuiApplication>

UkmediaSettingsWidget::UkmediaSettingsWidget(QWidget *parent) : QWidget(parent)
{
    m_pAdvancedSettingsWidget = new QFrame(this);
    m_pEquipmentControlWidget = new QFrame(m_pAdvancedSettingsWidget);
    m_pAppSoundCtrlWidget = new QFrame(m_pAdvancedSettingsWidget);

    m_pAdvancedSettingsWidget->setFrameShape(QFrame::Shape::Box);
    m_pEquipmentControlWidget->setFrameShape(QFrame::Shape::Box);
    m_pAppSoundCtrlWidget->setFrameShape(QFrame::Shape::Box);

    //高级设置标签
    //~ contents_path /Audio/Advanced Settings
    m_pAdvancedSettingsLabel = new TitleLabel(this);
    m_pAdvancedSettingsLabel->setText(tr("Advanced Settings"));
    m_pAdvancedSettingsLabel->setContentsMargins(16,0,16,0);
    m_pAdvancedSettingsLabel->setStyleSheet("QLabel{color: palette(windowText);}");

    //声音设备管理标签
    //~ contents_path /Audio/Sound Equipment Control
    m_pEquipmentControlLabel = new QLabel(tr("Sound Equipment Control"), m_pEquipmentControlWidget);
    m_pDevControlDetailsBtn = new QPushButton(m_pEquipmentControlWidget);
    m_pDevControlDetailsBtn->setText(tr("Details"));
    m_pDevControlDetailsBtn->setProperty("useButtonPalette",true);

    //应用声音标签
    //~ contents_path /Audio/App Sound Control
    m_pAppSoundLabel = new QLabel(tr("App Sound Control"), m_pAppSoundCtrlWidget);
    m_pAppSoundDetailsBtn = new QPushButton(m_pAppSoundCtrlWidget);
    m_pAppSoundDetailsBtn->setText(tr("Details"));
    m_pAppSoundDetailsBtn->setProperty("useButtonPalette", true);

    //设置大小
    m_pAdvancedSettingsWidget->setMinimumSize(550,0);
    m_pAdvancedSettingsWidget->setMaximumSize(16777215,240);
    m_pEquipmentControlWidget->setMinimumSize(550,60);
    m_pEquipmentControlWidget->setMaximumSize(16777215,60);
    m_pAppSoundCtrlWidget->setMinimumSize(550,60);
    m_pAppSoundCtrlWidget->setMaximumSize(16777215,60);

    //声音设备管控
    QHBoxLayout *EquipmentControlLayout = new QHBoxLayout(m_pEquipmentControlWidget);
    EquipmentControlLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    EquipmentControlLayout->addWidget(m_pEquipmentControlLabel);
    EquipmentControlLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    EquipmentControlLayout->addWidget(m_pDevControlDetailsBtn);
    EquipmentControlLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    EquipmentControlLayout->setSpacing(0);
    m_pEquipmentControlWidget->setLayout(EquipmentControlLayout);
    m_pEquipmentControlWidget->layout()->setContentsMargins(0,0,0,0);

    //应用声音
    QHBoxLayout *AppSoundLayout = new QHBoxLayout(m_pAppSoundCtrlWidget);
    AppSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    AppSoundLayout->addWidget(m_pAppSoundLabel);
    AppSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Expanding));
    AppSoundLayout->addWidget(m_pAppSoundDetailsBtn);
    AppSoundLayout->addItem(new QSpacerItem(16,20,QSizePolicy::Fixed));
    AppSoundLayout->setSpacing(0);
    m_pAppSoundCtrlWidget->setLayout(AppSoundLayout);
    m_pAppSoundCtrlWidget->layout()->setContentsMargins(0,0,0,0);

    //设备管控和应用声音添加布局
    QVBoxLayout *m_pVlayout1 = new QVBoxLayout(m_pAdvancedSettingsWidget);
    m_pVlayout1->setContentsMargins(0,0,0,0);
    m_pVlayout1->addWidget(m_pEquipmentControlWidget);
    m_pVlayout1->addWidget(myLine());
    m_pVlayout1->addWidget(m_pAppSoundCtrlWidget);
    m_pVlayout1->setSpacing(0);
    m_pAdvancedSettingsWidget->setLayout(m_pVlayout1);
    m_pAdvancedSettingsWidget->layout()->setContentsMargins(0,0,0,0);

    //整体布局
    QVBoxLayout *vLayout1 = new QVBoxLayout(this);
    vLayout1->addWidget(m_pAdvancedSettingsLabel);
    vLayout1->addItem(new QSpacerItem(16,4,QSizePolicy::Fixed));
    vLayout1->addWidget(m_pAdvancedSettingsWidget);
    this->setLayout(vLayout1);
    this->layout()->setContentsMargins(0,0,0,0);
}

QFrame* UkmediaSettingsWidget::myLine()
{
    QFrame *line = new QFrame(this);
    line->setMinimumSize(QSize(0, 1));
    line->setMaximumSize(QSize(16777215, 1));
    line->setLineWidth(0);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    return line;
}

UkmediaSettingsWidget::~UkmediaSettingsWidget()
{

}
