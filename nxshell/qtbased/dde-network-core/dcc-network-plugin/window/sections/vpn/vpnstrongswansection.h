// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNSTRONGSWANSECTION_H
#define VPNSTRONGSWANSECTION_H

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

class VpnStrongSwanSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnStrongSwanSection(VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnStrongSwanSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onAuthTypeChanged(const QString &type);
    void onCustomCipherEnableChanged(const bool enabled);
    bool isIpv4Address(const QString &ip);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QPair<QString, QString>> AuthTypeStrMap;

    VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;
    NMStringMap m_secretMap;

    DCC_NAMESPACE::LineEditWidget *m_gateway;
    dcc::network::FileChooseWidget *m_caCert;
    DCC_NAMESPACE::ComboxWidget *m_authTypeChooser;

    dcc::network::FileChooseWidget *m_userCert;
    dcc::network::FileChooseWidget *m_userKey;
    DCC_NAMESPACE::LineEditWidget *m_userName;
    DCC_NAMESPACE::LineEditWidget *m_password;

    DCC_NAMESPACE::SwitchWidget *m_requestInnerIp;
    DCC_NAMESPACE::SwitchWidget *m_enforceUDP;
    DCC_NAMESPACE::SwitchWidget *m_useIPComp;
    DCC_NAMESPACE::SwitchWidget *m_enableCustomCipher;
    DCC_NAMESPACE::LineEditWidget *m_ike;
    DCC_NAMESPACE::LineEditWidget *m_esp;

    QString m_currentAuthType;
};

#endif /* VPNSTRONGSWANSECTION_H */
