// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_P_H
#define NETIF_P_H

// qt
#include <QSharedData>

// c++11
#include <memory>

// sys
#include <netinet/in.h>
#include <net/if.h>
#include <linux/if_arp.h>
#include <linux/wireless.h>
#include <sys/types.h>

#define ESSID_MAX_SIZE 31

namespace core {
namespace system {

struct inet_addr_t;
using INetAddr = std::shared_ptr<struct inet_addr_t>;
using INet4Addr = std::shared_ptr<struct inet_addr4_t>;
using INet6Addr = std::shared_ptr<struct inet_addr6_t>;
struct iw_info_t {
    QByteArray essid;
    struct iw_freq freq {
        0, 0, 0, 0
    };
    struct iw_quality qual {
        0, 0, 0, 0
    };
    // bitrate
    // mode
    // ap/cell
    // txpower
};

class NetifInfo;

class NetifInfoPrivate : public QSharedData
{
public:
    NetifInfoPrivate()
        : QSharedData()
        , index {0}
        , carrier {0}
        , oper_stat {0}
        , link_mode {0}
        , mtu {0}
        , flags {0}
        , arp_type {0}
        , txqlen {0}
        , carrier_changes {0}
        , speed {0}
        , ifname {}
        , alias {}
        , conn_type {}
        , hw_addr {}
        , hw_bcast {}
        , brand {}
        , recv_bps {0}
        , sent_bps {0}
        , addr4infolst {}
        , addr6infolst {}
        , iw_info(std::unique_ptr<iw_info_t>(new iw_info_t()))
        , rx_packets {0}
        , rx_bytes {0}
        , rx_errors {0}
        , rx_dropped {0}
        , rx_fifo {0}
        , rx_frame {0}
        , tx_packets {0}
        , tx_bytes {0}
        , tx_errors {0}
        , tx_dropped {0}
        , tx_fifo {0}
        , tx_carrier {0}
        , collisions {0}
    {
    }

    NetifInfoPrivate(const NetifInfoPrivate &other)
        : QSharedData(other)
        , index {other.index}
        , carrier {other.carrier}
        , oper_stat {other.oper_stat}
        , link_mode {other.link_mode}
        , mtu {other.mtu}
        , flags {other.flags}
        , arp_type {other.arp_type}
        , txqlen {other.txqlen}
        , carrier_changes {other.carrier_changes}
        , speed {other.speed}
        , ifname {other.ifname}
        , alias {other.alias}
        , conn_type {other.conn_type}
        , hw_addr {other.hw_addr}
        , hw_bcast {other.hw_bcast}
        , brand {other.brand}
        , recv_bps {other.recv_bps}
        , sent_bps {other.sent_bps}
        , addr4infolst {other.addr4infolst}
        , addr6infolst {other.addr6infolst}
        , iw_info {std::unique_ptr<iw_info_t>(new iw_info_t(*(other.iw_info)))}
        , rx_packets {other.rx_packets}
        , rx_bytes {other.rx_bytes}
        , rx_errors {other.rx_errors}
        , rx_dropped {other.rx_dropped}
        , rx_fifo {other.rx_fifo}
        , rx_frame {other.rx_frame}
        , tx_packets {other.tx_packets}
        , tx_bytes {other.tx_bytes}
        , tx_errors {other.tx_errors}
        , tx_dropped {other.tx_dropped}
        , tx_fifo {other.tx_fifo}
        , tx_carrier {other.tx_carrier}
        , collisions {other.collisions}
    {
    }
    ~NetifInfoPrivate() {}

private:
    // ====link params====

    int index;
    uint8_t carrier; // on:1/off:0
    uint8_t oper_stat; // operational status
    uint8_t link_mode;
    uint mtu;
    uint flags;
    uint arp_type; // connection type
    uint txqlen; // transmission queue length
    uint carrier_changes; // carrier state changes since boot
    uint speed;     // 原始speed值

    QByteArray ifname;
    QByteArray alias;
    QByteArray conn_type;
    QByteArray hw_addr; // link layer address
    QByteArray hw_bcast; // link layer broadcast address
    QString brand;

    qreal recv_bps = 0;             // 接收速度
    qreal sent_bps = 0;             // 发送速度

    QList<INet4Addr> addr4infolst;
    QList<INet6Addr> addr6infolst;

    // wireless link extension
    std::unique_ptr<iw_info_t> iw_info;
    bool isWireless = false;
    // ====stats====

    unsigned long long rx_packets; // total packets received
    unsigned long long rx_bytes; // total bytes received
    unsigned long long rx_errors; // bad packets received
    unsigned long long rx_dropped; // no space in linux buffers
    unsigned long long rx_fifo; // FIFO overruns
    unsigned long long rx_frame; // frame alignment error

    unsigned long long tx_packets; // total packets transmitted
    unsigned long long tx_bytes; // total bytes transmitted
    unsigned long long tx_errors; // packet transmit problems
    unsigned long long tx_dropped; // no space available in linux
    unsigned long long tx_fifo; // FIFO overruns
    unsigned long long tx_carrier; // loss of link pulse

    unsigned long long collisions; // network congestion

    friend class NetifInfo;
};

} // namespace system
} // namespace core

#endif // NETIF_P_H
