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
#ifndef VPNIPV4PAGE_H
#define VPNIPV4PAGE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>

#include "kyvpnconnectoperation.h"

class VpnIpv4Page : public QFrame
{
    Q_OBJECT
public:
    VpnIpv4Page(QWidget *parent = nullptr);
    ~VpnIpv4Page() = default;

    void setVpnIpv4Info(const KyVpnConfig &vpnInfo);
    void setDns(const QString &firstDns);
    void setSearchDomain(const QString &searchDomain);
    void setDhcpClientId(const QString &dhcpId);

    bool checkIsChanged(const KyVpnConfig &vpnInfo);
    void updateVpnIpv4Info(KyVpnConfig &vpnInfo);

private:
    QLabel    *m_configLabel;
    QLabel    *m_addressLabel;
    QLabel    *m_maskLabel;
    QLabel    *m_gateWayLabel;
    QLabel    *m_dnsServerLabel;
    QLabel    *m_searchDomainLabel;
    QLabel    *m_dhcpClientIdLabel;

    QComboBox *m_ipv4ConfigCombox;
    QLineEdit *m_ipv4addressEdit;
    QLineEdit *m_netMaskEdit;
    QLineEdit *m_gateWayEdit;
    QLineEdit *m_dnsServerEdit;
    QLineEdit *m_searchDomainEdit;
    QLineEdit *m_dhcpClientIdEdit;

    void initUI();
    void initConnect();
    void setManualEnabled(bool state);

    bool getTextEditState(QString text);
    bool netMaskIsValide(QString text);
    QString getNetMaskText(QString text);
    bool checkConfirmBtnIsEnabled();

private Q_SLOTS:
    void setEnableOfConfirmBtn();
    void onConfigChanged(int index);

Q_SIGNALS:
    void setIpv4PageState(bool);
};

#endif // VPNIPV4PAGE_H
