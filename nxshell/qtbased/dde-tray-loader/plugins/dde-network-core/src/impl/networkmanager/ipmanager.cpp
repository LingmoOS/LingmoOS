// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ipmanager.h"

#include <QDBusMetaType>
#include <QDBusConnection>
#include <QDBusInterface>

#include <NetworkManagerQt/IpConfig>

using namespace dde::network;

IpManager::IpManager(NetworkManager::Device::Ptr device, QObject *parent)
    : QObject (parent)
    , m_device(device)
{
    qRegisterMetaType<QList<QVariantMap>>("QList<QVariantMap>");
    qDBusRegisterMetaType<QList<QVariantMap>>();

    QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", device->uni(), "org.freedesktop.DBus.Properties", "PropertiesChanged",
                                         this, SLOT(onDevicePropertiesChanged(const QString &, const QVariantMap &, const QStringList &)));

    QDBusInterface dbus("org.freedesktop.NetworkManager", device->uni(), "org.freedesktop.NetworkManager.Device", QDBusConnection::systemBus());
    changeIpv4Config(dbus.property("Ip4Config").value<QDBusObjectPath>().path());
}

NetworkManager::IpAddresses IpManager::ipAddresses() const
{
    return m_ipAddresses;
}

bool IpManager::changeIpv4Config(const QString &uni)
{
    if (uni.isEmpty() || uni == "/")
        return false;

    QDBusConnection::systemBus().connect("org.freedesktop.NetworkManager", uni, "org.freedesktop.DBus.Properties",
                                            "PropertiesChanged", this, SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
    NetworkManager::IpConfig ipConfig;
    ipConfig.setIPv4Path(uni);
    m_ipAddresses = ipConfig.addresses();
    return true;
}

void IpManager::onPropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);

    if (interfaceName != "org.freedesktop.NetworkManager.IP4Config")
        return;

    if (changedProperties.contains("AddressData")) {
        QHostAddress addr;

        NetworkManager::IpAddresses addresses;
        QList<QVariantMap> addressDatas = qdbus_cast<QList<QVariantMap>>(changedProperties.value("AddressData").value<QDBusArgument>());
        for (QVariantMap addressData : addressDatas) {
            NetworkManager::IpAddress addr;
            addr.setIp(QHostAddress(addressData.value("address").toString()));
            addr.setPrefixLength(addressData.value("prefix").toInt());
            addresses << addr;
        }
        if (addresses.size() != m_ipAddresses.size()) {
            m_ipAddresses = addresses;
            emit ipChanged();
        } else {
            bool changed = false;
            for (const NetworkManager::IpAddress &address : addresses) {
                if (m_ipAddresses.contains(address))
                    continue;

                changed = true;
                break;
            }

            if (changed) {
                // 如果有IP不一致，则认为IP发生了变化，发送IP变化的信号
                m_ipAddresses = addresses;
                emit ipChanged();
            }
        }
    }
}

void IpManager::onDevicePropertiesChanged(const QString &interfaceName, const QVariantMap &changedProperties, const QStringList &invalidatedProperties)
{
    Q_UNUSED(invalidatedProperties);

    if (!interfaceName.startsWith("org.freedesktop.NetworkManager.Device"))
        return;

    if (changedProperties.contains("Ip4Config")) {
        QDBusObjectPath ipv4ConfigPath = changedProperties.value("Ip4Config").value<QDBusObjectPath>();
        if (changeIpv4Config(ipv4ConfigPath.path()))
            emit ipChanged();
    }
}
