/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "setting.h"

#undef signals
#include <nm-version.h>
#define signals Q_SIGNALS

#include <QDebug>

#if !NM_CHECK_VERSION(1, 16, 0)
#define NM_SETTING_WIREGUARD_SETTING_NAME "wireguard"
#endif

#if !NM_CHECK_VERSION(1, 14, 0)
#define NM_SETTING_MATCH_SETTING_NAME "match"
#endif

#if !NM_CHECK_VERSION(1, 10, 0)
#define NM_SETTING_OVS_BRIDGE_SETTING_NAME "ovs-bridge"
#define NM_SETTING_OVS_INTERFACE_SETTING_NAME "ovs-interface"
#define NM_SETTING_OVS_PATCH_SETTING_NAME "ovs-patch"
#define NM_SETTING_OVS_PORT_SETTING_NAME "ovs-port"
#define NM_SETTING_TC_CONFIG_SETTING_NAME "tc"
#define NM_SETTING_TEAM_PORT_SETTING_NAME "team-port"
#endif

#if !NM_CHECK_VERSION(1, 8, 0)
#define NM_SETTING_USER_SETTING_NAME "user"
#endif

#if !NM_CHECK_VERSION(1, 6, 0)
#define NM_SETTING_MACSEC_SETTING_NAME "macsec"
#define NM_SETTING_PROXY_SETTING_NAME "proxy"
#endif

namespace NetworkManager
{
class SettingPrivate
{
public:
    SettingPrivate();

    Setting::SettingType type;
    bool initialized;
};

QDebug operator<<(QDebug dbg, const Setting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';
    return dbg.maybeSpace();
}

}

NetworkManager::SettingPrivate::SettingPrivate()
    : type(Setting::Wired)
    , initialized(false)
{
}

QString NetworkManager::Setting::typeAsString(NetworkManager::Setting::SettingType type)
{
    QString typeString;

    switch (type) {
    case Cdma:
        typeString = QLatin1String(NM_SETTING_CDMA_SETTING_NAME);
        break;
    case Gsm:
        typeString = QLatin1String(NM_SETTING_GSM_SETTING_NAME);
        break;
    case Bluetooth:
        typeString = QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME);
        break;
    case Ipv4:
        typeString = QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME);
        break;
    case Ipv6:
        typeString = QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME);
        break;
    case Ppp:
        typeString = QLatin1String(NM_SETTING_PPP_SETTING_NAME);
        break;
    case Pppoe:
        typeString = QLatin1String(NM_SETTING_PPPOE_SETTING_NAME);
        break;
    case Security8021x:
        typeString = QLatin1String(NM_SETTING_802_1X_SETTING_NAME);
        break;
    case Serial:
        typeString = QLatin1String(NM_SETTING_SERIAL_SETTING_NAME);
        break;
    case Vpn:
        typeString = QLatin1String(NM_SETTING_VPN_SETTING_NAME);
        break;
    case Wired:
        typeString = QLatin1String(NM_SETTING_WIRED_SETTING_NAME);
        break;
    case Wireless:
        typeString = QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME);
        break;
    case WirelessSecurity:
        typeString = QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME);
        break;
    case OlpcMesh:
        typeString = QLatin1String(NM_SETTING_OLPC_MESH_SETTING_NAME);
        break;
    case Vlan:
        typeString = QLatin1String(NM_SETTING_VLAN_SETTING_NAME);
        break;
    case Wimax:
        typeString = QLatin1String(NM_SETTING_WIMAX_SETTING_NAME);
        break;
    case Bond:
        typeString = QLatin1String(NM_SETTING_BOND_SETTING_NAME);
        break;
    case Bridge:
        typeString = QLatin1String(NM_SETTING_BRIDGE_SETTING_NAME);
        break;
    case Team:
        typeString = QLatin1String(NM_SETTING_TEAM_SETTING_NAME);
        break;
    case Vxlan:
        typeString = QLatin1String(NM_SETTING_VXLAN_SETTING_NAME);
        break;
    case IpTunnel:
        typeString = QLatin1String(NM_SETTING_IP_TUNNEL_SETTING_NAME);
        break;
    case Proxy:
        typeString = QLatin1String(NM_SETTING_PROXY_SETTING_NAME);
        break;
    case User:
        typeString = QLatin1String(NM_SETTING_USER_SETTING_NAME);
        break;
    case OvsInterface:
        typeString = QLatin1String(NM_SETTING_OVS_INTERFACE_SETTING_NAME);
        break;
    case OvsBridge:
        typeString = QLatin1String(NM_SETTING_OVS_BRIDGE_SETTING_NAME);
        break;
    case OvsPatch:
        typeString = QLatin1String(NM_SETTING_OVS_PATCH_SETTING_NAME);
        break;
    case OvsPort:
        typeString = QLatin1String(NM_SETTING_OVS_PORT_SETTING_NAME);
        break;
    case Match:
        typeString = QLatin1String(NM_SETTING_MATCH_SETTING_NAME);
        break;
    case Tc:
        typeString = QLatin1String(NM_SETTING_TC_CONFIG_SETTING_NAME);
        break;
    case TeamPort:
        typeString = QLatin1String(NM_SETTING_TEAM_PORT_SETTING_NAME);
        break;
    case Macsec:
        typeString = QLatin1String(NM_SETTING_MACSEC_SETTING_NAME);
        break;
    case Dcb:
        typeString = QLatin1String(NM_SETTING_DCB_SETTING_NAME);
        break;
    case NetworkManager::Setting::WireGuard:
        typeString = QLatin1String(NM_SETTING_WIREGUARD_SETTING_NAME);
        break;
    case NetworkManager::Setting::Generic:
        typeString = QLatin1String(NM_SETTING_GENERIC_SETTING_NAME);
        break;
    default:
        break;
    }

    return typeString;
}

NetworkManager::Setting::SettingType NetworkManager::Setting::typeFromString(const QString &typeString)
{
    SettingType type = Wired;

    if (typeString == QLatin1String(NM_SETTING_CDMA_SETTING_NAME)) {
        type = Cdma;
    } else if (typeString == QLatin1String(NM_SETTING_GSM_SETTING_NAME)) {
        type = Gsm;
    } else if (typeString == QLatin1String(NM_SETTING_BLUETOOTH_SETTING_NAME)) {
        type = Bluetooth;
    } else if (typeString == QLatin1String(NM_SETTING_IP4_CONFIG_SETTING_NAME)) {
        type = Ipv4;
    } else if (typeString == QLatin1String(NM_SETTING_IP6_CONFIG_SETTING_NAME)) {
        type = Ipv6;
    } else if (typeString == QLatin1String(NM_SETTING_PPP_SETTING_NAME)) {
        type = Ppp;
    } else if (typeString == QLatin1String(NM_SETTING_PPPOE_SETTING_NAME)) {
        type = Pppoe;
    } else if (typeString == QLatin1String(NM_SETTING_SERIAL_SETTING_NAME)) {
        type = Serial;
    } else if (typeString == QLatin1String(NM_SETTING_802_1X_SETTING_NAME)) {
        type = Security8021x;
    } else if (typeString == QLatin1String(NM_SETTING_VPN_SETTING_NAME)) {
        type = Vpn;
    } else if (typeString == QLatin1String(NM_SETTING_WIRED_SETTING_NAME)) {
        type = Wired;
    } else if (typeString == QLatin1String(NM_SETTING_WIRELESS_SETTING_NAME)) {
        type = Wireless;
    } else if (typeString == QLatin1String(NM_SETTING_WIRELESS_SECURITY_SETTING_NAME)) {
        type = WirelessSecurity;
    } else if (typeString == QLatin1String(NM_SETTING_OLPC_MESH_SETTING_NAME)) {
        type = OlpcMesh;
    } else if (typeString == QLatin1String(NM_SETTING_VLAN_SETTING_NAME)) {
        type = Vlan;
    } else if (typeString == QLatin1String(NM_SETTING_WIMAX_SETTING_NAME)) {
        type = Wimax;
    } else if (typeString == QLatin1String(NM_SETTING_BOND_SETTING_NAME)) {
        type = Bond;
    } else if (typeString == QLatin1String(NM_SETTING_BRIDGE_SETTING_NAME)) {
        type = Bridge;
    } else if (typeString == QLatin1String(NM_SETTING_TEAM_SETTING_NAME)) {
        type = Team;
    } else if (typeString == QLatin1String(NM_SETTING_VXLAN_SETTING_NAME)) {
        type = Vxlan;
    } else if (typeString == QLatin1String(NM_SETTING_IP_TUNNEL_SETTING_NAME)) {
        type = IpTunnel;
    } else if (typeString == QLatin1String(NM_SETTING_GENERIC_SETTING_NAME)) {
        type = Generic;
    } else if (typeString == QLatin1String(NM_SETTING_USER_SETTING_NAME)) {
        type = User;
    } else if (typeString == QLatin1String(NM_SETTING_PROXY_SETTING_NAME)) {
        type = Proxy;
    } else if (typeString == QLatin1String(NM_SETTING_OVS_INTERFACE_SETTING_NAME)) {
        type = OvsInterface;
    } else if (typeString == QLatin1String(NM_SETTING_OVS_BRIDGE_SETTING_NAME)) {
        type = OvsBridge;
    } else if (typeString == QLatin1String(NM_SETTING_OVS_PATCH_SETTING_NAME)) {
        type = OvsPatch;
    } else if (typeString == QLatin1String(NM_SETTING_OVS_PORT_SETTING_NAME)) {
        type = OvsPort;
    } else if (typeString == QLatin1String(NM_SETTING_MATCH_SETTING_NAME)) {
        type = Match;
    } else if (typeString == QLatin1String(NM_SETTING_TC_CONFIG_SETTING_NAME)) {
        type = Tc;
    } else if (typeString == QLatin1String(NM_SETTING_TEAM_PORT_SETTING_NAME)) {
        type = TeamPort;
    } else if (typeString == QLatin1String(NM_SETTING_MACSEC_SETTING_NAME)) {
        type = Macsec;
    } else if (typeString == QLatin1String(NM_SETTING_DCB_SETTING_NAME)) {
        type = Dcb;
    } else if (typeString == QLatin1String(NM_SETTING_WIREGUARD_SETTING_NAME)) {
        type = WireGuard;
    }

    return type;
}

NetworkManager::Setting::Setting(SettingType type)
    : d_ptr(new SettingPrivate())
{
    setType(type);
}

NetworkManager::Setting::Setting(const NetworkManager::Setting::Ptr &setting)
    : d_ptr(new SettingPrivate())
{
    setInitialized(!setting->isNull());
    setType(setting->type());
}

NetworkManager::Setting::~Setting()
{
    delete d_ptr;
}

void NetworkManager::Setting::fromMap(const QVariantMap &map)
{
    Q_UNUSED(map);
}

QVariantMap NetworkManager::Setting::toMap() const
{
    return QVariantMap();
}

QStringList NetworkManager::Setting::needSecrets(bool requestNew) const
{
    Q_UNUSED(requestNew);
    return QStringList();
}

QString NetworkManager::Setting::name() const
{
    return QString();
}

void NetworkManager::Setting::secretsFromMap(const QVariantMap &map)
{
    Q_UNUSED(map);
}

void NetworkManager::Setting::secretsFromStringMap(const NMStringMap &map)
{
    QVariantMap secretsMap;
    NMStringMap::ConstIterator i = map.constBegin();
    while (i != map.constEnd()) {
        secretsMap.insert(i.key(), i.value());
        ++i;
    }
    secretsFromMap(secretsMap);
}

QVariantMap NetworkManager::Setting::secretsToMap() const
{
    return QVariantMap();
}

NMStringMap NetworkManager::Setting::secretsToStringMap() const
{
    NMStringMap ret;
    QVariantMap secretsMap = secretsToMap();
    QVariantMap::ConstIterator i = secretsMap.constBegin();
    while (i != secretsMap.constEnd()) {
        ret.insert(i.key(), i.value().toString());
        ++i;
    }

    return ret;
}

void NetworkManager::Setting::setInitialized(bool initialized)
{
    Q_D(Setting);

    d->initialized = initialized;
}

bool NetworkManager::Setting::isNull() const
{
    Q_D(const Setting);

    return !d->initialized;
}

void NetworkManager::Setting::setType(NetworkManager::Setting::SettingType type)
{
    Q_D(Setting);

    d->type = type;
}

NetworkManager::Setting::SettingType NetworkManager::Setting::type() const
{
    Q_D(const Setting);

    return d->type;
}
