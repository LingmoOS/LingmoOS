/*
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 *
 */
#include "vpnipv4page.h"
#include "math.h"
#include <QDebug>

#define  NO_LAYOUT_MARGINS  0, 0, 0, 0
#define  NO_SPACE  0
#define  HLAYOUT_SPACING  16
#define  VLAYOUT_SPACING  24
#define  AUTO_CONFIG  0
#define  MANUAL_CONFIG  1
#define  LOG_FLAG  "[VPN IPv4Page]"

VpnIpv4Page::VpnIpv4Page(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
}

void VpnIpv4Page::setVpnIpv4Info(const KyVpnConfig &vpnInfo)
{
    if (vpnInfo.m_ipv4ConfigIpType == CONFIG_IP_DHCP) {
        m_ipv4ConfigCombox->setCurrentIndex(AUTO_CONFIG);
    } else {
        m_ipv4ConfigCombox->setCurrentIndex(MANUAL_CONFIG);
        if (!vpnInfo.m_ipv4Address.isEmpty()) {
            m_ipv4addressEdit->setText(vpnInfo.m_ipv4Address.at(0).ip().toString());
            m_netMaskEdit->setText(vpnInfo.m_ipv4Address.at(0).netmask().toString());
            m_gateWayEdit->setText(vpnInfo.m_ipv4Address.at(0).gateway().toString());
        }
    }
}

void VpnIpv4Page::setDns(const QString &firstDns)
{
    m_dnsServerEdit->setText(firstDns);
}

void VpnIpv4Page::setSearchDomain(const QString &searchDomain)
{
    m_searchDomainEdit->setText(searchDomain);
}

void VpnIpv4Page::setDhcpClientId(const QString &dhcpId)
{
    m_dhcpClientIdEdit->setText(dhcpId);
}

bool VpnIpv4Page::checkIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;

    if (m_ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        if (vpnInfo.m_ipv4ConfigIpType != CONFIG_IP_DHCP) {
            qDebug() << LOG_FLAG << "ipv4ConfigType change to Auto";
            isChanged = true;
        }
    } else {
        if (vpnInfo.m_ipv4ConfigIpType != CONFIG_IP_MANUAL) {
            qDebug() << LOG_FLAG <<  "ipv4ConfigType change to Manual";
            isChanged = true;
        }

        if (!vpnInfo.m_ipv4Address.isEmpty()) {
            if(vpnInfo.m_ipv4Address.at(0).ip().toString() != m_ipv4addressEdit->text()
                    || vpnInfo.m_ipv4Address.at(0).netmask().toString() != getNetMaskText(m_netMaskEdit->text())
                    || vpnInfo.m_ipv4Address.at(0).gateway().toString() != m_gateWayEdit->text()) {

                isChanged = true;
            }
        }
    }

    if (!vpnInfo.m_ipv4Dns.isEmpty() && vpnInfo.m_ipv4Dns.at(0).toString() != m_dnsServerEdit->text()) {
        isChanged = true;
    }

    if (!vpnInfo.m_ipv4DnsSearch.isEmpty() && vpnInfo.m_ipv4DnsSearch.at(0) != m_searchDomainEdit->text()) {
        isChanged = true;
    }
    if (vpnInfo.m_ipv4DhcpClientId != m_dhcpClientIdEdit->text()){
        isChanged = true;
    }
    return isChanged;
}

void VpnIpv4Page::updateVpnIpv4Info(KyVpnConfig &vpnInfo)
{
    if (m_ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        QString ipv4address("");
        QString netMask("");
        QString gateWay("");
        vpnInfo.setIpConfigType(IPADDRESS_V4, CONFIG_IP_DHCP);
        qDebug() << LOG_FLAG << "IPv4 method" << vpnInfo.m_ipv4ConfigIpType;
        qDebug() << LOG_FLAG <<  "Update IPv4 info finished";
    } else {
        QString ipv4address =m_ipv4addressEdit->text();
        QString netMask = getNetMaskText(m_netMaskEdit->text());
        QString gateWay = m_gateWayEdit->text();
        qDebug() << ipv4address << netMask << gateWay;
        vpnInfo.setIpConfigType(IPADDRESS_V4, CONFIG_IP_MANUAL);
        vpnInfo.dumpInfo();
        qDebug() << LOG_FLAG <<  "Update IPv4 info finished";
    }

    QList<QHostAddress> ipv4DnsList;
    ipv4DnsList.clear();
    if (!m_dnsServerEdit->text().isEmpty()) {
        ipv4DnsList.append(QHostAddress(m_dnsServerEdit->text()));
    }

    vpnInfo.m_ipv4DnsSearch.clear();
    vpnInfo.m_ipv4DnsSearch.append(m_searchDomainEdit->text());
    vpnInfo.m_ipv4DhcpClientId = m_dhcpClientIdEdit->text();
}

void VpnIpv4Page::initUI()
{
    /**********控件**********/
    m_configLabel = new QLabel(this);
    m_addressLabel = new QLabel(this);
    m_maskLabel = new QLabel(this);
    m_gateWayLabel = new QLabel(this);
    m_dnsServerLabel = new QLabel(this);
    m_searchDomainLabel = new QLabel(this);
    m_dhcpClientIdLabel = new QLabel(this);

    m_ipv4ConfigCombox =new QComboBox(this);
    m_ipv4addressEdit = new QLineEdit(this);
    m_netMaskEdit = new QLineEdit(this);
    m_gateWayEdit = new QLineEdit(this);
    m_dnsServerEdit = new QLineEdit(this);
    m_searchDomainEdit = new QLineEdit(this);
    m_dhcpClientIdEdit = new QLineEdit(this);


    /**********布局**********/
    QFormLayout *mainLayout = new QFormLayout(this);
    mainLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    mainLayout->setHorizontalSpacing(HLAYOUT_SPACING);
    mainLayout->setVerticalSpacing(VLAYOUT_SPACING);
    mainLayout->addRow(m_configLabel, m_ipv4ConfigCombox);
    mainLayout->addRow(m_addressLabel, m_ipv4addressEdit);
    mainLayout->addRow(m_maskLabel, m_netMaskEdit);
    mainLayout->addRow(m_gateWayLabel, m_gateWayEdit);
    mainLayout->addRow(m_dnsServerLabel, m_dnsServerEdit);
    mainLayout->addRow(m_searchDomainLabel, m_searchDomainEdit);
    mainLayout->addRow(m_dhcpClientIdLabel, m_dhcpClientIdEdit);


    /**********控件显示**********/
    m_configLabel->setText(tr("IPv4 Config"));
    m_addressLabel->setText(tr("Address"));
    m_maskLabel->setText(tr("Netmask"));
    m_gateWayLabel->setText(tr("Default Gateway"));
    m_dnsServerLabel->setText(tr("DNS Server"));
    m_searchDomainLabel->setText(tr("Search Domain")); //搜索域
    m_dhcpClientIdLabel->setText(tr("DHCP Client ID")); //DHCP客户端ID

    m_ipv4ConfigCombox->addItem(tr("Auto(DHCP)"), AUTO_CONFIG); //"自动(DHCP)"
    m_ipv4ConfigCombox->addItem(tr("Manual"), MANUAL_CONFIG); //"手动"

    // IP的正则格式限制
    QRegExp rxIp("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    QRegExpValidator *ipExpVal = new QRegExpValidator(rxIp, this);
    m_ipv4addressEdit->setValidator(ipExpVal);
    m_netMaskEdit->setValidator(ipExpVal);
    m_gateWayEdit->setValidator(ipExpVal);
    m_dnsServerEdit->setValidator(ipExpVal);
}

void VpnIpv4Page::initConnect()
{
    if (m_ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        setManualEnabled(false);
    } else if (m_ipv4ConfigCombox->currentIndex() == MANUAL_CONFIG) {
        setManualEnabled(true);
    }
    connect(m_ipv4ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(onConfigChanged(int)));
    connect(m_ipv4ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfConfirmBtn()));
    connect(m_ipv4addressEdit, &QLineEdit::textChanged, this, &VpnIpv4Page::setEnableOfConfirmBtn);
    connect(m_netMaskEdit, &QLineEdit::textChanged, this, &VpnIpv4Page::setEnableOfConfirmBtn);
    connect(m_gateWayEdit, &QLineEdit::textChanged, this, &VpnIpv4Page::setEnableOfConfirmBtn);
    connect(m_dnsServerEdit, &QLineEdit::textChanged, this, &VpnIpv4Page::setEnableOfConfirmBtn);
    connect(m_searchDomainEdit, &QLineEdit::textChanged, this, &VpnIpv4Page::setEnableOfConfirmBtn);
    connect(m_dhcpClientIdEdit, &QLineEdit::textChanged, this, &VpnIpv4Page::setEnableOfConfirmBtn);
}

void VpnIpv4Page::setManualEnabled(bool state)
{
    m_addressLabel->setEnabled(state);
    m_maskLabel->setEnabled(state);
    m_gateWayLabel->setEnabled(state);
    m_ipv4addressEdit->setEnabled(state);
    m_netMaskEdit->setEnabled(state);
    m_gateWayEdit->setEnabled(state);
}

bool VpnIpv4Page::getTextEditState(QString text)
{
    if (text.isEmpty()) {
        return true;
    }
    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");

    bool match = false;
    match = rx.exactMatch(text);

    return match;
}

bool VpnIpv4Page::netMaskIsValide(QString text)
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

QString VpnIpv4Page::getNetMaskText(QString text)
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

bool VpnIpv4Page::checkConfirmBtnIsEnabled()
{
    if (m_ipv4ConfigCombox->currentIndex() == AUTO_CONFIG) {
        return true;
    } else {
        if (m_ipv4addressEdit->text().isEmpty() || !getTextEditState(m_ipv4addressEdit->text())) {
            qDebug() << LOG_FLAG <<  "IPv4 address empty or invalid";
            return false;
        }

        if (m_netMaskEdit->text().isEmpty() || !netMaskIsValide(m_netMaskEdit->text())) {
            qDebug() << LOG_FLAG <<  "IPv4 netMask empty or invalid";
            return false;
        }
    }
    return true;
}

void VpnIpv4Page::setEnableOfConfirmBtn()
{
    Q_EMIT setIpv4PageState(checkConfirmBtnIsEnabled());
}

void VpnIpv4Page::onConfigChanged(int index)
{
    if (index == AUTO_CONFIG) {
        m_ipv4addressEdit->clear();
        m_netMaskEdit->clear();
        m_gateWayEdit->clear();
        setManualEnabled(false);
    } else if (index == MANUAL_CONFIG) {
        setManualEnabled(true);
    }
}
