/*  This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Kevin Ottens <ervin@kde.org>
    SPDX-FileCopyrightText: 2008-2010 Dario Freddi <drf@kde.org>
    SPDX-FileCopyrightText: 2010 Alejandro Fiestas <alex@eyeos.org>
    SPDX-FileCopyrightText: 2010-2013 Lukáš Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2023 Nicolas Fella <nicolas.fella@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only

*/

#include "suspendcontroller.h"

#include <QDBusConnection>
#include <QDBusConnectionInterface>

using namespace Qt::StringLiterals;

inline constexpr QLatin1StringView LOGIN1_SERVICE("org.freedesktop.login1");
inline constexpr QLatin1StringView CONSOLEKIT2_SERVICE("org.freedesktop.ConsoleKit");

SuspendController::SuspendController()
    : QObject()
{
    // interfaces
    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(LOGIN1_SERVICE)) {
        // Activate it.
        QDBusConnection::systemBus().interface()->startService(LOGIN1_SERVICE);
    }

    if (!QDBusConnection::systemBus().interface()->isServiceRegistered(CONSOLEKIT2_SERVICE)) {
        // Activate it.
        QDBusConnection::systemBus().interface()->startService(CONSOLEKIT2_SERVICE);
    }

    if (QDBusConnection::systemBus().interface()->isServiceRegistered(LOGIN1_SERVICE)) {
        m_login1Interface =
            new QDBusInterface(LOGIN1_SERVICE, u"/org/freedesktop/login1"_s, u"org.freedesktop.login1.Manager"_s, QDBusConnection::systemBus(), this);
    }

    // if login1 isn't available, try using the same interface with ConsoleKit2
    if (!m_login1Interface && QDBusConnection::systemBus().interface()->isServiceRegistered(CONSOLEKIT2_SERVICE)) {
        m_login1Interface = new QDBusInterface(CONSOLEKIT2_SERVICE,
                                               u"/org/freedesktop/ConsoleKit/Manager"_s,
                                               u"org.freedesktop.ConsoleKit.Manager"_s,
                                               QDBusConnection::systemBus(),
                                               this);
    }

    // "resuming" signal
    if (m_login1Interface) {
        connect(m_login1Interface.data(), SIGNAL(PrepareForSleep(bool)), this, SLOT(slotLogin1PrepareForSleep(bool)));
    }
}

bool SuspendController::canSuspend() const
{
    return m_sessionManagement.canSuspend();
}

void SuspendController::suspend()
{
    m_sessionManagement.suspend();
}

bool SuspendController::canHibernate() const
{
    return m_sessionManagement.canHibernate();
}

void SuspendController::hibernate()
{
    m_sessionManagement.hibernate();
}

bool SuspendController::canHybridSuspend() const
{
    return m_sessionManagement.canHybridSuspend();
}

void SuspendController::hybridSuspend()
{
    m_sessionManagement.hybridSuspend();
}

bool SuspendController::canSuspendThenHibernate() const
{
    return m_sessionManagement.canSuspendThenHibernate();
}

void SuspendController::suspendThenHibernate()
{
    m_sessionManagement.suspendThenHibernate();
}

void SuspendController::slotLogin1PrepareForSleep(bool active)
{
    if (active) {
        Q_EMIT aboutToSuspend();
    } else {
        Q_EMIT resumeFromSuspend();
    }
}

#include "moc_suspendcontroller.cpp"