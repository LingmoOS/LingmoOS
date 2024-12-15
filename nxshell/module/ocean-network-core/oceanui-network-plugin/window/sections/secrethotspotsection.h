// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef SECRETHOTSPOTSECTION_H
#define SECRETHOTSPOTSECTION_H

#include "abstractsection.h"

#include <networkmanagerqt/wirelesssecuritysetting.h>

namespace DCC_NAMESPACE {
class ComboxWidget;
class LineEditWidget;
}

class SecretHotspotSection : public AbstractSection
{
    Q_OBJECT

public:
    SecretHotspotSection(NetworkManager::WirelessSecuritySetting::Ptr wsSeting, QFrame *parent = nullptr);
    virtual ~SecretHotspotSection() Q_DECL_OVERRIDE;

    bool allInputValid() Q_DECL_OVERRIDE;
    void saveSettings() Q_DECL_OVERRIDE;

private:
    void initStrMaps();
    void initUI();
    void initConnection();
    void onKeyMgmtChanged(NetworkManager::WirelessSecuritySetting::KeyMgmt);
    void saveUserInputPassword();
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QMap<QString, NetworkManager::WirelessSecuritySetting::KeyMgmt> KeyMgmtStrMap;

    DCC_NAMESPACE::ComboxWidget *m_keyMgmtChooser;
    DCC_NAMESPACE::LineEditWidget *m_passwdEdit;

    NetworkManager::WirelessSecuritySetting::KeyMgmt m_currentKeyMgmt;

    NetworkManager::WirelessSecuritySetting::Ptr m_wsSetting;

    QMap<NetworkManager::WirelessSecuritySetting::KeyMgmt, QString> m_userInputPasswordMap;
};

#include "declare_metatype_for_networkmanager.h"

#endif /* SECRETHOTSPOTSECTION_H */
