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
#include "vpnipv6page.h"
#include "math.h"
#include <QDebug>

#define  NO_LAYOUT_MARGINS  0, 0, 0, 0
#define  NO_SPACE  0
#define  HLAYOUT_SPACING  16
#define  VLAYOUT_SPACING  24
#define  AUTO_CONFIG  0
#define  MANUAL_CONFIG  1
#define  LOG_FLAG  "[VPN IPv6Page]"

VpnIpv6Page::VpnIpv6Page(QWidget *parent) : QFrame(parent)
{
    initUI();
    initConnect();
}

void VpnIpv6Page::setVpnIpv6Info(const KyVpnConfig &vpnInfo)
{
    if (vpnInfo.m_ipv6ConfigIpType == CONFIG_IP_DHCP) {
        m_ipv6ConfigCombox->setCurrentIndex(AUTO_CONFIG);
    } else {
        m_ipv6ConfigCombox->setCurrentIndex(MANUAL_CONFIG);
        if (!vpnInfo.m_ipv6Address.isEmpty()) {
            m_ipv6addressEdit->setText(vpnInfo.m_ipv6Address.at(0).ip().toString());
            m_netMaskEdit->setText(vpnInfo.m_ipv6Address.at(0).netmask().toString());
            m_gateWayEdit->setText(vpnInfo.m_ipv6Address.at(0).gateway().toString());
        }
    }
}

void VpnIpv6Page::setDns(const QString &firstDns)
{
    m_dnsServerEdit->setText(firstDns);
}

void VpnIpv6Page::setSearchDomain(const QString &searchDomain)
{
    m_searchDomainEdit->setText(searchDomain);
}

bool VpnIpv6Page::checkIsChanged(const KyVpnConfig &vpnInfo)
{
    bool isChanged = false;

    if (m_ipv6ConfigCombox->currentIndex() == AUTO_CONFIG) {
        if (vpnInfo.m_ipv6ConfigIpType != CONFIG_IP_DHCP) {
            qDebug() << LOG_FLAG << "ipv6ConfigType change to Auto";
            isChanged = true;
        }
    } else {
        if (vpnInfo.m_ipv6ConfigIpType != CONFIG_IP_MANUAL) {
            qDebug() << LOG_FLAG <<  "ipv6ConfigType change to Manual";
            isChanged = true;
        }

        if (!vpnInfo.m_ipv6Address.isEmpty()) {
            if(vpnInfo.m_ipv6Address.at(0).ip().toString() != m_ipv6addressEdit->text()
                    || vpnInfo.m_ipv6Address.at(0).netmask().toString() != m_netMaskEdit->text()
                    || vpnInfo.m_ipv6Address.at(0).gateway().toString() != m_gateWayEdit->text()) {

                isChanged = true;
            }
        }
    }

    if (!vpnInfo.m_ipv6Dns.isEmpty() && vpnInfo.m_ipv6Dns.at(0).toString() != m_dnsServerEdit->text()) {
        isChanged = true;
    }

    if (!vpnInfo.m_ipv6DnsSearch.isEmpty() && vpnInfo.m_ipv6DnsSearch.at(0) != m_searchDomainEdit->text()) {
        isChanged = true;
    }

    return isChanged;
}

void VpnIpv6Page::updateVpnIpv6Info(KyVpnConfig &vpnInfo)
{
    if (m_ipv6ConfigCombox->currentIndex() == AUTO_CONFIG) {
        QString ipv6address("");
        QString netMask("");
        QString gateWay("");
        vpnInfo.setIpConfigType(IPADDRESS_V6, CONFIG_IP_DHCP);
        qDebug() << LOG_FLAG << "IPv6 method" << vpnInfo.m_ipv6ConfigIpType;
        qDebug() << LOG_FLAG <<  "Update IPv6 info finished";
    } else {
        QString ipv6address =m_ipv6addressEdit->text();
        QString netMask = m_netMaskEdit->text();
        QString gateWay = m_gateWayEdit->text();
        qDebug() << ipv6address << netMask << gateWay;
        vpnInfo.setIpConfigType(IPADDRESS_V6, CONFIG_IP_MANUAL);
        vpnInfo.dumpInfo();
        qDebug() << LOG_FLAG <<  "Update IPv6 info finished";
    }

    QList<QHostAddress> ipv6DnsList;
    ipv6DnsList.clear();
    if (!m_dnsServerEdit->text().isEmpty()) {
        ipv6DnsList.append(QHostAddress(m_dnsServerEdit->text()));
    }

    vpnInfo.m_ipv6DnsSearch.clear();
    vpnInfo.m_ipv6DnsSearch.append(m_searchDomainEdit->text());
}

void VpnIpv6Page::initUI()
{
    /**********控件**********/
    m_configLabel = new QLabel(this);
    m_addressLabel = new QLabel(this);
    m_maskLabel = new QLabel(this);
    m_gateWayLabel = new QLabel(this);
    m_dnsServerLabel = new QLabel(this);
    m_searchDomainLabel = new QLabel(this);

    m_ipv6ConfigCombox =new QComboBox(this);
    m_ipv6addressEdit = new QLineEdit(this);
    m_netMaskEdit = new QLineEdit(this);
    m_gateWayEdit = new QLineEdit(this);
    m_dnsServerEdit = new QLineEdit(this);
    m_searchDomainEdit = new QLineEdit(this);


    /**********布局**********/
    QFormLayout *mainLayout = new QFormLayout(this);
    mainLayout->setContentsMargins(NO_LAYOUT_MARGINS);
    mainLayout->setHorizontalSpacing(HLAYOUT_SPACING);
    mainLayout->setVerticalSpacing(VLAYOUT_SPACING);
    mainLayout->addRow(m_configLabel, m_ipv6ConfigCombox);
    mainLayout->addRow(m_addressLabel, m_ipv6addressEdit);
    mainLayout->addRow(m_maskLabel, m_netMaskEdit);
    mainLayout->addRow(m_gateWayLabel, m_gateWayEdit);
    mainLayout->addRow(m_dnsServerLabel, m_dnsServerEdit);
    mainLayout->addRow(m_searchDomainLabel, m_searchDomainEdit);


    /**********控件显示**********/
    m_configLabel->setText(tr("IPv6 Config"));
    m_addressLabel->setText(tr("Address"));
    m_maskLabel->setText(tr("Netmask"));
    m_gateWayLabel->setText(tr("Default Gateway"));
    m_dnsServerLabel->setText(tr("DNS Server"));
    m_searchDomainLabel->setText(tr("Search Domain")); //搜索域

    m_ipv6ConfigCombox->addItem(tr("Auto(DHCP)"), AUTO_CONFIG); //"自动(DHCP)"
    m_ipv6ConfigCombox->addItem(tr("Manual"), MANUAL_CONFIG); //"手动"

    // IP的正则格式限制
    QRegExp ipv6_rx("^\\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:)))(%.+)?\\s*$");
    QRegExpValidator *ipv6ExpVal = new QRegExpValidator(ipv6_rx, this);
    m_ipv6addressEdit->setValidator(ipv6ExpVal);
    m_gateWayEdit->setValidator(ipv6ExpVal);
    m_dnsServerEdit->setValidator(ipv6ExpVal);

    QRegExp prefix_rx("\\b(?:(?:12[0-8]|1[0-1][0-9]|^[1-9][0-9]?$)\\.){3}(?:12[0-8]|1[0-1][0-9]|^[1-9][0-9]?$)\\b");
    m_netMaskEdit->setValidator(new QRegExpValidator(prefix_rx,this));
}

void VpnIpv6Page::initConnect()
{
    if (m_ipv6ConfigCombox->currentIndex() == AUTO_CONFIG) {
        setManualEnabled(false);
    } else if (m_ipv6ConfigCombox->currentIndex() == MANUAL_CONFIG) {
        setManualEnabled(true);
    }
    connect(m_ipv6ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(onConfigChanged(int)));
    connect(m_ipv6ConfigCombox, SIGNAL(currentIndexChanged(int)), this, SLOT(setEnableOfConfirmBtn()));
    connect(m_ipv6addressEdit, &QLineEdit::textChanged, this, &VpnIpv6Page::setEnableOfConfirmBtn);
    connect(m_netMaskEdit, &QLineEdit::textChanged, this, &VpnIpv6Page::setEnableOfConfirmBtn);
    connect(m_gateWayEdit, &QLineEdit::textChanged, this, &VpnIpv6Page::setEnableOfConfirmBtn);
    connect(m_dnsServerEdit, &QLineEdit::textChanged, this, &VpnIpv6Page::setEnableOfConfirmBtn);
    connect(m_searchDomainEdit, &QLineEdit::textChanged, this, &VpnIpv6Page::setEnableOfConfirmBtn);
}

void VpnIpv6Page::setManualEnabled(bool state)
{
    m_addressLabel->setEnabled(state);
    m_maskLabel->setEnabled(state);
    m_gateWayLabel->setEnabled(state);
    m_ipv6addressEdit->setEnabled(state);
    m_netMaskEdit->setEnabled(state);
    m_gateWayEdit->setEnabled(state);
}

bool VpnIpv6Page::getIpv6EditState(QString text)
{
    if (text.isEmpty()) {
        return true;
    }
    QRegExp rx("^\\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)(\\.(25[0-5]|2[0-4]\\d|1\\d\\d|[1-9]?\\d)){3}))|:)))(%.+)?\\s*$");

    bool match = false;
    match = rx.exactMatch(text);

    return match;
}

bool VpnIpv6Page::checkConfirmBtnIsEnabled()
{
    if (m_ipv6ConfigCombox->currentIndex() == AUTO_CONFIG) {
        return true;
    } else {
        if (m_ipv6addressEdit->text().isEmpty() || !getIpv6EditState(m_ipv6addressEdit->text())) {
            qDebug() << "IPv6 address empty or invalid";
            return false;
        }

        if (m_netMaskEdit->text().isEmpty()) {
            qDebug() << "IPv6 netMask empty or invalid";
            return false;
        }
        if (m_gateWayEdit->text().isEmpty() || !getIpv6EditState(m_gateWayEdit->text())) {
            qDebug() << "IPv6 gateway empty or invalid";
            return false;
        }
    }
    return true;
}

void VpnIpv6Page::setEnableOfConfirmBtn()
{
    Q_EMIT setIpv6PageState(checkConfirmBtnIsEnabled());
}

void VpnIpv6Page::onConfigChanged(int index)
{
    if (index == AUTO_CONFIG) {
        m_ipv6addressEdit->clear();
        m_netMaskEdit->clear();
        m_gateWayEdit->clear();
        setManualEnabled(false);
    } else if (index == MANUAL_CONFIG) {
        setManualEnabled(true);
    }
}
