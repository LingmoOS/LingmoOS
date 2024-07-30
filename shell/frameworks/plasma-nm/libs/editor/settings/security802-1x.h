/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_SECURITY8021X_H
#define PLASMA_NM_SECURITY8021X_H

#include "plasmanm_editor_export.h"

#include <QRegularExpressionValidator>
#include <QWidget>

#include <NetworkManagerQt/Security8021xSetting>

#include "passwordfield.h"
#include "settingwidget.h"

namespace Ui
{
class Security8021x;
}

class PLASMANM_EDITOR_EXPORT Security8021x : public SettingWidget
{
    Q_OBJECT
public:
    enum Type { Ethernet = 0, WirelessWpaEap, WirelessWpaEapSuiteB192 };

    explicit Security8021x(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                           Type type = WirelessWpaEap,
                           QWidget *parent = nullptr,
                           Qt::WindowFlags f = {});
    ~Security8021x() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

    void setPasswordOption(PasswordField::PasswordOption option);

private Q_SLOTS:
    void altSubjectMatchesButtonClicked();
    void connectToServersButtonClicked();
    void currentAuthChanged(int index);

private:
    NetworkManager::Security8021xSetting::Ptr m_setting;
    Ui::Security8021x *const m_ui;
    QRegularExpressionValidator *altSubjectValidator = nullptr;
    QRegularExpressionValidator *serversValidator = nullptr;
};

#endif // SECURITY8021X_H
