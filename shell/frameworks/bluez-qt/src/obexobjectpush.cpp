/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexobjectpush.h"
#include "pendingcall.h"
#include "utils.h"

#include "obexobjectpush1.h"

namespace BluezQt
{
typedef org::bluez::obex::ObjectPush1 BluezObjectPush;

class ObexObjectPushPrivate
{
public:
    ObexObjectPush *q;
    BluezObjectPush *m_bluezObjectPush;
};

ObexObjectPush::ObexObjectPush(const QDBusObjectPath &path, QObject *parent)
    : QObject(parent)
    , d(new ObexObjectPushPrivate)
{
    d->m_bluezObjectPush = new BluezObjectPush(Strings::orgBluezObex(), path.path(), DBusConnection::orgBluezObex(), this);
}

ObexObjectPush::~ObexObjectPush() = default;

QDBusObjectPath ObexObjectPush::objectPath() const
{
    return QDBusObjectPath(d->m_bluezObjectPush->path());
}

PendingCall *ObexObjectPush::sendFile(const QString &fileName)
{
    return new PendingCall(d->m_bluezObjectPush->SendFile(fileName), PendingCall::ReturnTransferWithProperties, this);
}

PendingCall *ObexObjectPush::pullBusinessCard(const QString &targetFileName)
{
    return new PendingCall(d->m_bluezObjectPush->PullBusinessCard(targetFileName), PendingCall::ReturnTransferWithProperties, this);
}

PendingCall *ObexObjectPush::exchangeBusinessCards(const QString &clientFileName, const QString &targetFileName)
{
    return new PendingCall(d->m_bluezObjectPush->ExchangeBusinessCards(clientFileName, targetFileName), PendingCall::ReturnTransferWithProperties, this);
}

} // namespace BluezQt

#include "moc_obexobjectpush.cpp"
