// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNSSTPPROXYSECTION_H
#define VPNSSTPPROXYSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
}

namespace dcc {
namespace network {
class SpinBoxWidget;
}
}

class VpnSstpProxySection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnSstpProxySection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnSstpProxySection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnection();

private:
    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;
    NMStringMap m_secretMap;

    DCC_NAMESPACE::LineEditWidget *m_server;
    dcc::network::SpinBoxWidget *m_port;
    DCC_NAMESPACE::LineEditWidget *m_userName;
    DCC_NAMESPACE::LineEditWidget *m_password;
};

#endif /* VPNSSTPPROXYSECTION_H */
