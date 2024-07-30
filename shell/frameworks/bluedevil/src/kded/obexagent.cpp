/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "obexagent.h"
#include "bluedevil_kded.h"
#include "bluedevildaemon.h"
#include "filereceiversettings.h"
#include "receivefilejob.h"

#include <QDBusObjectPath>

ObexAgent::ObexAgent(BlueDevilDaemon *daemon)
    : BluezQt::ObexAgent(daemon)
    , m_manager(daemon->manager())
{
}

BluezQt::Manager *ObexAgent::manager() const
{
    return m_manager;
}

bool ObexAgent::shouldAutoAcceptTransfer(const QString &address) const
{
    if (!m_transferTimes.contains(address)) {
        return false;
    }

    // Auto-accept transfers from the same device within 2 seconds from last finished transfer
    const int timeout = 2;
    return m_transferTimes.value(address).secsTo(QDateTime::currentDateTime()) < timeout;
}

QDBusObjectPath ObexAgent::objectPath() const
{
    return QDBusObjectPath(QStringLiteral("/modules/bluedevil/ObexAgent"));
}

void ObexAgent::authorizePush(BluezQt::ObexTransferPtr transfer, BluezQt::ObexSessionPtr session, const BluezQt::Request<QString> &request)
{
    qCDebug(BLUEDEVIL_KDED_LOG) << "ObexAgent-AuthorizePush";

    FileReceiverSettings::self()->load();

    ReceiveFileJob *job = new ReceiveFileJob(request, transfer, session, this);
    connect(job, &ReceiveFileJob::finished, this, &ObexAgent::receiveFileJobFinished);
    job->start();
}

void ObexAgent::receiveFileJobFinished(KJob *job)
{
    Q_ASSERT(qobject_cast<ReceiveFileJob *>(job));
    ReceiveFileJob *j = static_cast<ReceiveFileJob *>(job);

    if (j->error()) {
        m_transferTimes.remove(j->deviceAddress());
        return;
    }

    m_transferTimes[j->deviceAddress()] = QDateTime::currentDateTime();
}

#include "moc_obexagent.cpp"
