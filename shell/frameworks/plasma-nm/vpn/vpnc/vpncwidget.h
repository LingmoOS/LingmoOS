/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_VPNC_WIDGET_H
#define PLASMA_NM_VPNC_WIDGET_H

#include <NetworkManagerQt/VpnSetting>

#include <QPointer>

#include "settingwidget.h"
#include "vpncadvancedwidget.h"

namespace Ui
{
class VpncWidget;
}

class VpncWidget : public SettingWidget
{
    Q_OBJECT
public:
    explicit VpncWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~VpncWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void userPasswordTypeChanged(int index);
    void groupPasswordTypeChanged(int index);
    void showAdvanced();

private:
    Ui::VpncWidget *const m_ui;
    NetworkManager::VpnSetting::Ptr m_setting;
    NetworkManager::VpnSetting::Ptr m_tmpSetting;
    QPointer<VpncAdvancedWidget> m_advancedWidget;
};

#endif // PLASMA_NM_VPNC_WIDGET_H
