/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "agent.h"

namespace BluezQt
{
Agent::Agent(QObject *parent)
    : QObject(parent)
{
}

Agent::Capability Agent::capability() const
{
    return DisplayYesNo;
}

void Agent::requestPinCode(DevicePtr device, const Request<QString> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Agent::displayPinCode(DevicePtr device, const QString &pinCode)
{
    Q_UNUSED(device)
    Q_UNUSED(pinCode)
}

void Agent::requestPasskey(DevicePtr device, const Request<quint32> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Agent::displayPasskey(DevicePtr device, const QString &passkey, const QString &entered)
{
    Q_UNUSED(device)
    Q_UNUSED(passkey)
    Q_UNUSED(entered)
}

void Agent::requestConfirmation(DevicePtr device, const QString &passkey, const Request<> &request)
{
    Q_UNUSED(device)
    Q_UNUSED(passkey)

    request.cancel();
}

void Agent::requestAuthorization(DevicePtr device, const Request<> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Agent::authorizeService(DevicePtr device, const QString &uuid, const Request<> &request)
{
    Q_UNUSED(device)
    Q_UNUSED(uuid)

    request.cancel();
}

void Agent::cancel()
{
}

void Agent::release()
{
}

} // namespace BluezQt

#include "moc_agent.cpp"
