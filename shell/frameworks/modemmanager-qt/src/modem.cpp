/*
    SPDX-FileCopyrightText: 2008, 2011 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2010 Lamarque Souza <lamarque@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Lukas Tinkl <ltinkl@redhat.com>
    SPDX-FileCopyrightText: 2013-2015 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "modem.h"
#include "modem_p.h"

#ifdef MMQT_STATIC
#include "dbus/fakedbus.h"
#else
#include "dbus/dbus.h"
#endif
#include "generictypes_p.h"
#include "mmdebug_p.h"

#include <QTimer>

namespace ModemManager
{
class ModemManager::BearerProperties::Private
{
public:
    Private()
    {
    }
    QString apn;
    MMBearerIpFamily ipType;
    MMBearerAllowedAuth allowedAuth;
    QString user;
    QString password;
    bool allowRoaming;
    MMModemCdmaRmProtocol rmProtocol;
    QString number;
};

}

ModemManager::BearerProperties::BearerProperties()
    : d(new Private())
{
}

ModemManager::BearerProperties::BearerProperties(const ModemManager::BearerProperties &other)
    : d(new Private)
{
    *this = other;
}

ModemManager::BearerProperties::~BearerProperties()
{
    delete d;
}

QString ModemManager::BearerProperties::apn() const
{
    return d->apn;
}

void ModemManager::BearerProperties::setApn(const QString &apn)
{
    d->apn = apn;
}

MMBearerIpFamily ModemManager::BearerProperties::ipType() const
{
    return d->ipType;
}

void ModemManager::BearerProperties::setIpType(MMBearerIpFamily ipType)
{
    d->ipType = ipType;
}

MMBearerAllowedAuth ModemManager::BearerProperties::allowedAuthentication() const
{
    return d->allowedAuth;
}

void ModemManager::BearerProperties::setAllowedAuthentication(MMBearerAllowedAuth allowedAuth)
{
    d->allowedAuth = allowedAuth;
}

QString ModemManager::BearerProperties::user() const
{
    return d->user;
}

void ModemManager::BearerProperties::setUser(const QString &user)
{
    d->user = user;
}

QString ModemManager::BearerProperties::password() const
{
    return d->password;
}

void ModemManager::BearerProperties::setPassword(const QString &password)
{
    d->password = password;
}

bool ModemManager::BearerProperties::allowRoaming() const
{
    return d->allowRoaming;
}

void ModemManager::BearerProperties::setAllowRoaming(bool allow)
{
    d->allowRoaming = allow;
}

MMModemCdmaRmProtocol ModemManager::BearerProperties::rmProtocol() const
{
    return d->rmProtocol;
}

void ModemManager::BearerProperties::setRmProtocol(MMModemCdmaRmProtocol rmProtocol)
{
    d->rmProtocol = rmProtocol;
}

QString ModemManager::BearerProperties::number() const
{
    return d->number;
}

void ModemManager::BearerProperties::setNumber(const QString &number)
{
    d->number = number;
}

ModemManager::BearerProperties &ModemManager::BearerProperties::operator=(const ModemManager::BearerProperties &other)
{
    if (this == &other) {
        return *this;
    }

    *d = *other.d;
    return *this;
}

ModemManager::ModemPrivate::ModemPrivate(const QString &path, Modem *q)
    : InterfacePrivate(path, q)
#ifdef MMQT_STATIC
    , modemIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::sessionBus())
#else
    , modemIface(QLatin1String(MMQT_DBUS_SERVICE), path, QDBusConnection::systemBus())
#endif
    , q_ptr(q)
{
    if (modemIface.isValid()) {
        simPath = modemIface.sim().path();
        Q_FOREACH (uint cap, modemIface.supportedCapabilities()) {
            supportedCapabilities.append((MMModemCapability)cap);
        }
        currentCapabilities = (QFlags<MMModemCapability>)modemIface.currentCapabilities();
        maxBearers = modemIface.maxBearers();
        maxActiveBearers = modemIface.maxActiveBearers();
        manufacturer = modemIface.manufacturer();
        model = modemIface.model();
        revision = modemIface.revision();
        deviceIdentifier = modemIface.deviceIdentifier();
        device = modemIface.device();
        drivers = modemIface.drivers();
        plugin = modemIface.plugin();
        primaryPort = modemIface.primaryPort();
        ports = modemIface.ports();
        equipmentIdentifier = modemIface.equipmentIdentifier();
        unlockRequired = (MMModemLock)modemIface.unlockRequired();
        unlockRetries = modemIface.unlockRetries();
        state = (MMModemState)modemIface.state();
        stateFailedReason = (MMModemStateFailedReason)modemIface.stateFailedReason();
        accessTechnologies = (ModemManager::Modem::AccessTechnologies)modemIface.accessTechnologies();
        signalQuality = modemIface.signalQuality();
        ownNumbers = modemIface.ownNumbers();
        powerState = (MMModemPowerState)modemIface.powerState();
        supportedModes = modemIface.supportedModes();
        currentModes = modemIface.currentModes();
        Q_FOREACH (uint band, modemIface.supportedBands()) {
            supportedBands.append((MMModemBand)band);
        }
        Q_FOREACH (uint band, modemIface.currentBands()) {
            currentBands.append((MMModemBand)band);
        }
        supportedIpFamilies = (ModemManager::Modem::IpBearerFamilies)modemIface.supportedIpFamilies();

        QTimer::singleShot(0, this, &ModemManager::ModemPrivate::initializeBearers);
    }
}

void ModemManager::ModemPrivate::initializeBearers()
{
    Q_Q(Modem);

#if MM_CHECK_VERSION(1, 2, 0)
    QList<QDBusObjectPath> bearersList = modemIface.bearers();
    Q_FOREACH (const QDBusObjectPath &bearer, bearersList) {
        if (!bearers.contains(bearer.path())) {
            bearers.insert(bearer.path(), Bearer::Ptr());
            Q_EMIT q->bearerAdded(bearer.path());
        }
    }
#else
    QDBusPendingReply<QList<QDBusObjectPath>> reply = modemIface.ListBearers();
    reply.waitForFinished();
    if (reply.isValid()) {
        Q_FOREACH (const QDBusObjectPath &bearer, reply.value()) {
            if (!bearers.contains(bearer.path())) {
                bearers.insert(bearer.path(), Bearer::Ptr());
                Q_EMIT q->bearerAdded(bearer.path());
            }
        }
    }
#endif
}

ModemManager::Modem::Modem(const QString &path, QObject *parent)
    : Interface(*new ModemPrivate(path, this), parent)
{
    Q_D(Modem);

    qRegisterMetaType<AccessTechnologies>();
    qRegisterMetaType<Capabilities>();
    qRegisterMetaType<ModemModes>();
    qRegisterMetaType<IpBearerFamilies>();
    qRegisterMetaType<MMModemLock>();
    qRegisterMetaType<MMModemPowerState>();
    qRegisterMetaType<MMModemState>();
    qRegisterMetaType<MMModemStateChangeReason>();
    qRegisterMetaType<MMModemStateFailedReason>();

    if (d->modemIface.isValid()) {
#ifdef MMQT_STATIC
        QDBusConnection::sessionBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                              d->uni,
                                              QLatin1String(DBUS_INTERFACE_PROPS),
                                              QLatin1String("PropertiesChanged"),
                                              d,
                                              SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#else
        QDBusConnection::systemBus().connect(QLatin1String(MMQT_DBUS_SERVICE),
                                             d->uni,
                                             QLatin1String(DBUS_INTERFACE_PROPS),
                                             QLatin1String("PropertiesChanged"),
                                             d,
                                             SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
#endif
    }

    connect(&d->modemIface, &OrgFreedesktopModemManager1ModemInterface::StateChanged, d, &ModemPrivate::onStateChanged);
}

ModemManager::Modem::~Modem()
{
}

QString ModemManager::Modem::uni() const
{
    Q_D(const Modem);
    return d->uni;
}

bool ModemManager::Modem::isEnabled() const
{
    Q_D(const Modem);
    return (MMModemPowerState)d->powerState == MM_MODEM_POWER_STATE_ON;
}

bool ModemManager::Modem::isValid() const
{
    Q_D(const Modem);
    return d->modemIface.isValid();
}

QDBusPendingReply<void> ModemManager::Modem::setEnabled(bool enable)
{
    Q_D(Modem);
    return d->modemIface.Enable(enable);
}

QDBusPendingReply<QDBusObjectPath> ModemManager::Modem::createBearer(const ModemManager::BearerProperties &bearerProperties)
{
    Q_D(Modem);
    QVariantMap map;
    map.insert(QStringLiteral("apn"), bearerProperties.apn());
    if (bearerProperties.ipType() != MM_BEARER_IP_FAMILY_NONE) {
        map.insert(QStringLiteral("ip-type"), (uint)bearerProperties.ipType());
    }
    if (bearerProperties.allowedAuthentication() != MM_BEARER_ALLOWED_AUTH_UNKNOWN) {
        map.insert(QStringLiteral("allowed-auth"), (uint)bearerProperties.allowedAuthentication());
    }
    if (!bearerProperties.user().isEmpty()) {
        map.insert(QStringLiteral("user"), bearerProperties.user());
    }
    if (!bearerProperties.password().isEmpty()) {
        map.insert(QStringLiteral("password"), bearerProperties.password());
    }
    map.insert(QStringLiteral("allow-roaming"), bearerProperties.allowRoaming());
    if (bearerProperties.rmProtocol() != MM_MODEM_CDMA_RM_PROTOCOL_UNKNOWN) {
        map.insert(QStringLiteral("rm-protocol"), (uint)bearerProperties.rmProtocol());
    }
    if (!bearerProperties.number().isEmpty()) {
        map.insert(QStringLiteral("number"), bearerProperties.number());
    }
    return d->modemIface.CreateBearer(map);
}

QDBusPendingReply<void> ModemManager::Modem::deleteBearer(const QString &bearer)
{
    Q_D(Modem);
    return d->modemIface.DeleteBearer(QDBusObjectPath(bearer));
}

ModemManager::Bearer::List ModemManager::Modem::listBearers() const
{
    Q_D(const Modem);

    ModemManager::Bearer::List list;
    QMap<QString, Bearer::Ptr>::const_iterator i = d->bearers.constBegin();
    while (i != d->bearers.constEnd()) {
        ModemManager::Bearer::Ptr bearer = const_cast<ModemPrivate *>(d)->findRegisteredBearer(i.key());
        if (bearer) {
            list << bearer;
        }
        ++i;
    }
    return list;
}

ModemManager::Bearer::Ptr ModemManager::Modem::findBearer(const QString &bearer) const
{
    Q_D(const Modem);
    return const_cast<ModemPrivate *>(d)->findRegisteredBearer(bearer);
}

QDBusPendingReply<void> ModemManager::Modem::reset()
{
    Q_D(Modem);
    return d->modemIface.Reset();
}

QDBusPendingReply<void> ModemManager::Modem::factoryReset(const QString &code)
{
    Q_D(Modem);
    return d->modemIface.FactoryReset(code);
}

QDBusPendingReply<void> ModemManager::Modem::setPowerState(MMModemPowerState state)
{
    Q_D(Modem);
    return d->modemIface.SetPowerState(state);
}

QDBusPendingReply<void> ModemManager::Modem::setCurrentCapabilities(Capabilities caps)
{
    Q_D(Modem);
    return d->modemIface.SetCurrentCapabilities((uint)caps);
}

QDBusPendingReply<void> ModemManager::Modem::setCurrentModes(const CurrentModesType &mode)
{
    Q_D(Modem);
    return d->modemIface.SetCurrentModes(mode);
}

QDBusPendingReply<void> ModemManager::Modem::setCurrentBands(const QList<MMModemBand> &bands)
{
    Q_D(Modem);
    QList<uint> tmp;
    Q_FOREACH (const MMModemBand band, bands) {
        tmp.append(band);
    }
    return d->modemIface.SetCurrentBands(tmp);
}

QDBusPendingReply<QString> ModemManager::Modem::command(const QString &cmd, uint timeout)
{
    Q_D(Modem);
    return d->modemIface.Command(cmd, timeout);
}

QString ModemManager::Modem::simPath() const
{
    Q_D(const Modem);
    return d->simPath;
}

QList<MMModemCapability> ModemManager::Modem::supportedCapabilities() const
{
    Q_D(const Modem);
    return d->supportedCapabilities;
}

ModemManager::Modem::Capabilities ModemManager::Modem::currentCapabilities() const
{
    Q_D(const Modem);
    return d->currentCapabilities;
}

uint ModemManager::Modem::maxBearers() const
{
    Q_D(const Modem);
    return d->maxBearers;
}

uint ModemManager::Modem::maxActiveBearers() const
{
    Q_D(const Modem);
    return d->maxActiveBearers;
}

QString ModemManager::Modem::manufacturer() const
{
    Q_D(const Modem);
    return d->manufacturer;
}

QString ModemManager::Modem::model() const
{
    Q_D(const Modem);
    return d->model;
}

QString ModemManager::Modem::revision() const
{
    Q_D(const Modem);
    return d->revision;
}

QString ModemManager::Modem::deviceIdentifier() const
{
    Q_D(const Modem);
    return d->deviceIdentifier;
}

QString ModemManager::Modem::device() const
{
    Q_D(const Modem);
    return d->device;
}

QStringList ModemManager::Modem::drivers() const
{
    Q_D(const Modem);
    return d->drivers;
}

QString ModemManager::Modem::plugin() const
{
    Q_D(const Modem);
    return d->plugin;
}

QString ModemManager::Modem::primaryPort() const
{
    Q_D(const Modem);
    return d->primaryPort;
}

ModemManager::PortList ModemManager::Modem::ports() const
{
    Q_D(const Modem);
    return d->ports;
}

QString ModemManager::Modem::equipmentIdentifier() const
{
    Q_D(const Modem);
    return d->equipmentIdentifier;
}

MMModemLock ModemManager::Modem::unlockRequired() const
{
    Q_D(const Modem);
    return d->unlockRequired;
}

ModemManager::UnlockRetriesMap ModemManager::Modem::unlockRetries() const
{
    Q_D(const Modem);
    return d->unlockRetries;
}

MMModemState ModemManager::Modem::state() const
{
    Q_D(const Modem);
    return d->state;
}

MMModemStateFailedReason ModemManager::Modem::stateFailedReason() const
{
    Q_D(const Modem);
    return d->stateFailedReason;
}

ModemManager::Modem::AccessTechnologies ModemManager::Modem::accessTechnologies() const
{
    Q_D(const Modem);
    return d->accessTechnologies;
}

ModemManager::SignalQualityPair ModemManager::Modem::signalQuality() const
{
    Q_D(const Modem);
    return d->signalQuality;
}

QStringList ModemManager::Modem::ownNumbers() const
{
    Q_D(const Modem);
    return d->ownNumbers;
}

MMModemPowerState ModemManager::Modem::powerState() const
{
    Q_D(const Modem);
    return d->powerState;
}

ModemManager::SupportedModesType ModemManager::Modem::supportedModes() const
{
    Q_D(const Modem);
    return d->supportedModes;
}

ModemManager::CurrentModesType ModemManager::Modem::currentModes() const
{
    Q_D(const Modem);
    return d->currentModes;
}

QList<MMModemBand> ModemManager::Modem::supportedBands() const
{
    Q_D(const Modem);
    return d->supportedBands;
}

QList<MMModemBand> ModemManager::Modem::currentBands() const
{
    Q_D(const Modem);
    return d->currentBands;
}

ModemManager::Modem::IpBearerFamilies ModemManager::Modem::supportedIpFamilies() const
{
    Q_D(const Modem);
    return d->supportedIpFamilies;
}

ModemManager::Bearer::Ptr ModemManager::ModemPrivate::findRegisteredBearer(const QString &path)
{
    Q_Q(Modem);
    ModemManager::Bearer::Ptr ret;
    if (!path.isEmpty()) {
        bool contains = bearers.contains(path);
        if (contains && bearers.value(path)) {
            ret = bearers.value(path);
        } else {
            ret = ModemManager::Bearer::Ptr(new ModemManager::Bearer(path), &QObject::deleteLater);
            bearers[path] = ret;
            if (!contains) {
                Q_EMIT q->bearerAdded(path);
            }
        }
    }
    return ret;
}

void ModemManager::Modem::setTimeout(int timeout)
{
    Q_D(Modem);
    d->modemIface.setTimeout(timeout);
}

int ModemManager::Modem::timeout() const
{
    Q_D(const Modem);
    return d->modemIface.timeout();
}

void ModemManager::ModemPrivate::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    Q_UNUSED(invalidatedProps);
    Q_Q(Modem);
    qCDebug(MMQT) << ifaceName << changedProps.keys();

    if (ifaceName == QLatin1String(MMQT_DBUS_INTERFACE_MODEM)) {
        QVariantMap::const_iterator it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_SIM));

        if (it != changedProps.constEnd()) {
            Q_EMIT q->simPathChanged(simPath, it->toString());
            simPath = it->toString();
        }
#if MM_CHECK_VERSION(1, 2, 0)
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_BEARERS));
        if (it != changedProps.constEnd()) {
            QList<QDBusObjectPath> bearerPaths = qdbus_cast<QList<QDBusObjectPath>>(*it);
            if (bearerPaths.isEmpty()) {
                QMap<QString, Bearer::Ptr>::const_iterator it = bearers.constBegin();
                while (it != bearers.constEnd()) {
                    Q_EMIT q->bearerRemoved(it.key());
                    ++it;
                }
                bearers.clear();
            } else {
                QStringList knownBearers = bearers.keys();
                Q_FOREACH (const QDBusObjectPath &bearer, bearerPaths) {
                    if (!bearers.contains(bearer.path())) {
                        bearers.insert(bearer.path(), ModemManager::Bearer::Ptr());
                        Q_EMIT q->bearerAdded(bearer.path());
                    } else {
                        knownBearers.removeOne(bearer.path());
                    }
                }
                Q_FOREACH (const QString &path, knownBearers) {
                    bearers.remove(path);
                    Q_EMIT q->bearerRemoved(path);
                }
            }
            Q_EMIT q->bearersChanged();
        }
#endif
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDCAPABILITIES));
        if (it != changedProps.constEnd()) {
            supportedCapabilities.clear();
            Q_FOREACH (const uint cap, qdbus_cast<QList<uint>>(*it)) {
                supportedCapabilities << ((MMModemCapability)cap);
            }
            Q_EMIT q->supportedCapabilitiesChanged(supportedCapabilities);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_CURRENTCAPABILITIES));
        if (it != changedProps.constEnd()) {
            currentCapabilities = (QFlags<MMModemCapability>)it->toUInt();
            Q_EMIT q->currentCapabilitiesChanged(currentCapabilities);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_MAXBEARERS));
        if (it != changedProps.constEnd()) {
            maxBearers = it->toUInt();
            Q_EMIT q->maxBearersChanged(maxBearers);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_MAXACTIVEBEARERS));
        if (it != changedProps.constEnd()) {
            maxActiveBearers = it->toUInt();
            Q_EMIT q->maxActiveBearersChanged(maxActiveBearers);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_MANUFACTURER));
        if (it != changedProps.constEnd()) {
            manufacturer = it->toString();
            Q_EMIT q->manufacturerChanged(manufacturer);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_MODEL));
        if (it != changedProps.constEnd()) {
            model = it->toString();
            Q_EMIT q->modelChanged(model);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_REVISION));
        if (it != changedProps.constEnd()) {
            revision = it->toString();
            Q_EMIT q->revisionChanged(revision);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_DEVICEIDENTIFIER));
        if (it != changedProps.constEnd()) {
            deviceIdentifier = it->toString();
            Q_EMIT q->deviceIdentifierChanged(deviceIdentifier);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_DEVICE));
        if (it != changedProps.constEnd()) {
            device = it->toString();
            Q_EMIT q->deviceChanged(device);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_DRIVERS));
        if (it != changedProps.constEnd()) {
            drivers = it->toStringList();
            Q_EMIT q->driversChanged(drivers);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_PLUGIN));
        if (it != changedProps.constEnd()) {
            plugin = it->toString();
            Q_EMIT q->pluginChanged(plugin);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_PRIMARYPORT));
        if (it != changedProps.constEnd()) {
            primaryPort = it->toString();
            Q_EMIT q->primaryPortChanged(primaryPort);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_PORTS));
        if (it != changedProps.constEnd()) {
            ports = qdbus_cast<QList<Port>>(*it);
            Q_EMIT q->portsChanged(ports);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_EQUIPMENTIDENTIFIER));
        if (it != changedProps.constEnd()) {
            equipmentIdentifier = it->toString();
            Q_EMIT q->equipmentIdentifierChanged(equipmentIdentifier);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_UNLOCKREQUIRED));
        if (it != changedProps.constEnd()) {
            unlockRequired = (MMModemLock)it->toUInt();
            Q_EMIT q->unlockRequiredChanged(unlockRequired);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_UNLOCKRETRIES));
        if (it != changedProps.constEnd()) {
            unlockRetries = qdbus_cast<UnlockRetriesMap>(*it);
            Q_EMIT q->unlockRetriesChanged(unlockRetries);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_STATE));
        if (it != changedProps.constEnd()) {
            // Should be handled by StateChanged signal
            //             Q_EMIT q->stateChanged(state, (MMModemState)it->toInt());
            //             state = (MMModemState)it->toInt();
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_STATEFAILEDREASON));
        if (it != changedProps.constEnd()) {
            stateFailedReason = (MMModemStateFailedReason)it->toUInt();
            Q_EMIT q->stateFailedReasonChanged(stateFailedReason);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_ACCESSTECHNOLOGIES));
        if (it != changedProps.constEnd()) {
            accessTechnologies = (ModemManager::Modem::AccessTechnologies)it->toUInt();
            Q_EMIT q->accessTechnologiesChanged(accessTechnologies);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_SIGNALQUALITY));
        if (it != changedProps.constEnd()) {
            signalQuality = qdbus_cast<SignalQualityPair>(*it);
            Q_EMIT q->signalQualityChanged(signalQuality);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_OWNNUMBERS));
        if (it != changedProps.constEnd()) {
            ownNumbers = it->toStringList();
            Q_EMIT q->ownNumbersChanged(ownNumbers);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_POWERSTATE));
        if (it != changedProps.constEnd()) {
            powerState = (MMModemPowerState)it->toUInt();
            Q_EMIT q->powerStateChanged(powerState);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDMODES));
        if (it != changedProps.constEnd()) {
            supportedModes = qdbus_cast<SupportedModesType>(*it);
            Q_EMIT q->supportedModesChanged(supportedModes);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_CURRENTMODES));
        if (it != changedProps.constEnd()) {
            currentModes = qdbus_cast<CurrentModesType>(*it);
            Q_EMIT q->currentModesChanged(currentModes);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDBANDS));
        if (it != changedProps.constEnd()) {
            supportedBands.clear();
            Q_FOREACH (const uint cap, qdbus_cast<QList<uint>>(*it)) {
                supportedBands << ((MMModemBand)cap);
            }
            Q_EMIT q->supportedBandsChanged(supportedBands);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_CURRENTBANDS));
        if (it != changedProps.constEnd()) {
            currentBands.clear();
            Q_FOREACH (const uint cap, qdbus_cast<QList<uint>>(*it)) {
                currentBands << ((MMModemBand)cap);
            }
            Q_EMIT q->currentBandsChanged(currentBands);
        }
        it = changedProps.constFind(QLatin1String(MM_MODEM_PROPERTY_SUPPORTEDIPFAMILIES));
        if (it != changedProps.constEnd()) {
            supportedIpFamilies = (ModemManager::Modem::IpBearerFamilies)it->toUInt();
            Q_EMIT q->supportedIpFamiliesChanged(supportedIpFamilies);
        }
    }
}

void ModemManager::ModemPrivate::onStateChanged(int oldState, int newState, uint reason)
{
    Q_Q(Modem);
    state = (MMModemState)newState;
    Q_EMIT q->stateChanged((MMModemState)oldState, (MMModemState)newState, (MMModemStateChangeReason)reason);
}

#include "moc_modem.cpp"
#include "moc_modem_p.cpp"
