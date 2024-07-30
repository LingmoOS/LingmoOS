/*
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef PLASMA_NM_CONNECTION_WIDGET_H
#define PLASMA_NM_CONNECTION_WIDGET_H

#include <QWidget>

#include <NetworkManagerQt/ConnectionSettings>

namespace Ui
{
class ConnectionWidget;
}

class ConnectionWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ConnectionWidget(const NetworkManager::ConnectionSettings::Ptr &settings = NetworkManager::ConnectionSettings::Ptr(),
                              QWidget *parent = nullptr,
                              Qt::WindowFlags f = {});
    ~ConnectionWidget() override;

    void loadConfig(const NetworkManager::ConnectionSettings::Ptr &settings);

    NMVariantMapMap setting() const;

    bool allUsers() const;

private Q_SLOTS:
    void autoVpnToggled(bool on);
    void openAdvancedPermissions();

Q_SIGNALS:
    void settingChanged();
    void allUsersChanged();

private:
    // list of VPN: UUID, name
    NMStringMap vpnConnections() const;
    // list of firewalld zones
    QStringList firewallZones() const;

    void populateVpnConnections();
    Ui::ConnectionWidget *const m_widget;
    NetworkManager::ConnectionSettings m_tmpSetting;
    NetworkManager::ConnectionSettings::ConnectionType m_type;
    QString m_masterUuid;
    QString m_slaveType;
};

#endif // PLASMA_NM_CONNECTION_WIDGET_H
