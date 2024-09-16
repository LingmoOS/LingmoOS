// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNSECTION_H
#define VPNSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class LineEditWidget;
class ComboxWidget;
}

class VpnSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onPasswordFlagsChanged(NetworkManager::Setting::SecretFlagType type);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    bool isIpv4Address(const QString &ip);

private:
    QList<QPair<QString, NetworkManager::Setting::SecretFlagType>> PasswordFlagsStrMap;

    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NetworkManager::Setting::SecretFlagType m_currentPasswordType;
    NMStringMap m_dataMap;
    NMStringMap m_secretMap;

    DCC_NAMESPACE::LineEditWidget *m_gateway;
    DCC_NAMESPACE::LineEditWidget *m_userName;
    DCC_NAMESPACE::ComboxWidget *m_passwordFlagsChooser;
    DCC_NAMESPACE::LineEditWidget *m_password;
    DCC_NAMESPACE::LineEditWidget *m_domain;
};

#include "declare_metatype_for_networkmanager.h"

#endif /* VPNSECTION_H */
