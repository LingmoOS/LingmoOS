/*
    SPDX-FileCopyrightText: 2012-2013 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "olpcmeshsetting.h"
#include "olpcmeshsetting_p.h"

#include <QDebug>

NetworkManager::OlpcMeshSettingPrivate::OlpcMeshSettingPrivate()
    : name(NM_SETTING_OLPC_MESH_SETTING_NAME)
    , channel(0)
{
}

NetworkManager::OlpcMeshSetting::OlpcMeshSetting()
    : Setting(Setting::Bond)
    , d_ptr(new OlpcMeshSettingPrivate())
{
}

NetworkManager::OlpcMeshSetting::OlpcMeshSetting(const Ptr &other)
    : Setting(other)
    , d_ptr(new OlpcMeshSettingPrivate())
{
    setSsid(other->ssid());
    setChannel(other->channel());
    setDhcpAnycastAddress(other->dhcpAnycastAddress());
}

NetworkManager::OlpcMeshSetting::~OlpcMeshSetting()
{
    delete d_ptr;
}

QString NetworkManager::OlpcMeshSetting::name() const
{
    Q_D(const OlpcMeshSetting);

    return d->name;
}

void NetworkManager::OlpcMeshSetting::setSsid(const QByteArray &ssid)
{
    Q_D(OlpcMeshSetting);

    d->ssid = ssid;
}

QByteArray NetworkManager::OlpcMeshSetting::ssid() const
{
    Q_D(const OlpcMeshSetting);

    return d->ssid;
}

void NetworkManager::OlpcMeshSetting::setChannel(quint32 channel)
{
    Q_D(OlpcMeshSetting);

    d->channel = channel;
}

quint32 NetworkManager::OlpcMeshSetting::channel() const
{
    Q_D(const OlpcMeshSetting);

    return d->channel;
}

void NetworkManager::OlpcMeshSetting::setDhcpAnycastAddress(const QByteArray &address)
{
    Q_D(OlpcMeshSetting);

    d->dhcpAnycastAddress = address;
}

QByteArray NetworkManager::OlpcMeshSetting::dhcpAnycastAddress() const
{
    Q_D(const OlpcMeshSetting);

    return d->dhcpAnycastAddress;
}

void NetworkManager::OlpcMeshSetting::fromMap(const QVariantMap &setting)
{
    if (setting.contains(QLatin1String(NM_SETTING_OLPC_MESH_SSID))) {
        setSsid(setting.value(QLatin1String(NM_SETTING_OLPC_MESH_SSID)).toByteArray());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OLPC_MESH_CHANNEL))) {
        setChannel(setting.value(QLatin1String(NM_SETTING_OLPC_MESH_CHANNEL)).toUInt());
    }

    if (setting.contains(QLatin1String(NM_SETTING_OLPC_MESH_DHCP_ANYCAST_ADDRESS))) {
        setDhcpAnycastAddress(setting.value(QLatin1String(NM_SETTING_OLPC_MESH_DHCP_ANYCAST_ADDRESS)).toByteArray());
    }
}

QVariantMap NetworkManager::OlpcMeshSetting::toMap() const
{
    QVariantMap setting;

    if (!ssid().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OLPC_MESH_SSID), ssid());
    }

    if (channel()) {
        setting.insert(QLatin1String(NM_SETTING_OLPC_MESH_CHANNEL), channel());
    }

    if (!dhcpAnycastAddress().isEmpty()) {
        setting.insert(QLatin1String(NM_SETTING_OLPC_MESH_DHCP_ANYCAST_ADDRESS), dhcpAnycastAddress());
    }

    return setting;
}

QDebug NetworkManager::operator<<(QDebug dbg, const NetworkManager::OlpcMeshSetting &setting)
{
    dbg.nospace() << "type: " << setting.typeAsString(setting.type()) << '\n';
    dbg.nospace() << "initialized: " << !setting.isNull() << '\n';

    dbg.nospace() << NM_SETTING_OLPC_MESH_SSID << ": " << setting.ssid() << '\n';
    dbg.nospace() << NM_SETTING_OLPC_MESH_CHANNEL << ": " << setting.channel() << '\n';
    dbg.nospace() << NM_SETTING_OLPC_MESH_DHCP_ANYCAST_ADDRESS << ": " << setting.dhcpAnycastAddress() << '\n';

    return dbg.maybeSpace();
}
