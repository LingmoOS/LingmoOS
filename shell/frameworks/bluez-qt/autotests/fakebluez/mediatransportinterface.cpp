/*
 * SPDX-FileCopyrightText: 2019 Manuel Weichselbaumer <mincequi@web.de>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "mediatransportinterface.h"
#include "objectmanager.h"

#include <QDBusConnection>
#include <QDBusMessage>

MediaTransportObject::MediaTransportObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

MediaTransportInterface::MediaTransportInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.MediaTransport1"));
}

quint8 MediaTransportInterface::codec() const
{
    return Object::property(QStringLiteral("Codec")).toUInt();
}

QByteArray MediaTransportInterface::configuration() const
{
    return Object::property(QStringLiteral("Configuration")).toByteArray();
}

QString MediaTransportInterface::state() const
{
    return Object::property(QStringLiteral("State")).toString();
}

quint16 MediaTransportInterface::volume() const
{
    return Object::property(QStringLiteral("Volume")).toUInt();
}

void MediaTransportInterface::Acquire(const QDBusMessage &msg)
{
}

void MediaTransportInterface::TryAcquire(const QDBusMessage &msg)
{
}

void MediaTransportInterface::Release()
{
}

#include "moc_mediatransportinterface.cpp"
