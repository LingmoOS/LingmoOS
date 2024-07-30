/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fdacpluggedjob.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>

using namespace Solid;

FDAcPluggedJob::FDAcPluggedJob(QObject *parent)
    : AbstractAcPluggedJob(parent)
    , m_isPlugged(false)
{
}

void FDAcPluggedJob::doStart()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.UPower"),
                                                      QStringLiteral("/org/freedesktop/UPower"),
                                                      QStringLiteral("org.freedesktop.DBus.Properties"),
                                                      QStringLiteral("Get"));

    msg << QStringLiteral("org.freedesktop.UPower");
    msg << QStringLiteral("OnBattery");

    QDBusConnection::systemBus().callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

void FDAcPluggedJob::slotDBusReply(const QDBusMessage &msg)
{
    Q_ASSERT(!msg.arguments().isEmpty());

    m_isPlugged = !msg.arguments().first().value<QDBusVariant>().variant().toBool();
    emitResult();
}

void FDAcPluggedJob::slotDBusError(const QDBusError &error)
{
    setError(error.type());
    setErrorText(error.message());
    emitResult();
}

bool FDAcPluggedJob::isPlugged() const
{
    return m_isPlugged;
}

#include "moc_fdacpluggedjob.cpp"
