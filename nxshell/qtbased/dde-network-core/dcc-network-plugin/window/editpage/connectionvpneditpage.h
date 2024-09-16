// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef CONNECTIONVPNEDITPAGE_H
#define CONNECTIONVPNEDITPAGE_H

#include "connectioneditpage.h"

#include <networkmanagerqt/accesspoint.h>

#define ServiceTypeL2TP         "org.freedesktop.NetworkManager.l2tp"
#define ServiceTypePPTP         "org.freedesktop.NetworkManager.pptp"
#define ServiceTypeVPNC         "org.freedesktop.NetworkManager.vpnc"
#define ServiceTypeOpenVPN      "org.freedesktop.NetworkManager.openvpn"
#define ServiceTypeStrongSwan   "org.freedesktop.NetworkManager.strongswan"
#define ServiceTypeOpenConnect  "org.freedesktop.NetworkManager.openconnect"
#define ServiceTypeSSTP         "org.freedesktop.NetworkManager.sstp"

class ConnectionVpnEditPage : public ConnectionEditPage
{
    Q_OBJECT

public:
    enum VpnType {UNSET, L2TP, PPTP, VPNC, OPENVPN, STRONGSWAN, OPENCONNECT};

    explicit ConnectionVpnEditPage(const QString &connUuid = QString(), QWidget *parent = Q_NULLPTR);
    virtual ~ConnectionVpnEditPage() Q_DECL_OVERRIDE;

    // one of the following two methods must be called after initialization
    void initSettingsWidget() Q_DECL_OVERRIDE;
    // init Widget by create a new vpn connection
    void initSettingsWidgetByType(ConnectionVpnEditPage::VpnType vpnType);

private:
    void initApSecretType(NetworkManager::AccessPoint::Ptr nmAp);
    void resetConnectionIdByType(ConnectionVpnEditPage::VpnType vpnType);
    void exportConnConfig();
    void processConfigCA(const QString &file);

private:
    QPushButton *m_exportButton;
};

#endif /* CONNECTIONVPNEDITPAGE_H */
