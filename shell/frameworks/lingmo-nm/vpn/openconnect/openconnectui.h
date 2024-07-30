/*
    SPDX-FileCopyrightText: 2011 Ilia Kats <ilia-kats@gmx.net>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef OPENCONNECT_UI_H
#define OPENCONNECT_UI_H

#include "vpnuiplugin.h"

#include <QDialog>
#include <QVariant>

class Q_DECL_EXPORT OpenconnectUiPlugin : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit OpenconnectUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~OpenconnectUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;

    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
    QMessageBox::StandardButtons suggestedAuthDialogButtons() const override;
};

#endif //  OPENCONNECT_UI_H
