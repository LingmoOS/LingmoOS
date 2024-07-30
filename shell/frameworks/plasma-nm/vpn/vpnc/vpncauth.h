/*
    SPDX-FileCopyrightText: 2008 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PLASMA_NM_VPNC_AUTH_H
#define PLASMA_NM_VPNC_AUTH_H

#include <NetworkManagerQt/VpnSetting>

#include "settingwidget.h"

class VpncAuthDialogPrivate;

class VpncAuthDialog : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(VpncAuthDialog)
public:
    explicit VpncAuthDialog(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~VpncAuthDialog() override;

    virtual void readSecrets();
    QVariantMap setting() const override;

private:
    VpncAuthDialogPrivate *const d_ptr;
};

#endif // PLASMA_NM_VPNC_AUTH_H
