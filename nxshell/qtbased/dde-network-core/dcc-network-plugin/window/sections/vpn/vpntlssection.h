// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef VPNTLSSECTION_H
#define VPNTLSSECTION_H

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

class VpnTLSSection : public AbstractSection
{
    Q_OBJECT

public:
    explicit VpnTLSSection(NetworkManager::VpnSetting::Ptr vpnSetting, QFrame *parent = nullptr);
    virtual ~VpnTLSSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QList<QPair<QString, QString>> RemoteCertTypeStrMap;

    NetworkManager::VpnSetting::Ptr m_vpnSetting;
    NMStringMap m_dataMap;

    DCC_NAMESPACE::LineEditWidget *m_remote;
    DCC_NAMESPACE::ComboxWidget *m_remoteCertTypeChooser;
    dcc::network::FileChooseWidget *m_caCert;
    DCC_NAMESPACE::SwitchWidget *m_customKeyDirection;
    DCC_NAMESPACE::ComboxWidget *m_keyDirectionChooser;

    QString m_currentRemoteCertType;
    QString m_currentKeyDirection;
};

#endif /* VPNTLSSECTION_H */
