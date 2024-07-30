/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexsession.h"
#include "obexsession_p.h"
#include "pendingcall.h"
#include "utils.h"

namespace BluezQt
{
ObexSessionPrivate::ObexSessionPrivate(const QString &path, const QVariantMap &properties)
    : QObject()
{
    m_bluezSession = new BluezSession(Strings::orgBluezObex(), path, DBusConnection::orgBluezObex(), this);

    init(properties);
}

void ObexSessionPrivate::init(const QVariantMap &properties)
{
    m_source = properties.value(QStringLiteral("Source")).toString();
    m_destination = properties.value(QStringLiteral("Destination")).toString();
    m_channel = properties.value(QStringLiteral("Channel")).toUInt();
    m_target = properties.value(QStringLiteral("Target")).toString().toUpper();
    m_root = properties.value(QStringLiteral("Root")).toString();
}

ObexSession::ObexSession(const QString &path, const QVariantMap &properties)
    : QObject()
    , d(new ObexSessionPrivate(path, properties))
{
}

ObexSession::~ObexSession() = default;

ObexSessionPtr ObexSession::toSharedPtr() const
{
    return d->q.toStrongRef();
}

QDBusObjectPath ObexSession::objectPath() const
{
    return QDBusObjectPath(d->m_bluezSession->path());
}

QString ObexSession::source() const
{
    return d->m_source;
}

QString ObexSession::destination() const
{
    return d->m_destination;
}

quint8 ObexSession::channel() const
{
    return d->m_channel;
}

QString ObexSession::target() const
{
    return d->m_target;
}

QString ObexSession::root() const
{
    return d->m_root;
}

PendingCall *ObexSession::getCapabilities()
{
    return new PendingCall(d->m_bluezSession->GetCapabilities(), PendingCall::ReturnString, this);
}

} // namespace BluezQt

#include "moc_obexsession.cpp"
#include "moc_obexsession_p.cpp"
