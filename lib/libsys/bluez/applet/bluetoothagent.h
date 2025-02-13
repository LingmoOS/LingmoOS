/*
 *   SPDX-FileCopyrightText: 2010 Alex Fiestas <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef BLUETOOTHAGENT_H
#define BLUETOOTHAGENT_H

#include <BluezQt/Agent>

class BluetoothAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    explicit BluetoothAgent(QObject *parent = nullptr);

    QString pin();
    void setPin(const QString &pin);

    bool isFromDatabase();
    QString getPin(BluezQt::DevicePtr device);

    QDBusObjectPath objectPath() const override;

    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &req) override;
    void displayPinCode(BluezQt::DevicePtr device, const QString &pinCode) override;
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &req) override;
    void displayPasskey(BluezQt::DevicePtr device, const QString &passkey, const QString &entered) override;
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &req) override;

Q_SIGNALS:
    void pinRequested(const QString &pin);
    void confirmationRequested(const QString &passkey, const BluezQt::Request<> &req);

private:
    bool m_fromDatabase;
    QString m_pin;
};

#endif // BluetoothAgent_H
