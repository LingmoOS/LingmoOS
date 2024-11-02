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
#include "ipv4page.h"
#include "netdetail.h"
#include "math.h"

#define  LAYOUT_MARGINS 0,0,0,0
#define  LAYOUT_SPACING 0
#define  HINT_TEXT_MARGINS 8, 1, 0, 3
#define  LABEL_HEIGHT 24
#define FRAME_SPEED 150
#define ICON_SIZE 16,16

Ipv4Page::Ipv4Page(QWidget *parent):QFrame(parent)
{
    initUI();
    initComponent();
}

void Ipv4Page::initUI() {
    ipv4ConfigCombox = new QComboBox(this);
    ipv4addressEdit = new LineEdit(this);
    netMaskEdit = new LineEdit(this);
    gateWayEdit = new LineEdit(this);

    m_configLabel = new QLabel(this);
    m_addressLabel = new QLabel(this);
    m_maskLabel = new QLabel(this);
    m_gateWayLabel = new QLabel(this);

    m_configEmptyLabel = new QLabel(this);
    m_configEmptyLabel->setFixedHeight(LABEL_HEIGHT);

    m_addressHintLabel = new QLabel(this);
    m_addressHintLabel->setFixedHeight(LABEL_HEIGHT);
    m_addressHintLabel->setContentsMargins(HINT_TEXT_MARGINS);
    initConflictHintLable();

    m_maskHintLabel = new QLabel(this);
    m_maskHintLabel->setFixedHeight(LABEL_HEIGHT);
    m_maskHintLabel->setContentsMargins(HINT_TEXT_MARGINS);

    m_gateWayEmptyLabel = new QLabel(this);
    m_gateWayEmptyLabel->setFixedHeight(LABEL_HEIGHT);

    m_dnsEmptyLabel = new QLabel(this);
    m_dnsEmptyLabel->setFixedHeight(21);

    m_configLabel->setText(tr("IPv4Config"));
    m_addressLabel->setText(tr("Address"));
    m_maskLabel->setText(tr("Netmask"));
    m_gateWayLabel->setText(tr("Default Gateway"));

    m_statusLabel = new QLabel(this);
    m_statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QHBoxLayout *pPwdLayout = new QHBoxLayout(ipv4addressEdit);
    pPwdLayout->addStretch();
    pPwdLayout->addWidget(m_statusLabel);

    QPalette hintTextColor;
    hintTextColor.setColor(QPalette::WindowText, Qt::red);
    m_addressHintLabel->setPalette(hintTextColor);
    m_maskHintLabel->setPalette(hintTextColor);

    QWidget *addressWidget = new QWidget(this);
    QVBoxLayout *addressLayout = new QVBoxLayout(addressWidget);
    addressLayout->setContentsMargins(LAYOUT_MARGINS);
    addressLayout->setSpacing(LAYOUT_SPACING);
    addressLayout->addWidget(ipv4addressEdit);
    addressLayout->addWidget(m_addressHintLabel);

    QWidget *maskWidget = new QWidget(this);
    QVBoxLayout *maskLayout = new QVBoxLayout(maskWidget);
    maskLayout->setContentsMargins(LAYOUT_MARGINS);
    maskLayout->setSpacing(LAYOUT_SPACING);
    maskLayout->addWidget(netMaskEdit);
    maskLayout->addWidget(m_maskHintLabel);

    // IP的正则格式限制
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    Divider* divider = new Divider(false, this);
    m_dnsWidget = new MultipleDnsWidget(rx, false, this);

    m_detailLayout = new QFormLayout(this);
    m_detailLayout->setVerticalSpacing(0);
    m_detailLayout->setContentsMargins(LAYOUT_MARGINS);
    m_detailLayout->addRow(m_configLabel,ipv4ConfigCombox);
    m_detailLayout->addRow(m_configEmptyLabel);
    m_detailLayout->addRow(m_addressLabel,addressWidget);
    m_detailLayout->addRow(m_maskLabel,maskWidget);
    m_detailLayout->addRow(m_gateWayLabel,gateWayEdit);
    m_detailLayout->addRow(m_gateWayEmptyLabel);
    m_detailLayout->addRow(divider);
    m_detailLayout->addRow(m_dnsEmptyLabel);
    m_detailLayout->addRow(m_dnsWidget);
    m_addressLabel->setContentsMargins(0, 0, 0, LABEL_HEIGHT);  //解决布局错位问题
    m_maskLabel->setContentsMargins(0, 0, 0, LABEL_HEIGHT);

    ipv4ConfigCombox->addItem(tr("Auto(DHCP)")); //"自动(DHCP)"
    ipv4ConfigCombox->addItem(tr("Manual")); //"手动"

//    netMaskCombox->addItem("");
//    netMaskCombox->addItem("255.255.255.0"); //24
//    netMaskCombox->addItem("255.255.254.0"); //23
//    netMaskCombox->addItem("255.255.252.0"); //22
//    netMaskCombox->addItem("255.255.0.0"); //16
//    netMaskCombox->addItem("255.0.0.0"); //8

    ipv4addressEdit->setValidator(new QRegExpValidator(rx, this));
    gateWayEdit->setValidator(new QRegExpValidator(rx, this));
    netMaskEdit->setValidator(new QRegExpValidator(rx, this));

    initLoadingIcon();
}

void Ipv4Page::initComponent() {
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        setLineEnabled(false);
    } else if (ipv4ConfigCombox->currentIndex() == MANUAL_CONFIG) {
        setLineEnabled(true);
    }
    connect(ipv4ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(configChanged(int)));

    connect(ipv4addressEdit, SIGNAL(textChanged(QString)), this, SLOT(onAddressTextChanged()));
    connect(ipv4addressEdit, SIGNAL(editingFinished()), this, SLOT(onAddressEditFinished()));
    connect(netMaskEdit, SIGNAL(textChanged(QString)), this, SLOT(onNetMaskTextChanged()));
    connect(ipv4ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfSaveBtn()));
    connect(ipv4addressEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(netMaskEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(gateWayEdit, SIGNAL(textChanged(QString)), this, SLOT(setEnableOfSaveBtn()));
    connect(m_dnsWidget, &MultipleDnsWidget::scrollToBottom, this, &Ipv4Page::scrollToBottom);
}

void Ipv4Page::setIpv4Config(KyIpConfigType ipv4Config)
{
    if (ipv4Config == CONFIG_IP_MANUAL) {
        ipv4ConfigCombox->setCurrentIndex(MANUAL_CONFIG);
    } else {
        ipv4ConfigCombox->setCurrentIndex(AUTO_CONFIG);
    }
}

void Ipv4Page::setIpv4(const QString &ipv4)
{
    ipv4addressEdit->setText(ipv4);
}

void Ipv4Page::setNetMask(const QString &netMask)
{
    netMaskEdit->setText(netMask);
}

void Ipv4Page::setMulDns(const QList<QHostAddress> &dns)
{
    m_dnsWidget->setDnsListText(dns);
}

void Ipv4Page::setGateWay(const QString &gateWay)
{
    gateWayEdit->setText(gateWay);
}

bool Ipv4Page::checkIsChanged(const ConInfo info, KyConnectSetting &setting)
{
    bool isChanged = false;
    KyIpConfigType type;
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        type = CONFIG_IP_DHCP;
        if (info.ipv4ConfigType != CONFIG_IP_DHCP) {
            qDebug() << "ipv4ConfigType change to Auto";
            isChanged = true;
        }
    } else {
        type = CONFIG_IP_MANUAL;
        if (info.ipv4ConfigType != CONFIG_IP_MANUAL) {
            qDebug() << "ipv4ConfigType change to Manual";
            isChanged =  true;
        } else {
            if(info.strIPV4Address != ipv4addressEdit->text()
                    || info.strIPV4NetMask != getNetMaskText(netMaskEdit->text())
                    || info.strIPV4GateWay != gateWayEdit->text()) {
                qDebug() << "ipv4 info changed";
                isChanged =  true;
            }
        }
    }

    QList<QHostAddress> ipv4dnsList;
    ipv4dnsList.clear();
        ipv4dnsList = m_dnsWidget->getDns();
    if(info.ipv4DnsList != ipv4dnsList) {
        isChanged = true;
    }

    if (isChanged) {
        setting.setIpConfigType(IPADDRESS_V4, type);
        QString ipv4address = ipv4addressEdit->text();
        QString netMask = getNetMaskText(netMaskEdit->text());
        QString gateWay = gateWayEdit->text();
        setting.ipv4AddressConstruct(ipv4address, netMask, gateWay);
        setting.ipv4DnsConstruct(ipv4dnsList);
        setting.dumpInfo();
    }

    return isChanged;
}

bool Ipv4Page::checkDnsSettingsIsChanged() {
    return m_dnsWidget->getDnsSettingsChanged();
}

bool Ipv4Page::checkConnectBtnIsEnabled()
{
    qDebug() << "checkConnectBtnIsEnabled currentIndex" << ipv4ConfigCombox->currentIndex();
    if (ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        return true;
    } else {
        if (ipv4addressEdit->text().isEmpty() || !getTextEditState(ipv4addressEdit->text())) {
            qDebug() << "ipv4address empty or invalid";
            return false;
        }

        if (netMaskEdit->text().isEmpty() || !netMaskIsValide(netMaskEdit->text())) {
            qDebug() << "ipv4 netMask empty or invalid";
            return false;
        }
    }
    return true;
}

void Ipv4Page::configChanged(int index) {
    if (index == AUTO_CONFIG) {
        setLineEnabled(false);
    }
    if (index == MANUAL_CONFIG) {
        setLineEnabled(true);
    }
}

void Ipv4Page::onAddressTextChanged()
{
    m_iconLabel->hide();
    m_textLabel->hide();

    if (!getTextEditState(ipv4addressEdit->text())) {
        m_addressHintLabel->setText(tr("Invalid address"));
    } else {
        m_addressHintLabel->clear();
    }
}

void Ipv4Page::onNetMaskTextChanged()
{
    if (!netMaskIsValide(netMaskEdit->text())) {
        m_maskHintLabel->setText(tr("Invalid subnet mask"));
    } else {
        m_maskHintLabel->clear();
    }
}

void Ipv4Page::onAddressEditFinished()
{
    if (ipv4addressEdit->isModified()) {
        if (!ipv4addressEdit->text().isEmpty() && getTextEditState(ipv4addressEdit->text())) {
            Q_EMIT ipv4EditFinished(ipv4addressEdit->text());
        }
    }
}

void Ipv4Page::setLineEnabled(bool check) {
    if (!check) {
        ipv4addressEdit->clear();
        netMaskEdit->clear();
        gateWayEdit->clear();

        ipv4addressEdit->setPlaceholderText(" ");
        netMaskEdit->setPlaceholderText(" ");

    } else {
        ipv4addressEdit->setPlaceholderText(tr("Required")); //必填
        netMaskEdit->setPlaceholderText(tr("Required")); //必填
    }

    ipv4addressEdit->setEnabled(check);
    netMaskEdit->setEnabled(check);
    gateWayEdit->setEnabled(check);
}

void Ipv4Page::setEnableOfSaveBtn() {
    Q_EMIT setIpv4PageState(checkConnectBtnIsEnabled());
}

bool Ipv4Page::getTextEditState(QString text)
{
    if (text.isEmpty()) {
        return true;
    }
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    bool match = false;
    match = rx.exactMatch(text);

    return match;
}

bool Ipv4Page::netMaskIsValide(QString text)
{
    if (getTextEditState(text)) {
        return true;
    } else {
        if (text.length() > 0 && text.length() < 3) {
            int num = text.toInt();
            if (num > 0 && num < 33) {
                return true;
            }
        }
    }
    return false;
}

QString Ipv4Page::getNetMaskText(QString text)
{
    if (text.length() > 2) {
        return text;
    }

    int num = text.toInt();
    QStringList list;
    list << "0" << "0" << "0" << "0";
    int count = 0;
    while (num - 8 >= 0) {
        list[count] = "255";
        num = num - 8;
        count ++;
    }
    if (num > 0) {
        int size = pow(2, 8) - pow(2,(8-num));
        list[count] = QString::number(size);
    }
    return QString("%1.%2.%3.%4").arg(list[0],list[1],list[2],list[3]);
}

void Ipv4Page::initConflictHintLable()
{
    QIcon icon = QIcon::fromTheme("dialog-warning");
    m_iconLabel = new QLabel(m_addressHintLabel);
    m_iconLabel->setPixmap(icon.pixmap(ICON_SIZE));
    m_textLabel = new QLabel(m_addressHintLabel);
    m_textLabel->setText(tr("Address conflict"));
    QHBoxLayout *conflictHintLayout = new QHBoxLayout(m_addressHintLabel);
    conflictHintLayout->setContentsMargins(0, 0, 0, 0);
    conflictHintLayout->addWidget(m_iconLabel);
    conflictHintLayout->addWidget(m_textLabel);
    conflictHintLayout->addStretch();
    m_addressHintLabel->setLayout(conflictHintLayout);
    m_iconLabel->hide();
    m_textLabel->hide();
}

void Ipv4Page::initLoadingIcon()
{
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-1-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-2-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-3-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-4-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-5-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-6-symbolic"));
    m_loadIcons.append(QIcon::fromTheme("lingmo-loading-7-symbolic"));
    m_iconTimer = new QTimer(this);
    connect(m_iconTimer, &QTimer::timeout, this, &Ipv4Page::updateIcon);
}

void Ipv4Page::updateIcon()
{
    if (m_currentIconIndex > 6) {
        m_currentIconIndex = 0;
    }
    m_statusLabel->setPixmap(m_loadIcons.at(m_currentIconIndex).pixmap(ICON_SIZE));
    m_currentIconIndex ++;
}

void Ipv4Page::startLoading()
{
    m_iconTimer->start(FRAME_SPEED);
}

void Ipv4Page::stopLoading()
{
    m_iconTimer->stop();
    m_statusLabel->clear();
}

void Ipv4Page::showIpv4AddressConflict(bool isConflict)
{
    if (isConflict) {
        m_iconLabel->show();
        m_textLabel->show();
    } else {
        m_iconLabel->hide();
        m_textLabel->hide();
    }
}
