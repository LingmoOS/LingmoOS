// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNADVOPENVPNSECTION_H
#define VPNADVOPENVPNSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class SwitchWidget;
}

namespace dcc {
namespace network {
class SpinBoxWidget;
}
}

class VpnAdvOpenVPNSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnAdvOpenVPNSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnAdvOpenVPNSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnection();
    virtual bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;

private:
    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    DCC_NAMESPACE::SwitchWidget *m_portSwitch;
    DCC_NAMESPACE::SwitchWidget *m_renegIntervalSwitch;
    DCC_NAMESPACE::SwitchWidget *m_compLZOSwitch;
    DCC_NAMESPACE::SwitchWidget *m_tcpProtoSwitch;
    DCC_NAMESPACE::SwitchWidget *m_useTapSwitch;
    DCC_NAMESPACE::SwitchWidget *m_tunnelMTUSwitch;
    DCC_NAMESPACE::SwitchWidget *m_udpFragSizeSwitch;
    DCC_NAMESPACE::SwitchWidget *m_restrictMSSSwitch;
    DCC_NAMESPACE::SwitchWidget *m_randomRemoteSwitch;

    dcc::network::SpinBoxWidget *m_port;
    dcc::network::SpinBoxWidget *m_renegInterval;
    dcc::network::SpinBoxWidget *m_tunnelMTU;
    dcc::network::SpinBoxWidget *m_udpFragSize;
};

#endif /* VPNADVOPENVPNSECTION_H */
