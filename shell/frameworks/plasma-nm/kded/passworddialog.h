/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_PASSWORD_DIALOG_H
#define PLASMA_NM_PASSWORD_DIALOG_H

#include <NetworkManagerQt/ConnectionSettings>
#include <NetworkManagerQt/SecretAgent>

#include <QDialog>

namespace Ui
{
class PasswordDialog;
}

class SettingWidget;

class Q_DECL_EXPORT PasswordDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PasswordDialog(const NetworkManager::ConnectionSettings::Ptr &connectionSettings,
                            NetworkManager::SecretAgent::GetSecretsFlags flags,
                            const QString &setting_name,
                            const QStringList &hints = QStringList(),
                            QWidget *parent = nullptr);
    ~PasswordDialog() override;

    bool hasError() const;
    NetworkManager::SecretAgent::Error error() const;
    QString errorMessage() const;

    NMVariantMapMap secrets() const;

private:
    void initializeUi();

    Ui::PasswordDialog *m_ui = nullptr;
    bool m_hasError = false;
    QString m_errorMessage;
    QString m_settingName;
    QStringList m_neededSecrets;
    NetworkManager::ConnectionSettings::Ptr m_connectionSettings;
    NetworkManager::SecretAgent::Error m_error;
    NetworkManager::SecretAgent::GetSecretsFlags m_flags;
    SettingWidget *m_vpnWidget = nullptr;
    QStringList m_hints;
};

#endif // PLASMA_NM_PASSWORD_DIALOG_H
