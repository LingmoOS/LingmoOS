/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_PPTP_H
#define PLASMA_NM_PPTP_H

#include "vpnuiplugin.h"

#include <QVariant>

class Q_DECL_EXPORT PptpUiPlugin : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit PptpUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~PptpUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;

    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
};

#endif //  PLASMA_NM_PPTP_H
