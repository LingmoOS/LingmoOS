/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_WIRED_SECURITY_H
#define PLASMA_NM_WIRED_SECURITY_H

#include "plasmanm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/Security8021xSetting>

#include "security802-1x.h"
#include "settingwidget.h"

namespace Ui
{
class WiredSecurity;
}

class PLASMANM_EDITOR_EXPORT WiredSecurity : public SettingWidget
{
    Q_OBJECT
public:
    explicit WiredSecurity(const NetworkManager::Security8021xSetting::Ptr &setting8021x = NetworkManager::Security8021xSetting::Ptr(),
                           QWidget *parent = nullptr,
                           Qt::WindowFlags f = {});
    ~WiredSecurity() override;
    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;
    QVariantMap setting() const override;

    bool enabled8021x() const;

private:
    Ui::WiredSecurity *const m_ui;
    Security8021x *m_8021xWidget = nullptr;
    NetworkManager::Security8021xSetting::Ptr m_8021xSetting;
};

#endif // PLASMA_NM_WIRED_SECURITY_H
