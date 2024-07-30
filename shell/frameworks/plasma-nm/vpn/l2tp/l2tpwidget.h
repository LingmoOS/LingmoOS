/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>
    SPDX-FileCopyrightText: 2020 Douglas Kosovic <doug@uq.edu.au>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_L2TP_WIDGET_H
#define PLASMA_NM_L2TP_WIDGET_H

#include <NetworkManagerQt/VpnSetting>

#include "settingwidget.h"

namespace Ui
{
class L2tpWidget;
}

class L2tpWidget : public SettingWidget
{
    Q_OBJECT

    enum AuthType { Password = 0, TLS };

public:
    explicit L2tpWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr, Qt::WindowFlags f = {});
    ~L2tpWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

private Q_SLOTS:
    void updateStartDirUrl(const QUrl &);
    void showIpsec();
    void showPpp();

private:
    Ui::L2tpWidget *const m_ui;
    NetworkManager::VpnSetting::Ptr m_setting;
    NetworkManager::VpnSetting::Ptr m_tmpIpsecSetting;
    NetworkManager::VpnSetting::Ptr m_tmpPppSetting;
};

#endif // PLASMA_NM_L2TP_WIDGET_H
