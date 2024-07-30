/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem3gppussd.h"
#include "mmdebug_p.h"
#include "modem3gppussd_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::Modem3gppUssdPrivate::Modem3gppUssdPrivate(const QString &path, Modem3gppUssd *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , ussdIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , ussdIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (ussdIface.isValid()) {
        state = (MMModem3gppUssdSessionState)ussdIface.state();
        networkNotification = ussdIface.networkNotification();
        networkRequest = ussdIface.networkRequest();
    }
}

ModemManager::Modem3gppUssd::Modem3gppUssd(const QString &path, QObject *parent)
    : Interface(*new Modem3gppUssdPrivate(path, this), parent)
{
    Q_D(Modem3gppUssd);

    qRegisterMetaType<MMModem3gppUssdSessionState>();

#ifdef MMQT_STATIC
    QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                          d->uni,
                                          QLatin1String(DBUS_INTERFACE_PROPS),
                                          QStringLiteral("PropertiesChanged"),
                                          d,
                                          SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#else
    QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                         d->uni,
                                         QLatin1String(DBUS_INTERFACE_PROPS),
                                         QStringLiteral("PropertiesChanged"),
                                         d,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#endif
}

ModemManager::Modem3gppUssd::~Modem3gppUssd()
{
}

void ModemManager::Modem3gppUssdPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps)
{
    Q_Q(Modem3gppUssd);
    Q_UNUSED(invalidatedProps);
    qCDebug(MMQT) << interface << properties.keys();

    if (interface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEM3GPP_USSD)) {
        QVariantMap::const_iterator it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_USSD_PROPERTY_STATE));
        if (it != properties.constEnd()) {
            state = (MMModem3gppUssdSessionState)it->toUInt();
            Q_EMIT q->stateChanged(state);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_USSD_PROPERTY_NETWORKNOTIFICATION));
        if (it != properties.constEnd()) {
            networkNotification = it->toString();
            Q_EMIT q->networkNotificationChanged(networkNotification);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEM3GPP_USSD_PROPERTY_NETWORKREQUEST));
        if (it != properties.constEnd()) {
            networkRequest = it->toString();
            Q_EMIT q->networkRequestChanged(networkRequest);
        }
    }
}

QDBusPendingReply<QString> ModemManager::Modem3gppUssd::initiate(const QString &command)
{
    Q_D(Modem3gppUssd);
    return d->ussdIface.Initiate(command);
}

QDBusPendingReply<QString> ModemManager::Modem3gppUssd::respond(const QString &response)
{
    Q_D(Modem3gppUssd);
    return d->ussdIface.Respond(response);
}

void ModemManager::Modem3gppUssd::cancel()
{
    Q_D(Modem3gppUssd);
    d->ussdIface.Cancel();
}

MMModem3gppUssdSessionState ModemManager::Modem3gppUssd::state() const
{
    Q_D(const Modem3gppUssd);
    return d->state;
}

QString ModemManager::Modem3gppUssd::networkNotification() const
{
    Q_D(const Modem3gppUssd);
    return d->networkNotification;
}

QString ModemManager::Modem3gppUssd::networkRequest() const
{
    Q_D(const Modem3gppUssd);
    return d->networkRequest;
}

void ModemManager::Modem3gppUssd::setTimeout(int timeout)
{
    Q_D(Modem3gppUssd);
    d->ussdIface.setTimeout(timeout);
}

int ModemManager::Modem3gppUssd::timeout() const
{
    Q_D(const Modem3gppUssd);
    return d->ussdIface.timeout();
}

#include "moc_modem3gppussd.cpp"
#include "moc_modem3gppussd_p.cpp"
