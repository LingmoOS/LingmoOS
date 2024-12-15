// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_INFO_DB_H
#define NETIF_INFO_DB_H

#include "netif.h"
#include "netlink.h"

#include <QMultiMap>
#include <QMap>

#include "netif_monitor.h"
#include <memory>

namespace core {
namespace system {

struct netif_stat {
    unsigned long long rx_bytes; // received bytes
    unsigned long long tx_bytes; // transmitted bytes
    char iface[IF_NAMESIZE]; // interface name
};

class NetifInfoDB
{
    enum StatIndex { kLastStat = 0, kCurrentStat = 1, kStatCount = kCurrentStat + 1 };

public:
    explicit NetifInfoDB();
    virtual ~NetifInfoDB() = default;

    QMap<QByteArray, NetifInfoPtr> infoDB();
    void update();

protected:
    void update_addr();
    void update_netif_info();

private:
    std::unique_ptr<Netlink> m_netlink;

    QMultiMap<int, INet4Addr> m_addrIpv4DB;
    QMultiMap<int, INet6Addr> m_addrIpv6DB;

    QMap<QByteArray, NetifInfoPtr> m_infoDB;

    QMap<ino_t, SockIOStat> m_sockIOStatMap;


    timeval timevalList[kStatCount] = {timeval{0, 0}, timeval{0, 0}};
    QSharedPointer<struct netif_stat> m_netStat[kStatCount] {{}, {}};
};

inline QMap<QByteArray, NetifInfoPtr> NetifInfoDB::infoDB()
{
    return m_infoDB;
}

} // namespace system
} // namespace core

#endif // NETIF_INFO_DB_H
