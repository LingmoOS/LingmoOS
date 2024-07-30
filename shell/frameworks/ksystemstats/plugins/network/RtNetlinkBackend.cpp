/*
 * SPDX-FileCopyrightText: 2020 Arjen Hiemstra <ahiemstra@heimr.nl>
 * SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>
 * SPDX-FileCopyrightText: 2021 Alessio Bonfiglio <alessio.bonfiglio@mail.polimi.it>
 *
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#include "RtNetlinkBackend.h"

#include <systemstats/SysFsSensor.h>

#include <QNetworkAddressEntry>
#include <QHostAddress>
#include <QString>
#include <array>

#include <netlink/netlink.h>
#include <netlink/route/addr.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>

#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_link.h>
#include <linux/rtnetlink.h>

static const QString devicesFolder = QStringLiteral("/sys/class/net");

RtNetlinkDevice::RtNetlinkDevice(const QString &id)
    : NetworkDevice(id, id)
{
    // Even though we have no sensor, we need to have a name for the grouped text on the front page
    // of plasma-systemmonitor
    m_networkSensor->setValue(id);

    std::array<KSysGuard::SensorProperty*, 6> statisticSensors {m_downloadSensor, m_downloadBitsSensor, m_totalDownloadSensor, m_uploadSensor, m_uploadBitsSensor, m_totalUploadSensor};
    auto resetStatistics = [this, statisticSensors]() {
        if (std::none_of(statisticSensors.begin(), statisticSensors.end(), [](auto property) {return property->isSubscribed();})) {
            m_totalDownloadSensor->setValue(0);
            m_totalUploadSensor->setValue(0);
        }
    };
    for (auto property : statisticSensors) {
        connect(property, &KSysGuard::SensorProperty::subscribedChanged, this, resetStatistics);
    }
    connect(this, &RtNetlinkDevice::disconnected, this, resetStatistics);

    // FIXME: find the currently used dns servers
    m_ipv4DNSSensor->setValue(QString{});
    m_ipv6DNSSensor->setValue(QString{});
}

void RtNetlinkDevice::update(rtnl_link *link, nl_cache *address_cache, nl_cache *route_cache, qint64 elapsedTime)
{
    const bool isConnected = rtnl_link_get_operstate(link) == IF_OPER_UP;
    if (isConnected && !m_connected) {
        m_connected = isConnected;
        Q_EMIT connected();
    } else if (!isConnected && m_connected) {
        m_connected = isConnected;
        Q_EMIT disconnected();
    }

    if (!m_connected || !isSubscribed()) {
        return;
    }

    const qulonglong downloadedBytes = rtnl_link_get_stat(link, RTNL_LINK_RX_BYTES);
    const qulonglong previousDownloadedBytes = m_totalDownloadSensor->value().toULongLong();
    if (previousDownloadedBytes != 0) {
        m_downloadSensor->setValue((downloadedBytes - previousDownloadedBytes) * 1000 / elapsedTime);
        m_downloadBitsSensor->setValue((downloadedBytes - previousDownloadedBytes) * 1000 / elapsedTime * 8);
    }
    m_totalDownloadSensor->setValue(downloadedBytes);

    const qulonglong uploadedBytes = rtnl_link_get_stat(link, RTNL_LINK_TX_BYTES);
    const qulonglong previousUploadedBytes = m_totalUploadSensor->value().toULongLong();
    if (previousUploadedBytes != 0) {
        m_uploadSensor->setValue((uploadedBytes - previousUploadedBytes) * 1000 / elapsedTime);
        m_uploadBitsSensor->setValue((uploadedBytes - previousUploadedBytes) * 1000 / elapsedTime * 8);
    }
    m_totalUploadSensor->setValue(uploadedBytes);

    m_ipv4Sensor->setValue(QString());
    m_ipv4SubnetMaskSensor->setValue(QString{});
    m_ipv4WithPrefixLengthSensor->setValue(QString{});
    m_ipv6Sensor->setValue(QString());
    m_ipv6SubnetMaskSensor->setValue(QString{});
    m_ipv6WithPrefixLengthSensor->setValue(QString{});
    auto filterAddress = rtnl_addr_alloc();
    rtnl_addr_set_ifindex(filterAddress, rtnl_link_get_ifindex(link));
    nl_cache_foreach_filter(address_cache, reinterpret_cast<nl_object*>(filterAddress), [] (nl_object *object, void *arg) {
        auto self = static_cast<RtNetlinkDevice *>(arg);
        rtnl_addr *address = reinterpret_cast<rtnl_addr *>(object);
        int prefixLen = rtnl_addr_get_prefixlen(address);
        auto dummyAddress = QNetworkAddressEntry(); // conveniently used to compute the subnet mask
        if (rtnl_addr_get_family(address) == AF_INET) {
            if(self->m_ipv4Sensor->value().toString().isEmpty()) {
                char buffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET, nl_addr_get_binary_addr(rtnl_addr_get_local(address)), buffer, INET_ADDRSTRLEN);
                auto ipv4 = QString::fromLatin1(buffer);
                self->m_ipv4Sensor->setValue(ipv4);
                if(self->m_ipv4WithPrefixLengthSensor->value().toString().isEmpty()) {
                    self->m_ipv4WithPrefixLengthSensor->setValue(static_cast<QString>(ipv4 + '/' + QString::number(prefixLen)));
                }
            }
            if(self->m_ipv4SubnetMaskSensor->value().toString().isEmpty()) {
                dummyAddress.setIp(QHostAddress::AnyIPv4);
                dummyAddress.setPrefixLength(prefixLen);
                self->m_ipv4SubnetMaskSensor->setValue(dummyAddress.netmask().toString());
            }
        } else if (rtnl_addr_get_family(address) == AF_INET6) {
            if(self->m_ipv6Sensor->value().toString().isEmpty()) {
                char buffer[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, nl_addr_get_binary_addr(rtnl_addr_get_local(address)), buffer, INET6_ADDRSTRLEN);
                auto ipv6 = QString::fromLatin1(buffer);
                self->m_ipv6Sensor->setValue(ipv6);
                if(self->m_ipv6WithPrefixLengthSensor->value().toString().isEmpty()) {
                    self->m_ipv6WithPrefixLengthSensor->setValue(static_cast<QString>(ipv6 + '/' + QString::number(prefixLen)));
                }
            }
            if(self->m_ipv6SubnetMaskSensor->value().toString().isEmpty()) {
                dummyAddress.setIp(QHostAddress::AnyIPv6);
                dummyAddress.setPrefixLength(prefixLen);
                self->m_ipv6SubnetMaskSensor->setValue(dummyAddress.netmask().toString());
            }
        }
    }, this);

    m_ipv4GatewaySensor->setValue(QString());
    m_ipv6GatewaySensor->setValue(QString());
    // The gateway is found using a filter on the destination address with size = 0
    auto dst = nl_addr_build(AF_INET, NULL, 0);
    auto routeFilter = rtnl_route_alloc();
    rtnl_route_set_iif(routeFilter, rtnl_link_get_ifindex(link));
    rtnl_route_set_dst(routeFilter, dst);

    nl_cache_foreach_filter(route_cache, reinterpret_cast<nl_object*>(routeFilter), [] (nl_object *object, void *arg) {
        auto self = static_cast<RtNetlinkDevice *>(arg);
        auto route = reinterpret_cast<rtnl_route *>(object);
        if (rtnl_route_get_family(route) == AF_INET && self->m_ipv4GatewaySensor->value().toString().isEmpty()) {
            char buffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET, nl_addr_get_binary_addr(rtnl_route_nh_get_gateway(rtnl_route_nexthop_n(route,0))), buffer, INET_ADDRSTRLEN);
            self->m_ipv4GatewaySensor->setValue(QString::fromLatin1(buffer));
        } else if (rtnl_route_get_family(route) == AF_INET6 && self->m_ipv6GatewaySensor->value().toString().isEmpty()) {
            char buffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, nl_addr_get_binary_addr(rtnl_route_nh_get_gateway(rtnl_route_nexthop_n(route,0))), buffer, INET6_ADDRSTRLEN);
            self->m_ipv6GatewaySensor->setValue(QString::fromLatin1(buffer));
        }
    }, this);

    rtnl_addr_put(filterAddress);
    nl_addr_put(dst);
    rtnl_route_put(routeFilter);
}

RtNetlinkBackend::RtNetlinkBackend(QObject *parent)
    : NetworkBackend(parent)
    , m_socket(nl_socket_alloc(), nl_socket_free)
{
    nl_connect(m_socket.get(), NETLINK_ROUTE);
}

RtNetlinkBackend::~RtNetlinkBackend()
{
    qDeleteAll(m_devices);
}

bool RtNetlinkBackend::isSupported()
{
    return bool(m_socket);
}

void RtNetlinkBackend::start()
{
    if (!m_socket) {
        return;
    }
    update();
}

void RtNetlinkBackend::stop()
{
}

void RtNetlinkBackend::update()
{
    const qint64 elapsedTime = m_updateTimer.restart();
    nl_cache *link_cache, *address_cache, *route_cache;
    int error = rtnl_link_alloc_cache(m_socket.get(), AF_UNSPEC, &link_cache);
    if (error != 0) {
        qWarning() << nl_geterror(error);
        return;
    }
    error = rtnl_addr_alloc_cache(m_socket.get(), &address_cache);
    if (error != 0) {
        qWarning() << nl_geterror(error);
        return;
    }
    error = rtnl_route_alloc_cache(m_socket.get(), AF_UNSPEC, 0, &route_cache);
    if (error != 0) {
        qWarning() << nl_geterror(error);
        return;
    }

    for (nl_object *object = nl_cache_get_first(link_cache); object != nullptr; object = nl_cache_get_next(object)) {
        auto link = reinterpret_cast<rtnl_link *>(object);
        if (rtnl_link_get_arptype(link) != ARPHRD_ETHER) {
            // FIXME Maybe this is to aggresive? On my machines wifi is also ether
            continue;
        }
        // Hardware devices do have an empty type
        if (qstrlen(rtnl_link_get_type(link)) != 0) {
            continue;
        }
        const auto name = QByteArray(rtnl_link_get_name(link));
        if (!m_devices.contains(name)) {
            auto device = new RtNetlinkDevice(name);
            m_devices.insert(name, device);
            connect(device, &RtNetlinkDevice::connected, this, [device, this] { Q_EMIT deviceAdded(device); });
            connect(device, &RtNetlinkDevice::disconnected, this, [device, this] { Q_EMIT deviceRemoved(device); });
        }
        m_devices[name]->update(link, address_cache, route_cache, elapsedTime);
    }
    nl_cache_free(link_cache);
    nl_cache_free(address_cache);
    nl_cache_free(route_cache);
}

#include "moc_RtNetlinkBackend.cpp"
