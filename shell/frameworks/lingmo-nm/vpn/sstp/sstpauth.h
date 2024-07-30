/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SSTP_AUTH_H
#define SSTP_AUTH_H

#include "settingwidget.h"

#include <NetworkManagerQt/VpnSetting>

class SstpAuthWidgetPrivate;

class SstpAuthWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SstpAuthWidget)
public:
    explicit SstpAuthWidget(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent = nullptr);
    ~SstpAuthWidget() override;

    QVariantMap setting() const override;

private:
    SstpAuthWidgetPrivate *const d_ptr;
};

#endif // SSTP_AUTH_H
