/*
 *   SPDX-FileCopyrightText: 2010 Alejandro Fiestas Olivares <alex@eyeos.org>
 *   SPDX-FileCopyrightText: 2010 Eduardo Robles Elvira <edulix@gmail.com>
 *   SPDX-FileCopyrightText: 2010 UFO Coders <info@ufocoders.com>
 *   SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <BluezQt/Agent>

class BluezAgent : public BluezQt::Agent
{
    Q_OBJECT

public:
    explicit BluezAgent(QObject *parent);

    QDBusObjectPath objectPath() const override;

    void authorizeService(BluezQt::DevicePtr device, const QString &uuid, const BluezQt::Request<> &request) override;
    void requestPinCode(BluezQt::DevicePtr device, const BluezQt::Request<QString> &request) override;
    void requestPasskey(BluezQt::DevicePtr device, const BluezQt::Request<quint32> &request) override;
    void requestConfirmation(BluezQt::DevicePtr device, const QString &passkey, const BluezQt::Request<> &request) override;
    void requestAuthorization(BluezQt::DevicePtr device, const BluezQt::Request<> &request) override;
    void release() override;
    void cancel() override;

Q_SIGNALS:
    void agentReleased();
    void agentCanceled();
};
