/*
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef LINGMO_NM_WIFI_CONNECTION_WIDGET_H
#define LINGMO_NM_WIFI_CONNECTION_WIDGET_H

#include "lingmonm_editor_export.h"

#include <QWidget>

#include <NetworkManagerQt/WirelessSetting>

#include "settingwidget.h"

namespace Ui
{
class WifiConnectionWidget;
}

class LINGMONM_EDITOR_EXPORT WifiConnectionWidget : public SettingWidget
{
    Q_OBJECT

public:
    explicit WifiConnectionWidget(const NetworkManager::Setting::Ptr &setting = NetworkManager::Setting::Ptr(),
                                  QWidget *parent = nullptr,
                                  Qt::WindowFlags f = {});
    ~WifiConnectionWidget() override;

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;

    QVariantMap setting() const override;

    bool isValid() const override;

Q_SIGNALS:
    void ssidChanged(const QString &ssid);

private Q_SLOTS:
    void generateRandomClonedMac();
    void ssidChanged();
    void modeChanged(int mode);
    void bandChanged(int band);

private:
    Ui::WifiConnectionWidget *const m_ui;
    void fillChannels(NetworkManager::WirelessSetting::FrequencyBand band);
};

#endif // LINGMO_NM_WIFI_CONNECTION_WIDGET_H
