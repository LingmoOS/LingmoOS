/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukáš Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef OPENVPNWIDGET_H
#define OPENVPNWIDGET_H

#include "settingwidget.h"

#include <QProcess>

#include "ui_openvpn.h"

#include <NetworkManagerQt/VpnSetting>

class QUrl;
class QLineEdit;

class OpenVpnSettingWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit OpenVpnSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);
    ~OpenVpnSettingWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void updateStartDir(const QUrl &);
    void showAdvanced();

private:
    class Private;
    Private *const d;
    void setPasswordType(QLineEdit *, int);
    void fillOnePasswordCombo(PasswordField *, NetworkManager::Setting::SecretFlags);
    void handleOnePasswordType(const PasswordField *, const QString &, NMStringMap &) const;
};

#endif // OPENVPNWIDGET_H
