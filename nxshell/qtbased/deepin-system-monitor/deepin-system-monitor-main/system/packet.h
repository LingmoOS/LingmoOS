// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PACKET_H
#define PACKET_H

#include <QQueue>
#include <QSharedPointer>

#include <memory>

#include <pcap.h>
#include <time.h>
#include <netinet/in.h>

namespace core {
namespace system {

enum packet_direction {
    kInboundPacket,  // download
    kOutboundPacket, // upload
};

struct packet_payload_t {
    struct timeval ts;
    packet_direction direction;
    ino_t ino;
    int sa_family;
    uint proto;
    union {
        in_addr in4;
        in6_addr in6;
    } s_addr;

    uint16_t s_port;
    union {
        in_addr in4;
        in6_addr in6;
    } d_addr;

    uint16_t d_port;
    unsigned long long payload;
};
struct net_ifaddr_t {
    char iface[16]; // interface name
    int family; // address family
    // flags
    union {
        in_addr in4;
        in6_addr in6;
    } addr; // address
    // netmask
    // broadcast/p2p
};

// from /proc/net/tcp & /proc/net/udp & /proc/net/tcp6 & /proc/net/udp6
struct sock_stat_t {
    ino_t   ino;              // socket inode
    int     sa_family;        // AF_INET & AF_INET6
    int     proto;            // IPPROTO_TCP & IPPROTO_UDP
    union {
        in_addr in4;
        in6_addr in6;
    }       s_addr;           // source address
    uint    s_port;           // source port
    union {
        in_addr in4;
        in6_addr in6;
    }       d_addr;           // remote address
    uint    d_port;           // remote port
    uid_t   uid;              // socket uid
};

using PacketPayload      = QSharedPointer<struct packet_payload_t>;
using PacketPayloadQueue = QQueue<PacketPayload>;
using SockStat      = QSharedPointer<struct sock_stat_t>;
using SockStatMap   = QMultiMap<uint64_t, SockStat>; // [hash, SockStat]
using NetIFAddr     = QSharedPointer<struct net_ifaddr_t>;
using NetIFAddrsMap = QMultiMap<QString, NetIFAddr>;

} // namespace system
} // namespace core

#endif // PACKET_H
