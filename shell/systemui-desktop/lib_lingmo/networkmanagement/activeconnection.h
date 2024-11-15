/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     lingmoos <lingmo@lingmo.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIVECONNECTION_H
#define ACTIVECONNECTION_H

#include <QObject>

#include <NetworkManagerQt/ActiveConnection>
#include <NetworkManagerQt/Connection>
#include <NetworkManagerQt/Settings>
#include <NetworkManagerQt/WirelessDevice>

class ActiveConnection : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString wirelessName READ wirelessName NOTIFY wirelessNameChanged)
    Q_PROPERTY(QString wirelessIcon READ wirelessIcon NOTIFY wirelessIconChanged)

public:
    explicit ActiveConnection(QObject *parent = nullptr);

    QString wirelessName() { return m_wirelessName; }
    QString wirelessIcon() { return m_wirelessIcon; }

signals:
    void wirelessNameChanged();
    void wirelessIconChanged();

private slots:
    void statusChanged(NetworkManager::Status status);
    void updateWirelessIcon(NetworkManager::Device::Ptr device);
    void updateWirelessIconForSignalStrength(int strength);

private:
    QString m_wirelessName;
    QString m_wirelessIcon;

    NetworkManager::WirelessNetwork::Ptr m_wirelessNetwork;
};

#endif // ACTIVECONNECTION_H
