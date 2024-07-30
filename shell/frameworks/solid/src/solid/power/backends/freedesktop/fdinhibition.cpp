/*
    SPDX-FileCopyrightText: 2014 Alejandro Fiestas Olivares <afiestas@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fdinhibition.h"
#include "logindinhibitionargument.h"

#include <unistd.h>

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusUnixFileDescriptor>
#include <QDebug>
#include <QVariant>

using namespace Solid;

FdInhibition::FdInhibition(Power::InhibitionTypes inhibitions, const QString &description, QObject *parent)
    : AbstractInhibition(parent)
    , m_state(Inhibition::Stopped)
    , m_description(description)
    , m_inhibitions(inhibitions)
{
}

FdInhibition::~FdInhibition()
{
    close(m_fd);
}

void FdInhibition::start()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.login1"),
                                                      QStringLiteral("/org/freedesktop/login1"),
                                                      QStringLiteral("org.freedesktop.login1.Manager"),
                                                      QStringLiteral("Inhibit"));

    QList<QVariant> args;
    args << LogindInhibitionArgument::fromPowerState(m_inhibitions) << QCoreApplication::instance()->applicationName() << m_description
         << QStringLiteral("block");
    msg.setArguments(args);

    QDBusConnection::systemBus().callWithCallback(msg, this, SLOT(slotDBusReply(QDBusMessage)), SLOT(slotDBusError(QDBusError)));
}

void FdInhibition::stop()
{
    close(m_fd);
    setState(Inhibition::Stopped);
}

void FdInhibition::setDescription(const QString &description)
{
    m_description = description;
}

void FdInhibition::slotDBusReply(const QDBusMessage &msg)
{
    m_fd = msg.arguments().first().value<QDBusUnixFileDescriptor>().fileDescriptor();
    setState(Inhibition::Started);
}

void FdInhibition::slotDBusError(const QDBusError &error)
{
    qDebug() << error.message();
}

void FdInhibition::setState(const Inhibition::State &state)
{
    if (m_state == state) {
        return;
    }

    m_state = state;
    Q_EMIT stateChanged(m_state);
}

Inhibition::State FdInhibition::state() const
{
    return m_state;
}

#include "moc_fdinhibition.cpp"
