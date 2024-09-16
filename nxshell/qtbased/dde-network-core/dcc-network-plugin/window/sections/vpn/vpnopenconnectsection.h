// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNOPENCONNECTSECTION_H
#define VPNOPENCONNECTSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
class SwitchWidget;
}
namespace dcc {
namespace network {
class FileChooseWidget;
}
}

class VpnOpenConnectSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnOpenConnectSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = Q_NULLPTR);
    virtual ~VpnOpenConnectSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnect();
    bool isIpv4Address(const QString &ip);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    DCC_NAMESPACE::LineEditWidget *m_gateway;
    dcc::network::FileChooseWidget *m_caCert;
    DCC_NAMESPACE::LineEditWidget *m_proxy;
    DCC_NAMESPACE::SwitchWidget *m_enableCSDTrojan;
    DCC_NAMESPACE::LineEditWidget *m_csdScript;
    dcc::network::FileChooseWidget *m_userCert;
    dcc::network::FileChooseWidget *m_userKey;
    DCC_NAMESPACE::SwitchWidget *m_useFSID;
};

#endif /* VPNOPENCONNECTSECTION_H */
