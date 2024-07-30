/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010-2011 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "sim.h"
#include "mmdebug_p.h"
#include "sim_p.h"
#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif

ModemManager::SimPrivate::SimPrivate(const QString &path, Sim *q)
#ifdef MMQT_STATIC
    : simIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    : simIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , uni(path)
    , q_ptr(q)
{
    if (simIface.isValid()) {
        active = simIface.active();
        simIdentifier = simIface.simIdentifier();
        imsi = simIface.imsi();
        eid = simIface.eid();
        operatorIdentifier = simIface.operatorIdentifier();
        operatorName = simIface.operatorName();
        emergencyNumbers = simIface.emergencyNumbers();
        preferredNetworks = simIface.preferredNetworks();
#if MM_CHECK_VERSION(1, 20, 0)
        gid1 = simIface.gid1();
        gid2 = simIface.gid2();
        simType = (MMSimType)simIface.simType();
        esimStatus = (MMSimEsimStatus)simIface.esimStatus();
        removability = (MMSimRemovability)simIface.removability();
#endif
    }
}

ModemManager::Sim::Sim(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new SimPrivate(path, this))
{
    Q_D(Sim);
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
}

ModemManager::Sim::~Sim()
{
    delete d_ptr;
}

bool ModemManager::Sim::active() const
{
    Q_D(const Sim);
    return d->active;
}

QString ModemManager::Sim::simIdentifier() const
{
    Q_D(const Sim);
    return d->simIdentifier;
}

QString ModemManager::Sim::imsi() const
{
    Q_D(const Sim);
    return d->imsi;
}

QString ModemManager::Sim::eid() const
{
    Q_D(const Sim);
    return d->eid;
}

QString ModemManager::Sim::operatorIdentifier() const
{
    Q_D(const Sim);
    return d->operatorIdentifier;
}

QString ModemManager::Sim::operatorName() const
{
    Q_D(const Sim);
    return d->operatorName;
}

QStringList ModemManager::Sim::emergencyNumbers() const
{
    Q_D(const Sim);
    return d->emergencyNumbers;
}

QVariantMap ModemManager::Sim::preferredNetworks() const
{
    Q_D(const Sim);
    return d->preferredNetworks;
}

#if MM_CHECK_VERSION(1, 20, 0)
QByteArray ModemManager::Sim::gid1() const
{
    Q_D(const Sim);
    return d->gid1;
}

QByteArray ModemManager::Sim::gid2() const
{
    Q_D(const Sim);
    return d->gid2;
}

MMSimType ModemManager::Sim::simType() const
{
    Q_D(const Sim);
    return d->simType;
}

MMSimEsimStatus ModemManager::Sim::esimStatus() const
{
    Q_D(const Sim);
    return d->esimStatus;
}

MMSimRemovability ModemManager::Sim::removability() const
{
    Q_D(const Sim);
    return d->removability;
}
#endif

QDBusPendingReply<> ModemManager::Sim::sendPuk(const QString &puk, const QString &pin)
{
    Q_D(Sim);
    return d->simIface.SendPuk(puk, pin);
}

QDBusPendingReply<> ModemManager::Sim::sendPin(const QString &pin)
{
    Q_D(Sim);
    return d->simIface.SendPin(pin);
}

QDBusPendingReply<> ModemManager::Sim::enablePin(const QString &pin, bool enabled)
{
    Q_D(Sim);
    return d->simIface.EnablePin(pin, enabled);
}

QDBusPendingReply<> ModemManager::Sim::changePin(const QString &oldPin, const QString &newPin)
{
    Q_D(Sim);
    return d->simIface.ChangePin(oldPin, newPin);
}

QDBusPendingReply<> ModemManager::Sim::setPreferredNetworks(QVariantMap preferredNetworks)
{
    Q_D(Sim);
    return d->simIface.SetPreferredNetworks(preferredNetworks);
}

QString ModemManager::Sim::uni() const
{
    Q_D(const Sim);
    return d->uni;
}

void ModemManager::Sim::setTimeout(int timeout)
{
    Q_D(Sim);
    d->simIface.setTimeout(timeout);
}

int ModemManager::Sim::timeout() const
{
    Q_D(const Sim);
    return d->simIface.timeout();
}

void ModemManager::SimPrivate::onPropertiesChanged(const QString &interface, const QVariantMap &properties, const QStringList &invalidatedProps)
{
    Q_Q(Sim);
    Q_UNUSED(invalidatedProps);
    qCDebug(MMQT) << interface << properties.keys();

    if (interface == QLatin1String(MMQT_DBUS_INTERFACE_SIM)) {
        QVariantMap::const_iterator it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_ACTIVE));
        if (it != properties.constEnd()) {
            active = it->toBool();
            Q_EMIT q->activeChanged(active);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_SIMIDENTIFIER));
        if (it != properties.constEnd()) {
            simIdentifier = it->toString();
            Q_EMIT q->simIdentifierChanged(simIdentifier);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_IMSI));
        if (it != properties.constEnd()) {
            imsi = it->toString();
            Q_EMIT q->imsiChanged(imsi);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_EID));
        if (it != properties.constEnd()) {
            eid = it->toString();
            Q_EMIT q->eidChanged(eid);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_OPERATORIDENTIFIER));
        if (it != properties.constEnd()) {
            operatorIdentifier = it->toString();
            Q_EMIT q->operatorIdentifierChanged(operatorIdentifier);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_OPERATORNAME));
        if (it != properties.constEnd()) {
            operatorName = it->toString();
            Q_EMIT q->operatorNameChanged(operatorName);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_EMERGENCYNUMBERS));
        if (it != properties.constEnd()) {
            emergencyNumbers = it->toStringList();
            Q_EMIT q->emergencyNumbersChanged(emergencyNumbers);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_PREFERREDNETWORKS));
        if (it != properties.constEnd()) {
            preferredNetworks = it->toMap();
            Q_EMIT q->preferredNetworksChanged(preferredNetworks);
        }
#if MM_CHECK_VERSION(1, 20, 0)
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_GID1));
        if (it != properties.constEnd()) {
            gid1 = it->toByteArray();
            Q_EMIT q->gid1Changed(gid1);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_GID2));
        if (it != properties.constEnd()) {
            gid2 = it->toByteArray();
            Q_EMIT q->gid1Changed(gid2);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_SIMTYPE));
        if (it != properties.constEnd()) {
            simType = (MMSimType)it->toUInt();
            Q_EMIT q->simTypeChanged(simType);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_ESIMSTATUS));
        if (it != properties.constEnd()) {
            esimStatus = (MMSimEsimStatus)it->toUInt();
            Q_EMIT q->esimStatusChanged(esimStatus);
        }
        it = properties.constFind(QLatin1String(MM_SIM_PROPERTY_REMOVABILITY));
        if (it != properties.constEnd()) {
            removability = (MMSimRemovability)it->toUInt();
            Q_EMIT q->removabilityChanged(removability);
        }
#endif
    }
}

#include "moc_sim.cpp"
#include "moc_sim_p.cpp"
