/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_WIFI_SECURITY_H
#define PLASMA_NM_WIFI_SECURITY_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/Security8021xSetting>
#include <NetworkManagerQt/WirelessSecuritySetting>

#include "security802-1x.h"
#include "settingwidget.h"

namespace Ui
{
class WifiSecurity;
}

class PLASMANM_EDITOR_EXPORT WifiSecurity : public SettingWidget
{
    Q_OBJECT
public:
    // Keep this in sync with NetworkManager::WirelessSecurityType from
    // NetworkManagerQt.
    enum SecurityTypeIndex { None = 0, WepHex, WepPassphrase, Leap, DynamicWep, WpaPsk, WpaEap, SAE, Wpa3SuiteB192, OWE };

    explicit WifiSecurity(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                          const NetworkManager::Security8021xSetting::Ptr &setting8021x = NetworkManager::Security8021xSetting::Ptr(),
                          QWidget *parent = nullptr,
                          Qt::WindowFlags f = {});
    ~WifiSecurity() override;
    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;
    QVariantMap setting8021x() const;

    bool enabled() const;
    bool enabled8021x() const;

    bool isValid() const override;

    void setStoreSecretsSystemWide(bool system);

public Q_SLOTS:
    void onSsidChanged(const QString &ssid);

private Q_SLOTS:
    void securityChanged(int index);
    void setWepKey(int keyIndex);

private:
    Ui::WifiSecurity *const m_ui;
    Security8021x *m_8021xWidget = nullptr;
    Security8021x *m_WPA2Widget = nullptr;
    Security8021x *m_WPA3SuiteB192Widget = nullptr;
    NetworkManager::WirelessSecuritySetting::Ptr m_wifiSecurity;
    bool m_systemWideDefault = false;
};

#endif // PLASMA_NM_WIFI_SECURITY_H
