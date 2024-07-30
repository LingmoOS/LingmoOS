/*
    SPDX-FileCopyrightText: 2014 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modemoma.h"
#include "mmdebug_p.h"
#include "modemoma_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::ModemOmaPrivate::ModemOmaPrivate(const QString &path, ModemOma *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , omaIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , omaIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (omaIface.isValid()) {
        features = (ModemOma::Features)omaIface.features();
        pendingNetworkInitiatedSessions = omaIface.pendingNetworkInitiatedSessions();
        sessionType = (MMOmaSessionType)omaIface.sessionType();
        sessionState = (MMOmaSessionState)omaIface.sessionState();
    }
}

ModemManager::ModemOma::ModemOma(const QString &path, QObject *parent)
    : Interface(*new ModemOmaPrivate(path, this), parent)
{
    Q_D(ModemOma);

    qRegisterMetaType<QFlags<MMOmaFeature>>();
    qRegisterMetaType<MMOmaSessionType>();
    qRegisterMetaType<MMOmaSessionState>();
    qRegisterMetaType<MMOmaSessionStateFailedReason>();

    connect(&d->omaIface, &OrgFreedesktopModemManager1ModemOmaInterface::SessionStateChanged, d, &ModemOmaPrivate::onSessionStateChanged);
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

ModemManager::ModemOma::~ModemOma()
{
}

ModemManager::ModemOma::Features ModemManager::ModemOma::features() const
{
    Q_D(const ModemOma);

    return d->features;
}

ModemManager::OmaSessionTypes ModemManager::ModemOma::pendingNetworkInitiatedSessions() const
{
    Q_D(const ModemOma);
    return d->pendingNetworkInitiatedSessions;
}

MMOmaSessionType ModemManager::ModemOma::sessionType() const
{
    Q_D(const ModemOma);
    return d->sessionType;
}

MMOmaSessionState ModemManager::ModemOma::sessionState() const
{
    Q_D(const ModemOma);
    return d->sessionState;
}

QDBusPendingReply<void> ModemManager::ModemOma::setup(Features features)
{
    Q_D(ModemOma);
    return d->omaIface.Setup((uint)features);
}

QDBusPendingReply<void> ModemManager::ModemOma::startClientInitiatedSession(MMOmaSessionType sessionType)
{
    Q_D(ModemOma);
    return d->omaIface.StartClientInitiatedSession((uint)sessionType);
}

QDBusPendingReply<void> ModemManager::ModemOma::acceptNetworkInitiatedSession(uint sessionId, bool accept)
{
    Q_D(ModemOma);
    return d->omaIface.AcceptNetworkInitiatedSession(sessionId, accept);
}

QDBusPendingReply<void> ModemManager::ModemOma::cancelSession()
{
    Q_D(ModemOma);
    return d->omaIface.CancelSession();
}

void ModemManager::ModemOma::setTimeout(int timeout)
{
    Q_D(ModemOma);
    d->omaIface.setTimeout(timeout);
}

int ModemManager::ModemOma::timeout() const
{
    Q_D(const ModemOma);
    return d->omaIface.timeout();
}

void ModemManager::ModemOmaPrivate::onSessionStateChanged(int oldState, int newState, uint failedReason)
{
    Q_Q(ModemOma);
    sessionState = (MMOmaSessionState)newState;
    Q_EMIT q->sessionStateChanged((MMOmaSessionState)oldState, (MMOmaSessionState)newState, (MMOmaSessionStateFailedReason)failedReason);
}
void ModemManager::ModemOmaPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps)
{
    Q_Q(ModemOma);
    Q_UNUSED(invalidatedProps);
    qCDebug(MMQT) << interface << properties.keys();

    if (interface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_OMA)) {
        QVariantMap::const_iterator it = properties.constFind(QLatin1String(MM_MODEM_OMA_PROPERTY_FEATURES));
        if (it != properties.constEnd()) {
            features = (ModemOma::Features)it->toUInt();
            Q_EMIT q->featuresChanged(features);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_OMA_PROPERTY_PENDINGNETWORKINITIATEDSESSIONS));
        if (it != properties.constEnd()) {
            pendingNetworkInitiatedSessions = qdbus_cast<OmaSessionTypes>(*it);
            Q_EMIT q->pendingNetworkInitiatedSessionsChanged(pendingNetworkInitiatedSessions);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_OMA_PROPERTY_SESSIONTYPE));
        if (it != properties.constEnd()) {
            sessionType = (MMOmaSessionType)it->toUInt();
            Q_EMIT q->sessionTypeChanged(sessionType);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_OMA_PROPERTY_SESSIONSTATE));
        if (it != properties.constEnd()) {
            // Should be handled by sessionStateChanged() signal
        }
    }
}

#include "moc_modemoma.cpp"
