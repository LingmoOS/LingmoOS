/*
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemsimple_p.h"

#include "mmdebug_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::ModemSimplePrivate::ModemSimplePrivate(const QString &path, ModemSimple *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modemSimpleIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modemSimpleIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
}

ModemManager::ModemSimple::ModemSimple(const QString &path, QObject *parent)
    : Interface(*new ModemSimplePrivate(path, this), parent)
{
}

ModemManager::ModemSimple::~ModemSimple()
{
}

QString ModemManager::ModemSimple::uni() const
{
    Q_D(const ModemSimple);
    return d->uni;
}

QDBusPendingReply<QDBusObjectPath> ModemManager::ModemSimple::connectModem(const QVariantMap &properties)
{
    Q_D(ModemSimple);

    return d->modemSimpleIface.Connect(properties);
}

QDBusPendingReply<QVariantMap> ModemManager::ModemSimple::getStatus()
{
    Q_D(ModemSimple);
    return d->modemSimpleIface.GetStatus();
}

QDBusPendingReply<void> ModemManager::ModemSimple::disconnectModem(const QString &bearer)
{
    Q_D(ModemSimple);
    return d->modemSimpleIface.Disconnect(QDBusObjectPath(bearer));
}

QDBusPendingReply<void> ModemManager::ModemSimple::disconnectAllModems()
{
    return disconnectModem(QStringLiteral("/"));
}

void ModemManager::ModemSimple::setTimeout(int timeout)
{
    Q_D(ModemSimple);
    d->modemSimpleIface.setTimeout(timeout);
}

int ModemManager::ModemSimple::timeout() const
{
    Q_D(const ModemSimple);
    return d->modemSimpleIface.timeout();
}

#include "moc_modemsimple.cpp"
#include "moc_modemsimple_p.cpp"
