/*
 * Copyright (C) 2021 LingmoOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <BluezQt/Manager>
#include "bluetoothagent.h"

class BluetoothManager : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    Q_INVOKABLE void setName(const QString &name);
    Q_INVOKABLE void connectToDevice(const QString address);
    Q_INVOKABLE void requestParingConnection(const QString address);
    Q_INVOKABLE void confirmMatchButton(const bool match);
    Q_INVOKABLE void deviceDisconnect(const QString address);
    Q_INVOKABLE void deviceRemoved(const QString address);
    Q_INVOKABLE void stopMediaPlayer(const QString address);

signals:
    void showPairDialog(const QString name, const QString pin);
    void pairFailed(const QString name, const int deviceType);
    void connectFailed(const QString name, const int deviceType);

private slots:
    void onInitJobResult(BluezQt::InitManagerJob *job);
    void operationalChanged(bool operational);
    void confirmationRequested(const QString &passkey, const BluezQt::Request<> &req);
    void pairingFinished(BluezQt::PendingCall *call);
    void connectFinished(BluezQt::PendingCall *call);

private:
    BluezQt::Manager *m_manager;
    BluetoothAgent *m_agent;
    BluezQt::AdapterPtr m_adapter;
    BluezQt::DevicePtr m_device;
    BluezQt::Request<> m_req;
    QString m_name;
};

#endif // BLUETOOTHMANAGER_H
