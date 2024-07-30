/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_IODINE_AUTH_H
#define PLASMA_NM_IODINE_AUTH_H

#include <NetworkManagerQt/VpnSetting>

#include "settingwidget.h"

class IodineAuthDialogPrivate;

class IodineAuthDialog : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(IodineAuthDialog)
public:
    explicit IodineAuthDialog(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~IodineAuthDialog() override;
    QVariantMap setting() const override;

private:
    IodineAuthDialogPrivate *const d_ptr;
};

#endif // PLASMA_NM_IODINE_AUTH_H
