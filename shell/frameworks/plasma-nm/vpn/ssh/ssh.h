/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_SSH_H
#define PLASMA_NM_SSH_H

#include "vpnuiplugin.h"

class Q_DECL_EXPORT SshUiPlugin : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit SshUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~SshUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;
    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
};

#endif //  PLASMA_NM_SSH_H
