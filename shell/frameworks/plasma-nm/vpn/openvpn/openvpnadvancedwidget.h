/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_OPENVPN_ADVANCED_WIDGET_H
#define PLASMA_NM_OPENVPN_ADVANCED_WIDGET_H

#include "passwordfield.h"

#include <QDialog>
#include <QProcess>

#include <NetworkManagerQt/VpnSetting>

namespace Ui
{
class OpenVpnAdvancedWidget;
}

class OpenVpnAdvancedWidget : public QDialog
{
    Q_OBJECT

    enum CertCheckType {
        DontVerify = 0,
        VerifyWholeSubjectExactly,
        VerifyNameExactly,
        VerifyNameByPrefix,
        VerifySubjectPartially,
    };

public:
    explicit OpenVpnAdvancedWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);
    ~OpenVpnAdvancedWidget() override;
    void init();

    NetworkManager::VpnSetting::Ptr setting() const;

private Q_SLOTS:
    void gotOpenVpnCipherOutput();
    void openVpnCipherError(QProcess::ProcessError);
    void openVpnCipherFinished(int, QProcess::ExitStatus);
    void gotOpenVpnVersionOutput();
    void openVpnVersionError(QProcess::ProcessError);
    void openVpnVersionFinished(int, QProcess::ExitStatus);
    void certCheckTypeChanged(int);
    void proxyTypeChanged(int);

private:
    int compareVersion(const int x, const int y, const int z) const;
    void disableLegacySubjectMatch();
    void loadConfig();
    void fillOnePasswordCombo(PasswordField *passwordField, NetworkManager::Setting::SecretFlags type);
    void handleOnePasswordType(const PasswordField *passwordField, const QString &key, NMStringMap &data) const;
    Ui::OpenVpnAdvancedWidget *const m_ui;
    class Private;
    Private *const d;
};

#endif // PLASMA_NM_OPENVPN_ADVANCED_WIDGET_H
