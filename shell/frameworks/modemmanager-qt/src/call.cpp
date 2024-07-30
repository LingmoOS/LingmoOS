/*
    SPDX-FileCopyrightText: 2018 Aleksander Morgado <aleksander@aleksander.es>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "call.h"
#include "call_p.h"
#include "mmdebug_p.h"

#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

#include <ModemManager/ModemManager.h>

ModemManager::CallPrivate::CallPrivate(const QString &path, Call *q)
#ifdef MMQT_STATIC
    : callIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    : callIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (callIface.isValid()) {
        uni = path;
        state = (MMCallState)callIface.state();
        stateReason = (MMCallStateReason)callIface.stateReason();
        direction = (MMCallDirection)callIface.direction();
        number = callIface.number();
    }
}

ModemManager::Call::Call(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new CallPrivate(path, this))
{
    Q_D(Call);

    qRegisterMetaType<MMCallState>();
    qRegisterMetaType<MMCallStateReason>();
    qRegisterMetaType<MMCallDirection>();

#ifdef MMQT_STATIC
    QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                          path,
                                          QLatin1String(DBUS_INTERFACE_PROPS),
                                          QStringLiteral("PropertiesChanged"),
                                          d,
                                          SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#else
    QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                         path,
                                         QLatin1String(DBUS_INTERFACE_PROPS),
                                         QStringLiteral("PropertiesChanged"),
                                         d,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#endif

    connect(&d->callIface, &OrgFreedesktopModemManager1CallInterface::StateChanged, d, &CallPrivate::onStateChanged);
    connect(&d->callIface, &OrgFreedesktopModemManager1CallInterface::DtmfReceived, d, &CallPrivate::onDtmfReceived);
}

ModemManager::Call::~Call()
{
    delete d_ptr;
}

QString ModemManager::Call::uni() const
{
    Q_D(const Call);
    return d->uni;
}

QDBusPendingReply<> ModemManager::Call::start()
{
    Q_D(Call);
    return d->callIface.Start();
}

QDBusPendingReply<> ModemManager::Call::accept()
{
    Q_D(Call);
    return d->callIface.Accept();
}

QDBusPendingReply<> ModemManager::Call::hangup()
{
    Q_D(Call);
    return d->callIface.Hangup();
}

QDBusPendingReply<> ModemManager::Call::sendDtmf(const QString &dtmf)
{
    Q_D(Call);
    return d->callIface.SendDtmf(dtmf);
}

MMCallState ModemManager::Call::state() const
{
    Q_D(const Call);
    return d->state;
}

MMCallStateReason ModemManager::Call::stateReason() const
{
    Q_D(const Call);
    return d->stateReason;
}

MMCallDirection ModemManager::Call::direction() const
{
    Q_D(const Call);
    return d->direction;
}

QString ModemManager::Call::number() const
{
    Q_D(const Call);
    return d->number;
}

void ModemManager::Call::setTimeout(int timeout)
{
    Q_D(Call);
    d->callIface.setTimeout(timeout);
}

int ModemManager::Call::timeout() const
{
    Q_D(const Call);
    return d->callIface.timeout();
}

void ModemManager::CallPrivate::onStateChanged(int oldState, int newState, uint reason)
{
    Q_Q(Call);
    state = (MMCallState)newState;
    stateReason = (MMCallStateReason)reason;
    Q_EMIT q->stateChanged((MMCallState)oldState, (MMCallState)newState, (MMCallStateReason)reason);
}

void ModemManager::CallPrivate::onDtmfReceived(const QString &dtmf)
{
    Q_Q(Call);
    Q_EMIT q->dtmfReceived(dtmf);
}

void ModemManager::CallPrivate::onPropertiesChanged(const QString &interfaceName,
                                                    const QVariantMap &changedProperties,
                                                    const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    Q_Q(Call);

    if (interfaceName == QLatin1String(MMQT_DBUS_INTERFACE_CALL)) {
        QVariantMap::const_iterator it = changedProperties.constFind(QLatin1String(MM_CALL_PROPERTY_NUMBER));
        if (it != changedProperties.constEnd()) {
            number = it->toString();
            Q_EMIT q->numberChanged(number);
        }
    }
}

#include "moc_call.cpp"
#include "moc_call_p.cpp"
