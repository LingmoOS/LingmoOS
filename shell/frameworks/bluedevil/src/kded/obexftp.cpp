/*
 *  SPDX-FileCopyrightText: 2013 Alejandro Fiestas Fiestas <afiestas@kde.org>
 *  SPDX-FileCopyrightText: 2014-2015 David Rosca <nowrep@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "obexftp.h"
#include "bluedevil_kded.h"
#include "bluedevildaemon.h"

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

#include <KLocalizedString>

#include <BluezQt/Device>
#include <BluezQt/InitObexManagerJob>
#include <BluezQt/ObexFileTransfer>
#include <BluezQt/ObexManager>
#include <BluezQt/ObexSession>
#include <BluezQt/PendingCall>

ObexFtp::ObexFtp(BlueDevilDaemon *daemon)
    : QDBusAbstractAdaptor(daemon)
    , m_daemon(daemon)
{
    connect(m_daemon->obexManager(), &BluezQt::ObexManager::sessionRemoved, this, &ObexFtp::sessionRemoved);
}

bool ObexFtp::isOnline()
{
    return m_daemon->obexManager()->isOperational();
}

QString ObexFtp::preferredTarget(const QString &address)
{
    BluezQt::DevicePtr device = m_daemon->manager()->deviceForAddress(address);

    // Prefer pcsuite target on S60 devices
    if (device && device->uuids().contains(QStringLiteral("00005005-0000-1000-8000-0002EE000001"))) {
        return QStringLiteral("pcsuite");
    }
    return QStringLiteral("ftp");
}

QString ObexFtp::session(const QString &address, const QString &target, const QDBusMessage &msg)
{
    if (!m_daemon->obexManager()->isOperational()) {
        return QString();
    }

    if (m_sessionMap.contains(address)) {
        return m_sessionMap[address];
    }

    qCDebug(BLUEDEVIL_KDED_LOG) << "Creating obexftp session for" << address;

    // At this point we always want delayed reply
    msg.setDelayedReply(true);

    if (m_pendingSessions.contains(address)) {
        m_pendingSessions[address].append(msg);
        return QString();
    }

    m_pendingSessions.insert(address, {msg});

    QVariantMap args;
    args[QStringLiteral("Target")] = target;

    BluezQt::PendingCall *call = m_daemon->obexManager()->createSession(address, args);
    call->setUserData(address);
    connect(call, &BluezQt::PendingCall::finished, this, &ObexFtp::createSessionFinished);

    return QString();
}

bool ObexFtp::cancelTransfer(const QString &transfer, const QDBusMessage &msg)
{
    // We need this function because kio_obexftp is not owner of the transfer,
    // and thus cannot cancel it.

    msg.setDelayedReply(true);

    QDBusMessage call = QDBusMessage::createMethodCall(QStringLiteral("org.bluez.obex"), //
                                                       transfer,
                                                       QStringLiteral("org.bluez.obex.Transfer1"),
                                                       QStringLiteral("Cancel"));

    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(call));
    watcher->setProperty("ObexFtpDaemon-msg", QVariant::fromValue(msg));
    connect(watcher, &QDBusPendingCallWatcher::finished, this, &ObexFtp::cancelTransferFinished);

    return false;
}

void ObexFtp::createSessionFinished(BluezQt::PendingCall *call)
{
    QString path;

    if (call->error() == BluezQt::PendingCall::AlreadyExists) {
        // It may happen when kded crashes, or the session was created by different app
        // What to do here? We are not owners of the session...
        qCWarning(BLUEDEVIL_KDED_LOG) << "Obex session already exists but it was created by different process!";
    } else if (call->error()) {
        qCWarning(BLUEDEVIL_KDED_LOG) << "Error creating Obex session" << call->errorText();
    } else {
        path = call->value().value<QDBusObjectPath>().path();
        qCDebug(BLUEDEVIL_KDED_LOG) << "Created Obex session" << path;
    }

    const QString &address = call->userData().toString();

    // Send reply (empty session path in case of error)
    Q_FOREACH (const QDBusMessage &msg, m_pendingSessions[address]) {
        QDBusMessage reply = msg.createReply(path);
        QDBusConnection::sessionBus().send(reply);
    }

    m_pendingSessions.remove(address);

    if (!call->error()) {
        m_sessionMap.insert(address, path);
    }
}

void ObexFtp::cancelTransferFinished(QDBusPendingCallWatcher *watcher)
{
    const QDBusPendingReply<> &reply = *watcher;
    QDBusMessage msg = watcher->property("ObexFtpDaemon-msg").value<QDBusMessage>();

    bool success = !reply.isError();
    QDBusConnection::sessionBus().send(msg.createReply(QVariant(success)));
}

void ObexFtp::sessionRemoved(BluezQt::ObexSessionPtr session)
{
    const QString &path = session->objectPath().path();
    const QString &key = m_sessionMap.key(path);

    if (!m_sessionMap.contains(key)) {
        qCDebug(BLUEDEVIL_KDED_LOG) << "Removed Obex session is not ours" << path;
        return;
    }

    qCDebug(BLUEDEVIL_KDED_LOG) << "Removed Obex session" << path;
    m_sessionMap.remove(key);
}

#include "moc_obexftp.cpp"
