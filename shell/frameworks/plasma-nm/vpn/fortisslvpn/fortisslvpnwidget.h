/*
    SPDX-FileCopyrightText: 2017 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_FORTISSLVPN_WIDGET_H
#define PLASMA_NM_FORTISSLVPN_WIDGET_H

#include <NetworkManagerQt/VpnSetting>

#include "settingwidget.h"

class FortisslvpnWidgetPrivate;

class FortisslvpnWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(FortisslvpnWidget)
public:
    explicit FortisslvpnWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~FortisslvpnWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;
    QVariantMap setting() const override;
    bool isValid() const override;

private Q_SLOTS:
    void showAdvanced();

private:
    FortisslvpnWidgetPrivate *const d_ptr;
};

#endif // PLASMA_NM_FORTISSLVPN_WIDGET_H
