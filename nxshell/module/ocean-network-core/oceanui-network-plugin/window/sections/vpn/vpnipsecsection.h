// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNIPSECSECTION_H
#define VPNIPSECSECTION_H

#include "../abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class SwitchWidget;
class LineEditWidget;
}

class VpnIpsecSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnIpsecSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnIpsecSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initUI();
    void initConnection();
    void onIpsecCheckedChanged(const bool enabled);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    DCC_NAMESPACE::SwitchWidget *m_ipsecEnable;
    DCC_NAMESPACE::LineEditWidget *m_groupName;
    DCC_NAMESPACE::LineEditWidget *m_gatewayId;
    DCC_NAMESPACE::LineEditWidget *m_psk;
    DCC_NAMESPACE::LineEditWidget *m_ike;
    DCC_NAMESPACE::LineEditWidget *m_esp;
};

#endif /* VPNIPSECSECTION_H */
