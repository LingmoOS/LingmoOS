/*
 * BluezQt - Asynchronous Bluez wrapper library
 *
 * SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

#include "obexagentadaptor.h"
#include "dbusproperties.h"
#include "obexagent.h"
#include "obexmanager.h"
#include "obextransfer.h"
#include "obextransfer_p.h"
#include "utils.h"

#include <QDBusObjectPath>

namespace BluezQt
{
typedef org::freedesktop::DBus::Properties DBusProperties;

ObexAgentAdaptor::ObexAgentAdaptor(ObexAgent *parent, ObexManager *manager)
    : QDBusAbstractAdaptor(parent)
    , m_agent(parent)
    , m_manager(manager)
{
}

QString ObexAgentAdaptor::AuthorizePush(const QDBusObjectPath &transfer, const QDBusMessage &msg)
{
    msg.setDelayedReply(true);
    m_transferRequest = Request<QString>(OrgBluezObexAgent, msg);
    m_transferPath = transfer.path();

    DBusProperties dbusProperties(Strings::orgBluezObex(), m_transferPath, DBusConnection::orgBluezObex(), this);

    const QDBusPendingReply<QVariantMap> &call = dbusProperties.GetAll(Strings::orgBluezObexTransfer1());
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexAgentAdaptor::getPropertiesFinished);

    return QString();
}

void ObexAgentAdaptor::Cancel()
{
    m_agent->cancel();
}

void ObexAgentAdaptor::Release()
{
    m_agent->release();
}

void ObexAgentAdaptor::getPropertiesFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<QVariantMap> &reply = *watcher;
    watcher->deleteLater();

    if (reply.isError()) {
        m_transferRequest.cancel();
        return;
    }

    ObexTransferPtr transfer = ObexTransferPtr(new ObexTransfer(m_transferPath, reply.value()));
    transfer->d->q = transfer.toWeakRef();

    ObexSessionPtr session = m_manager->sessionForPath(transfer->objectPath());
    Q_ASSERT(session);

    if (!session) {
        m_transferRequest.cancel();
        return;
    }

    m_agent->authorizePush(transfer, session, m_transferRequest);
}

} // namespace BluezQt

#include "moc_obexagentadaptor.cpp"
