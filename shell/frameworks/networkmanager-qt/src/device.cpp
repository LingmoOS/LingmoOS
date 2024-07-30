/*
    SPDX-FileCopyrightText: 2008, 2010 Will Stephenson <wstephenson@kde.org>
    SPDX-FileCopyrightText: 2013 Daniel Nicoletti <dantti12@gmail.com>
    SPDX-FileCopyrightText: 2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "connection.h"
#include "device_p.h"
#include "manager.h"
#include "manager_p.h"
#include "nmdebug.h"
#include "settings.h"

#include <arpa/inet.h>

#include <QDBusMetaType>
#include <QDBusObjectPath>

// logging category for this framework, default: log stuff >= warning
Q_LOGGING_CATEGORY(NMQT, "kf.networkmanagerqt", QtWarningMsg)

namespace NetworkManager
{
class DeviceStateReasonPrivate
{
public:
    DeviceStateReasonPrivate(Device::State st, Device::StateChangeReason rsn)
        : state(st)
        , reason(rsn)
    {
    }
    DeviceStateReasonPrivate()
        : state(Device::UnknownState)
        , reason(Device::UnknownReason)
    {
    }
    Device::State state;
    Device::StateChangeReason reason;
};
}

NetworkManager::DeviceStateReason::DeviceStateReason(Device::State state, Device::StateChangeReason reason)
    : d_ptr(new DeviceStateReasonPrivate(state, reason))
{
}

NetworkManager::DeviceStateReason::DeviceStateReason(const NetworkManager::DeviceStateReason &other)
    : d_ptr(new DeviceStateReasonPrivate(*other.d_ptr))
{
}

NetworkManager::DeviceStateReason::~DeviceStateReason()
{
    delete d_ptr;
}

NetworkManager::Device::State NetworkManager::DeviceStateReason::state() const
{
    Q_D(const DeviceStateReason);
    return d->state;
}

NetworkManager::Device::StateChangeReason NetworkManager::DeviceStateReason::reason() const
{
    Q_D(const DeviceStateReason);
    return d->reason;
}

NetworkManager::DeviceStateReason &NetworkManager::DeviceStateReason::operator=(const NetworkManager::DeviceStateReason &other)
{
    if (&other != this) {
        *d_ptr = *other.d_ptr;
    }
    return *this;
}

NetworkManager::DevicePrivate::DevicePrivate(const QString &path, NetworkManager::Device *q)
#ifdef NMQT_STATIC
    : deviceIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::sessionBus())
#else
    : deviceIface(NetworkManagerPrivate::DBUS_SERVICE, path, QDBusConnection::systemBus())
#endif
    , uni(path)
    , designSpeed(0)
    , deviceType(Device::UnknownType)
    , dhcp4Config(nullptr)
    , dhcp6Config(nullptr)
    , mtu(0)
    , q_ptr(q)
{
}

NetworkManager::DevicePrivate::~DevicePrivate()
{
}

void NetworkManager::DevicePrivate::init()
{
    qDBusRegisterMetaType<UIntList>();
    qDBusRegisterMetaType<UIntListList>();
    qDBusRegisterMetaType<IpV6DBusAddress>();
    qDBusRegisterMetaType<IpV6DBusAddressList>();
    qDBusRegisterMetaType<IpV6DBusNameservers>();
    qDBusRegisterMetaType<IpV6DBusRoute>();
    qDBusRegisterMetaType<IpV6DBusRouteList>();
    qDBusRegisterMetaType<DeviceDBusStateReason>();

    QDBusConnection::systemBus().connect(NetworkManagerPrivate::DBUS_SERVICE,
                                         uni,
                                         NetworkManagerPrivate::FDO_DBUS_PROPERTIES,
                                         QLatin1String("PropertiesChanged"),
                                         this,
                                         SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
    QObject::connect(&deviceIface, &OrgFreedesktopNetworkManagerDeviceInterface::StateChanged, this, &DevicePrivate::deviceStateChanged);


    // The interfaceFlags will return NM_DEVICE_INTERFACE_FLAG_NONE when runtime NM < 1.22
    // we initialize the interfaceFlags
    interfaceFlags = NetworkManager::Device::Interfaceflag::None;
    deviceStatistics = DeviceStatistics::Ptr(new NetworkManager::DeviceStatistics(uni), &QObject::deleteLater);

    // Get all Device's properties at once
    QVariantMap initialProperties = NetworkManagerPrivate::retrieveInitialProperties(deviceIface.staticInterfaceName(), uni);
    if (!initialProperties.isEmpty()) {
        propertiesChanged(initialProperties);
    }

}

NetworkManager::Device::MeteredStatus NetworkManager::DevicePrivate::convertMeteredStatus(uint metered)
{
    NetworkManager::Device::MeteredStatus ourMeteredStatus = (NetworkManager::Device::MeteredStatus)metered;
    return ourMeteredStatus;
}

NetworkManager::Device::Capabilities NetworkManager::DevicePrivate::convertCapabilities(uint theirCaps)
{
    NetworkManager::Device::Capabilities ourCaps = (NetworkManager::Device::Capabilities)theirCaps;
    return ourCaps;
}

NetworkManager::Device::Interfaceflags NetworkManager::DevicePrivate::convertInterfaceflags(uint flags)
{
    NetworkManager::Device::Interfaceflags ourFlags = static_cast<NetworkManager::Device::Interfaceflags>(flags);
    return ourFlags;
}

NetworkManager::Device::State NetworkManager::DevicePrivate::convertState(uint theirState)
{
    NetworkManager::Device::State ourState = static_cast<NetworkManager::Device::State>(theirState);
    return ourState;
}

NetworkManager::Device::StateChangeReason NetworkManager::DevicePrivate::convertReason(uint theirReason)
{
    NetworkManager::Device::StateChangeReason ourReason = (NetworkManager::Device::StateChangeReason)theirReason;
    return ourReason;
}

NetworkManager::Device::Type NetworkManager::DevicePrivate::convertType(uint type)
{
    // These are identical to NM enums
    if (type <= NM_DEVICE_TYPE_TEAM) {
        return (NetworkManager::Device::Type)type;
    }

    switch (type) {
    case 16:
        // NM_DEVICE_TYPE_TUN
        return NetworkManager::Device::Tun;
    case 17:
        // NM_DEVICE_TYPE_IP_TUNNEL
        return NetworkManager::Device::IpTunnel;
    case 18:
        // NM_DEVICE_TYPE_MACVLAN
        return NetworkManager::Device::MacVlan;
    case 19:
        // NM_DEVICE_TYPE_VXLAN
        return NetworkManager::Device::VxLan;
    case 20:
        // NM_DEVICE_TYPE_VETH
        return NetworkManager::Device::Veth;
    case 21:
        // NM_DEVICE_TYPE_MACSEC
        return NetworkManager::Device::MacSec;
    case 22:
        // NM_DEVICE_TYPE_DUMMY
        return NetworkManager::Device::Dummy;
    case 23:
        // NM_DEVICE_TYPE_PPP
        return NetworkManager::Device::Ppp;
    case 24:
        // NM_DEVICE_TYPE_OVS_INTERFACE
        return NetworkManager::Device::OvsInterface;
    case 25:
        // NM_DEVICE_TYPE_OVS_PORT
        return NetworkManager::Device::OvsPort;
    case 26:
        // NM_DEVICE_TYPE_OVS_BRIDGE
        return NetworkManager::Device::OvsBridge;
    case 27:
        // NM_DEVICE_TYPE_WPAN
        return NetworkManager::Device::Wpan;
    case 28:
        // NM_DEVICE_TYPE_6LOWPAN
        return NetworkManager::Device::Lowpan;
    case 29:
        // NM_DEVICE_TYPE_NM_DEVICE_TYPE_WIREGUARD
        return NetworkManager::Device::WireGuard;
    case 30:
        // NM_DEVICE_TYPE_WIFI_P2P
        return NetworkManager::Device::WifiP2P;
    case 31:
        // NM_DEVICE_TYPE_VRF
        return NetworkManager::Device::VRF;
    case 32:
        // NM_DEVICE_TYPE_LOOPBACK
        return NetworkManager::Device::Loopback;
    }

    return NetworkManager::Device::UnknownType;
}

NetworkManager::Device::Device(const QString &path, QObject *parent)
    : QObject(parent)
    , d_ptr(new DevicePrivate(path, this))
{
    Q_D(Device);

    d->init();
}

NetworkManager::Device::Device(DevicePrivate &dd, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_D(Device);

    d->init();
}

void NetworkManager::DevicePrivate::propertyChanged(const QString &property, const QVariant &value)
{
    Q_Q(Device);

    // qCDebug(NMQT) << property  << " - " << value;
    if (property == QLatin1String("ActiveConnection")) {
        activeConnection = value.value<QDBusObjectPath>().path();
        Q_EMIT q->activeConnectionChanged();
    } else if (property == QLatin1String("Autoconnect")) {
        autoconnect = value.toBool();
        Q_EMIT q->autoconnectChanged();
    } else if (property == QLatin1String("AvailableConnections")) {
        QStringList newAvailableConnections;
        const QList<QDBusObjectPath> availableConnectionsTmp = qdbus_cast<QList<QDBusObjectPath>>(value);
        for (const QDBusObjectPath &availableConnection : availableConnectionsTmp) {
            newAvailableConnections << availableConnection.path();
            if (!availableConnections.contains(availableConnection.path())) {
                availableConnections << availableConnection.path();
                Q_EMIT q->availableConnectionAppeared(availableConnection.path());
            }
        }
        auto it = availableConnections.begin();
        while (it != availableConnections.end()) {
            const QString availableConnection = *it;
            if (!newAvailableConnections.contains(availableConnection)) {
                it = availableConnections.erase(it);
                Q_EMIT q->availableConnectionDisappeared(availableConnection);
            } else {
                ++it;
            }
        }
        Q_EMIT q->availableConnectionChanged();
    } else if (property == QLatin1String("Capabilities")) {
        capabilities = NetworkManager::DevicePrivate::convertCapabilities(value.toUInt());
        Q_EMIT q->capabilitiesChanged();
    } else if (property == QLatin1String("DeviceType")) {
        deviceType = convertType(value.toUInt());
    } else if (property == QLatin1String("Dhcp4Config")) {
        QDBusObjectPath dhcp4ConfigPathTmp = value.value<QDBusObjectPath>();
        if (dhcp4ConfigPathTmp.path().isNull()) {
            dhcp4Config.clear();
            dhcp4ConfigPath.clear();
        } else if (!dhcp4Config || dhcp4Config->path() != dhcp4ConfigPathTmp.path()) {
            dhcp4Config.clear();
            dhcp4ConfigPath = dhcp4ConfigPathTmp.path();
        }
        Q_EMIT q->dhcp4ConfigChanged();
    } else if (property == QLatin1String("Dhcp6Config")) {
        QDBusObjectPath dhcp6ConfigPathTmp = value.value<QDBusObjectPath>();
        if (dhcp6ConfigPathTmp.path().isNull()) {
            dhcp6Config.clear();
            dhcp6ConfigPath.clear();
        } else if (!dhcp6Config || dhcp6Config->path() != dhcp6ConfigPathTmp.path()) {
            dhcp6Config.clear();
            dhcp6ConfigPath = dhcp6ConfigPathTmp.path();
        }
        Q_EMIT q->dhcp6ConfigChanged();
    } else if (property == QLatin1String("Driver")) {
        driver = value.toString();
        Q_EMIT q->driverChanged();
    } else if (property == QLatin1String("DriverVersion")) {
        driverVersion = value.toString();
        Q_EMIT q->driverVersionChanged();
    } else if (property == QLatin1String("FirmwareMissing")) {
        firmwareMissing = value.toBool();
        Q_EMIT q->firmwareMissingChanged();
    } else if (property == QLatin1String("FirmwareVersion")) {
        firmwareVersion = value.toString();
        Q_EMIT q->firmwareVersionChanged();
    } else if (property == QLatin1String("Interface")) {
        interfaceName = value.toString();
        Q_EMIT q->interfaceNameChanged();
    } else if (property == QLatin1String("InterfaceFlags")) {
        interfaceFlags = NetworkManager::DevicePrivate::convertInterfaceflags(value.toUInt());
        Q_EMIT q->interfaceFlagsChanged();
    } else if (property == QLatin1String("Ip4Address")) {
        ipV4Address = QHostAddress(ntohl(value.toUInt()));
        Q_EMIT q->ipV4AddressChanged();
    } else if (property == QLatin1String("Ip4Config")) {
        QDBusObjectPath ip4ConfigObjectPathTmp = value.value<QDBusObjectPath>();
        if (ip4ConfigObjectPathTmp.path().isNull() || ip4ConfigObjectPathTmp.path() == QLatin1String("/")) {
            ipV4ConfigPath.clear();
        } else {
            ipV4ConfigPath = ip4ConfigObjectPathTmp.path();
        }
        ipV4Config = IpConfig();
        Q_EMIT q->ipV4ConfigChanged();
    } else if (property == QLatin1String("Ip6Config")) {
        QDBusObjectPath ip6ConfigObjectPathTmp = value.value<QDBusObjectPath>();
        if (ip6ConfigObjectPathTmp.path().isNull() || ip6ConfigObjectPathTmp.path() == QLatin1String("/")) {
            ipV6ConfigPath.clear();
        } else {
            ipV6ConfigPath = ip6ConfigObjectPathTmp.path();
        }
        ipV6Config = IpConfig();
        Q_EMIT q->ipV6ConfigChanged();
    } else if (property == QLatin1String("IpInterface")) {
        ipInterface = value.toString();
        Q_EMIT q->ipInterfaceChanged();
    } else if (property == QLatin1String("Managed")) {
        managed = value.toBool();
        Q_EMIT q->managedChanged();
    } else if (property == QLatin1String("State")) {
        connectionState = NetworkManager::DevicePrivate::convertState(value.toUInt());
        // FIXME NetworkManager 0.9.8 (maybe greater) doesn't
        // update ActiveConnection when disconnected
        // This is fixed in NM 73d128bbd17120225bb4986e3f05566f10fab581
        if (connectionState == NetworkManager::Device::Disconnected && activeConnection != QLatin1String("/")) {
            activeConnection = QLatin1Char('/');
            Q_EMIT q->activeConnectionChanged();
        }
        Q_EMIT q->connectionStateChanged();
    } else if (property == QLatin1String("StateReason")) { // just extracting the reason
        reason = NetworkManager::DevicePrivate::convertReason(qdbus_cast<DeviceDBusStateReason>(value).reason);
        Q_EMIT q->stateReasonChanged();
    } else if (property == QLatin1String("Udi")) {
        udi = value.toString();
        Q_EMIT q->udiChanged();
    } else if (property == QLatin1String("PhysicalPortId")) {
        physicalPortId = value.toString();
        Q_EMIT q->physicalPortIdChanged();
    } else if (property == QLatin1String("Mtu")) {
        mtu = value.toUInt();
        Q_EMIT q->mtuChanged();
    } else if (property == QLatin1String("NmPluginMissing")) {
        nmPluginMissing = value.toBool();
        Q_EMIT q->nmPluginMissingChanged(nmPluginMissing);
    } else if (property == QLatin1String("Metered")) {
        metered = NetworkManager::DevicePrivate::convertMeteredStatus(value.toUInt());
        Q_EMIT q->meteredChanged(metered);
    } else {
        qCDebug(NMQT) << Q_FUNC_INFO << "Unhandled property" << property;
    }
}

NetworkManager::Device::~Device()
{
    Q_D(Device);
    delete d;
}

QString NetworkManager::Device::uni() const
{
    Q_D(const Device);
    return d->uni;
}

QString NetworkManager::Device::interfaceName() const
{
    Q_D(const Device);
    return d->interfaceName;
}

QString NetworkManager::Device::ipInterfaceName() const
{
    Q_D(const Device);
    return d->ipInterface;
}

QString NetworkManager::Device::driver() const
{
    Q_D(const Device);
    return d->driver;
}

QString NetworkManager::Device::driverVersion() const
{
    Q_D(const Device);
    return d->driverVersion;
}

QString NetworkManager::Device::firmwareVersion() const
{
    Q_D(const Device);
    return d->firmwareVersion;
}

NetworkManager::ActiveConnection::Ptr NetworkManager::Device::activeConnection() const
{
    Q_D(const Device);
    return NetworkManager::findActiveConnection(d->activeConnection);
}

NetworkManager::Connection::List NetworkManager::Device::availableConnections()
{
    Q_D(const Device);

    NetworkManager::Connection::List list;
    for (const QString &availableConnection : std::as_const(d->availableConnections)) {
        NetworkManager::Connection::Ptr connection = NetworkManager::findConnection(availableConnection);
        if (connection) {
            list << connection;
        }
    }

    return list;
}

bool NetworkManager::Device::firmwareMissing() const
{
    Q_D(const Device);
    return d->firmwareMissing;
}

bool NetworkManager::Device::autoconnect() const
{
    Q_D(const Device);
    return d->autoconnect;
}

void NetworkManager::Device::setAutoconnect(bool autoconnect)
{
    Q_D(Device);
    d->deviceIface.setAutoconnect(autoconnect);
}

QString NetworkManager::Device::udi() const
{
    Q_D(const Device);
    return d->udi;
}

QString NetworkManager::Device::physicalPortId() const
{
    Q_D(const Device);
    return d->physicalPortId;
}

QHostAddress NetworkManager::Device::ipV4Address() const
{
    Q_D(const Device);
    return d->ipV4Address;
}

NetworkManager::DeviceStateReason NetworkManager::Device::stateReason() const
{
    Q_D(const Device);
    return DeviceStateReason(d->connectionState, d->reason);
}

NetworkManager::IpConfig NetworkManager::Device::ipV4Config() const
{
    Q_D(const Device);
    if (!d->ipV4Config.isValid() && !d->ipV4ConfigPath.isNull()) {
        d->ipV4Config.setIPv4Path(d->ipV4ConfigPath);
    }
    return d->ipV4Config;
}

NetworkManager::IpConfig NetworkManager::Device::ipV6Config() const
{
    Q_D(const Device);
    if (!d->ipV6Config.isValid() && !d->ipV6ConfigPath.isNull()) {
        d->ipV6Config.setIPv6Path(d->ipV6ConfigPath);
    }
    return d->ipV6Config;
}

NetworkManager::Dhcp4Config::Ptr NetworkManager::Device::dhcp4Config() const
{
    Q_D(const Device);
    if (!d->dhcp4Config && !d->dhcp4ConfigPath.isNull()) {
        d->dhcp4Config = NetworkManager::Dhcp4Config::Ptr(new Dhcp4Config(d->dhcp4ConfigPath), &QObject::deleteLater);
    }
    return d->dhcp4Config;
}

NetworkManager::Dhcp6Config::Ptr NetworkManager::Device::dhcp6Config() const
{
    Q_D(const Device);
    if (!d->dhcp6Config && !d->dhcp6ConfigPath.isNull()) {
        d->dhcp6Config = NetworkManager::Dhcp6Config::Ptr(new Dhcp6Config(d->dhcp6ConfigPath), &QObject::deleteLater);
    }
    return d->dhcp6Config;
}

bool NetworkManager::Device::isActive() const
{
    Q_D(const Device);
    /* clang-format off */
    return !(d->connectionState == NetworkManager::Device::Unavailable
             || d->connectionState == NetworkManager::Device::Unmanaged
             || d->connectionState == NetworkManager::Device::Disconnected
             || d->connectionState == NetworkManager::Device::Failed);
    /* clang-format on */
}

bool NetworkManager::Device::isValid() const
{
    Q_D(const Device);
    return d->deviceIface.isValid();
}

bool NetworkManager::Device::managed() const
{
    Q_D(const Device);
    return d->managed;
}

NetworkManager::Device::Interfaceflags NetworkManager::Device::interfaceFlags() const
{
    Q_D(const Device);
    return d->interfaceFlags;
}

uint NetworkManager::Device::mtu() const
{
    Q_D(const Device);
    return d->mtu;
}

bool NetworkManager::Device::nmPluginMissing() const
{
    Q_D(const Device);
    return d->nmPluginMissing;
}

NetworkManager::Device::MeteredStatus NetworkManager::Device::metered() const
{
    Q_D(const Device);
    return d->metered;
}

QDBusPendingReply<> NetworkManager::Device::reapplyConnection(const NMVariantMapMap &connection, qulonglong version_id, uint flags)
{
    Q_D(Device);
    return d->deviceIface.Reapply(connection, version_id, flags);
}

QDBusPendingReply<> NetworkManager::Device::disconnectInterface()
{
    Q_D(Device);
    return d->deviceIface.Disconnect();
}

QDBusPendingReply<> NetworkManager::Device::deleteInterface()
{
    if (NetworkManager::checkVersion(1, 0, 0)) {
        Q_D(Device);
        return d->deviceIface.Delete();
    } else {
        return QDBusPendingReply<>();
    }
}

NetworkManager::Device::State NetworkManager::Device::state() const
{
    Q_D(const Device);
    return d->connectionState;
}

int NetworkManager::Device::designSpeed() const
{
    Q_D(const Device);
    return d->designSpeed;
}

NetworkManager::Device::Capabilities NetworkManager::Device::capabilities() const
{
    Q_D(const Device);
    return d->capabilities;
}

QVariant NetworkManager::Device::capabilitiesV() const
{
    Q_D(const Device);
    return QVariant(d->capabilities);
}

NetworkManager::DeviceStatistics::Ptr NetworkManager::Device::deviceStatistics() const
{
    Q_D(const Device);
    return d->deviceStatistics;
}

void NetworkManager::DevicePrivate::deviceStateChanged(uint newState, uint oldState, uint reason)
{
    Q_Q(Device);
    connectionState = NetworkManager::DevicePrivate::convertState(newState);
    reason = NetworkManager::DevicePrivate::convertReason(reason);

    Q_EMIT q->stateChanged(connectionState, NetworkManager::DevicePrivate::convertState(oldState), NetworkManager::DevicePrivate::convertReason(reason));
}

void NetworkManager::DevicePrivate::dbusPropertiesChanged(const QString &interfaceName, const QVariantMap &properties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);
    if (interfaceName.contains(QLatin1String("org.freedesktop.NetworkManager.Device"))
        && interfaceName != QLatin1String("org.freedesktop.NetworkManager.Device.Statistics")) {
        propertiesChanged(properties);
    }
}

void NetworkManager::DevicePrivate::propertiesChanged(const QVariantMap &properties)
{
    // qCDebug(NMQT) << Q_FUNC_INFO << properties;

    QVariantMap::const_iterator it = properties.constBegin();
    while (it != properties.constEnd()) {
        propertyChanged(it.key(), it.value());
        ++it;
    }

    // FIXME workaround, we need to get a path to updated IPv[46]Config,
    // because NM doesn't Q_EMIT the updated value when the device is activated
    // BUG: https://bugzilla.gnome.org/show_bug.cgi?id=725657
    if (properties.contains(QLatin1String("State")) && connectionState == NetworkManager::Device::Activated) {
        propertyChanged(QLatin1String("Ip4Config"), QVariant::fromValue<QDBusObjectPath>(deviceIface.ip4Config()));
        propertyChanged(QLatin1String("Ip6Config"), QVariant::fromValue<QDBusObjectPath>(deviceIface.ip6Config()));
    }
}

NetworkManager::Device::Type NetworkManager::Device::type() const
{
    Q_D(const Device);
    return d->deviceType;
}

#include "moc_device.cpp"
#include "moc_device_p.cpp"
