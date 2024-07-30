/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_IODINE_H
#define PLASMA_NM_IODINE_H

#include "vpnuiplugin.h"

#include <QVariant>

class Q_DECL_EXPORT IodineUiPlugin : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit IodineUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~IodineUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;

    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
};

#endif //  PLASMA_NM_IODINE_H
