/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "profile.h"
#include "debug.h"
#include "profile_p.h"

#include <unistd.h>

#include <QDBusUnixFileDescriptor>
#include <QLocalSocket>

namespace BluezQt
{
Profile::Profile(QObject *parent)
    : QObject(parent)
    , d(new ProfilePrivate)
{
}

Profile::~Profile() = default;

void Profile::setName(const QString &name)
{
    d->options[QStringLiteral("Name")] = name;
}

void Profile::setService(const QString &service)
{
    d->options[QStringLiteral("Service")] = service;
}

void Profile::setLocalRole(Profile::LocalRole role)
{
    QString str;
    switch (role) {
    case ClientRole:
        str = QStringLiteral("client");
        break;

    case ServerRole:
        str = QStringLiteral("server");
        break;

    default:
        break;
    }

    if (str.isEmpty()) {
        qCWarning(BLUEZQT) << "Invalid local role!";
        return;
    }

    d->options[QStringLiteral("Role")] = str;
}

void Profile::setChannel(quint16 channel)
{
    if (channel > 31) {
        qCWarning(BLUEZQT) << "Invalid channel number. Must be 0-31!";
        return;
    }

    d->options[QStringLiteral("Channel")] = QVariant::fromValue(channel);
}

void Profile::setPsm(quint16 psm)
{
    d->options[QStringLiteral("PSM")] = QVariant::fromValue(psm);
}

void Profile::setRequireAuthentication(bool require)
{
    d->options[QStringLiteral("RequireAuthentication")] = require;
}

void Profile::setRequireAuthorization(bool require)
{
    d->options[QStringLiteral("RequireAuthorization")] = require;
}

void Profile::setAutoConnect(bool autoConnect)
{
    d->options[QStringLiteral("AutoConnect")] = autoConnect;
}

void Profile::setServiceRecord(const QString &serviceRecord)
{
    d->options[QStringLiteral("ServiceRecord")] = serviceRecord;
}

void Profile::setVersion(quint16 version)
{
    d->options[QStringLiteral("Version")] = QVariant::fromValue(version);
}

void Profile::setFeatures(quint16 features)
{
    d->options[QStringLiteral("Features")] = QVariant::fromValue(features);
}

QSharedPointer<QLocalSocket> Profile::createSocket(const QDBusUnixFileDescriptor &fd)
{
    int newfd = ::dup(fd.fileDescriptor());
    QSharedPointer<QLocalSocket> socket(new QLocalSocket);
    socket->setSocketDescriptor(newfd);
    return socket;
}

void Profile::newConnection(DevicePtr device, const QDBusUnixFileDescriptor &fd, const QVariantMap &properties, const Request<> &request)
{
    Q_UNUSED(device)
    Q_UNUSED(fd)
    Q_UNUSED(properties)

    request.cancel();
}

void Profile::requestDisconnection(DevicePtr device, const Request<> &request)
{
    Q_UNUSED(device)

    request.cancel();
}

void Profile::release()
{
}

} // namespace BluezQt

#include "moc_profile.cpp"
