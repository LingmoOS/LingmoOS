// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNPROXYSECTION_H
#define VPNPROXYSECTION_H

#include "sections/abstractsection.h"

#include <networkmanagerqt/vpnsetting.h>

namespace DCC_NAMESPACE {
class SwitchWidget;
class LineEditWidget;
class ComboxWidget;
}

namespace dcc {
namespace network {
class SpinBoxWidget;
}
}

using namespace NetworkManager;

class VpnProxySection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnProxySection(VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnProxySection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onProxyTypeChanged(const QString &type);
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QPair<QString, QString>> ProxyTypeStrMap;

    VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;
    NMStringMap m_secretMap;

    DCC_NAMESPACE::ComboxWidget *m_proxyTypeChooser;
    DCC_NAMESPACE::LineEditWidget *m_server;
    dcc::network::SpinBoxWidget *m_port;
    DCC_NAMESPACE::SwitchWidget *m_retry;
    DCC_NAMESPACE::LineEditWidget *m_userName;
    DCC_NAMESPACE::LineEditWidget *m_password;

    QString m_currentProxyType;
};

#endif /* VPNPROXYSECTION_H */
