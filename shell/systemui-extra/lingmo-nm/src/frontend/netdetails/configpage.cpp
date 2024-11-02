/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2022 Tianjin LINGMO Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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
#include "configpage.h"

#include <QProcess>

#define VERTICAL_SPACING  24
#define KSC_FIREWALL_NOCONFIG -1
#define KSC_FIREWALL_PUBLIC  0
#define KSC_FIREWALL_PRIVATE  1

ConfigPage::ConfigPage(QWidget *parent)
{
    initUi();
    initComponent();
}

void ConfigPage::initUi()
{
    m_descriptionLabel = new QLabel(this);
    m_publicButton = new QRadioButton(this);
    m_privateButton = new QRadioButton(this);
    m_publicLabel = new QLabel(this);
    m_privateLabel = new QLabel(this);
    m_congigBtn = new KBorderlessButton(this);

    QWidget *centerWidget = new QWidget(this);
    QFormLayout *formLayout = new QFormLayout(centerWidget);
    formLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->setVerticalSpacing(VERTICAL_SPACING);
    formLayout->addRow(m_publicButton, m_publicLabel);
    formLayout->addRow(m_privateButton, m_privateLabel);

    m_vBoxLayout = new QVBoxLayout(this);
    m_vBoxLayout->setContentsMargins(0, 0, 0, 0);
    m_vBoxLayout->setSpacing(VERTICAL_SPACING);
    m_vBoxLayout->addWidget(m_descriptionLabel);
    m_vBoxLayout->addWidget(centerWidget);
    m_vBoxLayout->addWidget(m_congigBtn);
    m_vBoxLayout->addStretch();

    //网络配置文件类型
    m_descriptionLabel->setText(tr("Network profile type"));
    m_descriptionLabel->setAlignment(Qt::AlignLeft);
    //公用（推荐） 网络中的设备不可发现此电脑。一般情况下适用于公共场所中的网络，如机场或咖啡店等等。
    m_publicLabel->setText(tr("Public(recommended)  Devices on the network cannot discover this computer. Generally, "
                               "it is suitable for networks in public places, such as airports or coffee shops, etc."));
    m_publicLabel->setWordWrap(true);
    //专用 网络中的设备可发现此电脑。一般情况下适用于家庭或工作单位的网络，您认识并信任网络上的个人和设备。
    m_privateLabel->setText(tr("Private  Devices on the network can discover this computer. Generally applicable to a network "
                                "at home or work where you know and trust the individuals and devices on the network."));
    m_privateLabel->setWordWrap(true);
    //配置防火墙和安全设置
    m_congigBtn->setText(tr("Config firewall and security settings"));

    QPalette btnPal;
    QColor btnColor = palette().highlight().color();
    btnPal.setColor(QPalette::ButtonText, btnColor);
    m_congigBtn->setPalette(btnPal);
}

void ConfigPage::initComponent()
{
    connect(m_congigBtn, &KBorderlessButton::clicked, this, &ConfigPage::onConfigButtonClicked);
}

void ConfigPage::setConfigState(int type)
{
    if (type == KSC_FIREWALL_PUBLIC) {
        m_publicButton->setChecked(true);

    } else if (type == KSC_FIREWALL_PRIVATE) {
        m_privateButton->setChecked(true);
    }

}

bool ConfigPage::checkIsChanged(int type)
{
    if (type == KSC_FIREWALL_PUBLIC && m_publicButton->isChecked()) {
        return false;
    } else if (type == KSC_FIREWALL_PRIVATE && m_privateButton->isChecked()) {
        return false;
    } else {
        return true;
    }
}

int ConfigPage::getConfigState()
{
    if (m_publicButton->isChecked()) {
        return KSC_FIREWALL_PUBLIC;
    } else if (m_privateButton->isChecked()) {
        return KSC_FIREWALL_PRIVATE;
    }
    return KSC_FIREWALL_NOCONFIG;
}


void ConfigPage::onConfigButtonClicked()
{
    qDebug() << "show ksc defender net protect";

    QProcess process;
    process.startDetached("/usr/sbin/ksc-defender --net-protect");

}
