// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNADVVPNCSECTION_H
#define VPNADVVPNCSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace dcc {
namespace network {
class SpinBoxWidget;
}
}

namespace DCC_NAMESPACE {
class LineEditWidget;
class ComboxWidget;
class SwitchWidget;
}

class VpnAdvVPNCSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnAdvVPNCSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnAdvVPNCSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QPair<QString, QString>> VendorStrMap;
    QList<QPair<QString, QString>> EncryptionStrMap;
    QList<QPair<QString, QString>> NATTravModeStrMap;
    QList<QPair<QString, QString>> IKEDHGroupStrMap;
    QList<QPair<QString, QString>> ForwardSecrecyStrMap;

    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    DCC_NAMESPACE::LineEditWidget *m_domain;
    DCC_NAMESPACE::ComboxWidget *m_vendorChooser;
    DCC_NAMESPACE::LineEditWidget *m_version;
    DCC_NAMESPACE::ComboxWidget *m_encryptionChooser;
    DCC_NAMESPACE::ComboxWidget *m_natTravModeChooser;
    DCC_NAMESPACE::ComboxWidget *m_ikeDHGroupChooser;
    DCC_NAMESPACE::ComboxWidget *m_forwordSecrecyChooser;
    dcc::network::SpinBoxWidget *m_localPort;
    DCC_NAMESPACE::SwitchWidget *m_disableDPD;

    QString m_currentVendor;
    QString m_currentEncryption;
    QString m_currentNatTravMod;
    QString m_currentIkeDHGroup;
    QString m_currentForwordSecrecy;
};

#endif /* VPNADVVPNCSECTION_H */
