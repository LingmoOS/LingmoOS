/*
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexclient.h"

#include <QDBusMessage>

ObexClient::ObexClient(QObject *parent)
    : QDBusAbstractAdaptor(parent)
{
    setName(QStringLiteral("org.bluez.obex.Client1"));
    setPath(QDBusObjectPath(QStringLiteral("/org/bluez/obex")));
}

void ObexClient::runAction(const QString &actionName, const QVariantMap &properties)
{
    Q_UNUSED(actionName)
    Q_UNUSED(properties)
}

void ObexClient::CreateSession(const QString &destination, const QVariantMap &args, const QDBusMessage &msg)
{
    Q_UNUSED(destination)
    Q_UNUSED(args)
    Q_UNUSED(msg)
}

void ObexClient::RemoveSession(const QDBusObjectPath &session, const QDBusMessage &msg)
{
    Q_UNUSED(session)
    Q_UNUSED(msg)
}

#include "moc_obexclient.cpp"
