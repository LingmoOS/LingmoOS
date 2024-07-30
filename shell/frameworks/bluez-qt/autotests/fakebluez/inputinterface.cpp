/*
 * SPDX-FileCopyrightText: 2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "inputinterface.h"

#include <QDBusConnection>

// InputObject
InputObject::InputObject(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
{
    QDBusConnection::sessionBus().registerObject(path.path(), this);
}

// InputInterface
InputInterface::InputInterface(const QDBusObjectPath &path, const QVariantMap &properties, QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setPath(path);
    setObjectParent(parent);
    setProperties(properties);
    setName(QStringLiteral("org.bluez.Input1"));
}

QString InputInterface::reconnectMode() const
{
    return Object::property(QStringLiteral("ReconnectMode")).toString();
}

#include "moc_inputinterface.cpp"
