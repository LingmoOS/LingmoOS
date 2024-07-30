/*
    SPDX-FileCopyrightText: 2009 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Maurus Rohrer <maurus.rohrer@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef STRONGSWANWIDGET_H
#define STRONGSWANWIDGET_H

#include "passwordfield.h"
#include "settingwidget.h"

#include <NetworkManagerQt/VpnSetting>

class StrongswanSettingWidgetPrivate;

class StrongswanSettingWidget : public SettingWidget
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StrongswanSettingWidget)
public:
    explicit StrongswanSettingWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);
    ~StrongswanSettingWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private:
    StrongswanSettingWidgetPrivate *const d_ptr;
};

#endif // STRONGSWANWIDGET_H
