// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNVPNCSECTION_H
#define VPNVPNCSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
class ComboxWidget;
class SwitchWidget;
}

namespace dcc {
namespace network {
class FileChooseWidget;
}
}

using namespace NetworkManager;

class VpnVPNCSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnVPNCSection(VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnVPNCSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onPasswordFlagsChanged(Setting::SecretFlagType type);
    void onGroupPasswordFlagsChanged(Setting::SecretFlagType type);
    bool isIpv4Address(const QString &ip);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QPair<QString, Setting::SecretFlagType>> PasswordFlagsStrMap;

    VpnSetting::Ptr m_vpnSetting;
    Setting::SecretFlagType m_currentPasswordType;
    Setting::SecretFlagType m_currentGroupPasswordType;
    NMStringMap m_dataMap;
    NMStringMap m_secretMap;

    DCC_NAMESPACE::LineEditWidget *m_gateway;
    DCC_NAMESPACE::LineEditWidget *m_userName;
    DCC_NAMESPACE::ComboxWidget *m_passwordFlagsChooser;
    DCC_NAMESPACE::LineEditWidget *m_password;

    DCC_NAMESPACE::LineEditWidget *m_groupName;
    DCC_NAMESPACE::ComboxWidget *m_groupPasswordFlagsChooser;
    DCC_NAMESPACE::LineEditWidget *m_groupPassword;
    DCC_NAMESPACE::SwitchWidget *m_userHybrid;
    dcc::network::FileChooseWidget *m_caFile;
};

#include "declare_metatype_for_networkmanager.h"

#endif /* VPNVPNCSECTION_H */
