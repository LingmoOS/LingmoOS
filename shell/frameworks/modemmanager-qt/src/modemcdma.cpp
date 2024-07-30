/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "mmdebug_p.h"
#include "modemcdma_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::ModemCdmaPrivate::ModemCdmaPrivate(const QString &path, ModemCdma *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modemCdmaIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modemCdmaIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (modemCdmaIface.isValid()) {
        activationState = (MMModemCdmaActivationState)modemCdmaIface.activationState();
        meid = modemCdmaIface.meid();
        esn = modemCdmaIface.esn();
        sid = modemCdmaIface.sid();
        nid = modemCdmaIface.nid();
        cdma1xRegistrationState = (MMModemCdmaRegistrationState)modemCdmaIface.cdma1xRegistrationState();
        evdoRegistrationState = (MMModemCdmaRegistrationState)modemCdmaIface.evdoRegistrationState();
    }
}

ModemManager::ModemCdma::ModemCdma(const QString &path, QObject *parent)
    : Interface(*new ModemCdmaPrivate(path, this), parent)
{
    Q_D(ModemCdma);

    qRegisterMetaType<MMModemCdmaActivationState>();
    qRegisterMetaType<MMCdmaActivationError>();
    qRegisterMetaType<MMModemCdmaRegistrationState>();

    connect(&d->modemCdmaIface, &OrgFreedesktopModemManager1ModemModemCdmaInterface::ActivationStateChanged, d, &ModemCdmaPrivate::onActivationStateChanged);
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

ModemManager::ModemCdma::~ModemCdma()
{
}

QDBusPendingReply<void> ModemManager::ModemCdma::activate(const QString &carrierCode)
{
    Q_D(ModemCdma);
    return d->modemCdmaIface.Activate(carrierCode);
}

QDBusPendingReply<void> ModemManager::ModemCdma::activateManual(const QVariantMap &properties)
{
    Q_D(ModemCdma);
    return d->modemCdmaIface.ActivateManual(properties);
}

MMModemCdmaActivationState ModemManager::ModemCdma::activationState() const
{
    Q_D(const ModemCdma);
    return d->activationState;
}

QString ModemManager::ModemCdma::meid() const
{
    Q_D(const ModemCdma);
    return d->meid;
}

QString ModemManager::ModemCdma::esn() const
{
    Q_D(const ModemCdma);
    return d->esn;
}

uint ModemManager::ModemCdma::sid() const
{
    Q_D(const ModemCdma);
    return d->sid;
}

uint ModemManager::ModemCdma::nid() const
{
    Q_D(const ModemCdma);
    return d->nid;
}

MMModemCdmaRegistrationState ModemManager::ModemCdma::cdma1xRegistrationState() const
{
    Q_D(const ModemCdma);
    return d->cdma1xRegistrationState;
}

MMModemCdmaRegistrationState ModemManager::ModemCdma::evdoRegistrationState() const
{
    Q_D(const ModemCdma);
    return d->evdoRegistrationState;
}

void ModemManager::ModemCdma::setTimeout(int timeout)
{
    Q_D(ModemCdma);
    d->modemCdmaIface.setTimeout(timeout);
}

int ModemManager::ModemCdma::timeout() const
{
    Q_D(const ModemCdma);
    return d->modemCdmaIface.timeout();
}

void ModemManager::ModemCdmaPrivate::onActivationStateChanged(uint activation_state, uint activation_error, const QVariantMap &status_changes)
{
    Q_Q(ModemCdma);
    activationState = (MMModemCdmaActivationState)activation_state;
    Q_EMIT q->activationStateChanged((MMModemCdmaActivationState)activation_state, (MMCdmaActivationError)activation_error, status_changes);
}

void ModemManager::ModemCdmaPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps)
{
    Q_Q(ModemCdma);
    Q_UNUSED(invalidatedProps);
    qCDebug(MMQT) << interface << properties.keys();

    if (interface == QLatin1String(MMQT_DBUS_INTERFACE_MODEM_MODEMCDMA)) {
        QVariantMap::const_iterator it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_ACTIVATIONSTATE));
        if (it != properties.constEnd()) {
            // Should be handled by activationStateChanged signal
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_MEID));
        if (it != properties.constEnd()) {
            meid = it->toString();
            Q_EMIT q->meidChanged(meid);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_ESN));
        if (it != properties.constEnd()) {
            esn = it->toString();
            Q_EMIT q->esnChanged(esn);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_SID));
        if (it != properties.constEnd()) {
            sid = it->toUInt();
            Q_EMIT q->sidChanged(sid);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_NID));
        if (it != properties.constEnd()) {
            nid = it->toUInt();
            Q_EMIT q->nidChanged(nid);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_CDMA1XREGISTRATIONSTATE));
        if (it != properties.constEnd()) {
            cdma1xRegistrationState = (MMModemCdmaRegistrationState)it->toUInt();
            Q_EMIT q->cdma1xRegistrationStateChanged(cdma1xRegistrationState);
        }
        it = properties.constFind(QLatin1String(MM_MODEM_MODEMCDMA_PROPERTY_EVDOREGISTRATIONSTATE));
        if (it != properties.constEnd()) {
            evdoRegistrationState = (MMModemCdmaRegistrationState)it->toUInt();
            Q_EMIT q->evdoRegistrationStateChanged(evdoRegistrationState);
        }
    }
}

#include "moc_modemcdma.cpp"
