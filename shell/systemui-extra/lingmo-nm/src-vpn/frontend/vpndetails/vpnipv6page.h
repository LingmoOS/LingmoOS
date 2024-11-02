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

#ifndef VPNIPV6PAGE_H
#define VPNIPV6PAGE_H

#include <QWidget>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QFormLayout>

#include "kyvpnconnectoperation.h"

class VpnIpv6Page : public QFrame
{
    Q_OBJECT
public:
    VpnIpv6Page(QWidget *parent = nullptr);
    ~VpnIpv6Page() = default;

    void setVpnIpv6Info(const KyVpnConfig &vpnInfo);
    void setDns(const QString &firstDns);
    void setSearchDomain(const QString &searchDomain);
    bool checkIsChanged(const KyVpnConfig &vpnInfo);

    void updateVpnIpv6Info(KyVpnConfig &vpnInfo);

private:
    QLabel    *m_configLabel;
    QLabel    *m_addressLabel;
    QLabel    *m_maskLabel;
    QLabel    *m_gateWayLabel;
    QLabel    *m_dnsServerLabel;
    QLabel    *m_searchDomainLabel;

    QComboBox *m_ipv6ConfigCombox;
    QLineEdit *m_ipv6addressEdit;
    QLineEdit *m_netMaskEdit;
    QLineEdit *m_gateWayEdit;
    QLineEdit *m_dnsServerEdit;
    QLineEdit *m_searchDomainEdit;

    void initUI();
    void initConnect();
    void setManualEnabled(bool state);

    bool getIpv6EditState(QString text);
    bool checkConfirmBtnIsEnabled();

private Q_SLOTS:
    void setEnableOfConfirmBtn();
    void onConfigChanged(int index);

Q_SIGNALS:
    void setIpv6PageState(bool);
};

#endif // VPNIPV6PAGE_H
