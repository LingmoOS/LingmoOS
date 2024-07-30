/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SSH_AUTH_H
#define SSH_AUTH_H

#include "settingwidget.h"

#include <NetworkManagerQt/VpnSetting>

class SshAuthWidgetPrivate;

class SshAuthWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SshAuthWidget)
public:
    explicit SshAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~SshAuthWidget() override;

    QVariantMap setting() const override;

private:
    SshAuthWidgetPrivate *const d_ptr;
};

#endif // SSH_AUTH_H
