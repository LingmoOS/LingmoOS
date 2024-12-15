// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETIF_H
#define NETIF_H

#include "private/netif_p.h"

#include <QByteArray>
#include <QSharedDataPointer>
#include <QList>

#include <netlink/route/link.h>

#include <memory>

namespace core {
namespace system {

struct inet_addr_t {
    int family;
    QByteArray addr;
};
struct inet_addr4_t : public inet_addr_t {
    QByteArray mask;
    QByteArray bcast;
};
struct inet_addr6_t : public inet_addr_t {
    int prefixlen;
    int scope;
};

#ifdef __sw_64__
struct ethtool_cmd {
        uint32_t   cmd;
        uint32_t   supported;
        uint32_t   advertising;
        uint16_t   speed;
        uint8_t    duplex;
        uint8_t    port;
        uint8_t    phy_address;
        uint8_t    transceiver;
        uint8_t    autoneg;
        uint8_t    mdio_support;
        uint32_t   maxtxpkt;
        uint32_t   maxrxpkt;
        uint16_t   speed_hi;
        uint8_t    eth_tp_mdix;
        uint8_t    eth_tp_mdix_ctrl;
        uint32_t   lp_advertising;
        uint16_t reserved[4];
};
#else
struct ethtool_cmd {
    uint32_t cmd;
    uint32_t supported;                                  /* Features this interface supports */
    uint32_t advertising;                                /* Features this interface advertises */
    uint16_t speed;                                      /* The forced speed, 10Mb, 100Mb, gigabit */
    uint8_t duplex;                                      /* Duplex, half or full */
    uint8_t port;                                        /* Which connector port */
    uint8_t phy_address;
    uint8_t transceiver;                                 /* Which tranceiver to use */
    uint8_t autoneg;                                     /* Enable or disable autonegotiation */
    uint16_t maxtxpkt;                                   /* Tx pkts before generating tx int */
    uint16_t maxrxpkt;                                   /* Rx pkts before generating rx int */
    uint16_t reserved[4];
};
#endif

/* The forced speed, 10Mb, 100Mb, gigabit, 10GbE. */
#define SPEED_10                10
#define SPEED_100               100
#define SPEED_1000              1000
#define SPEED_10000             10000

using INetAddr = std::shared_ptr<struct inet_addr_t>;
using INet4Addr = std::shared_ptr<struct inet_addr4_t>;
using INet6Addr = std::shared_ptr<struct inet_addr6_t>;

class NetifInfoData;
class NetifInfoDB;
class NLLink;
class NLAddr;
class wireless;

class NetifInfo
{
public:
    NetifInfo();
    NetifInfo(const NetifInfo &other);
    NetifInfo &operator=(const NetifInfo &rhs);
    ~NetifInfo();

    // link params
    int index() const;
    QByteArray ifname() const;
    QByteArray alias() const;
    uint mtu() const;
    uint flags() const;
    uint arpType() const;
    uint txqlen() const;
    quint8 operState() const;
    quint8 linkMode() const;
    quint8 carrier() const;
    uint carrierChanges() const;
    uint speed() const;
    QByteArray connectionType() const;
    QByteArray linkAddress() const;
    QByteArray linkBroadcast() const;
    QString brand() const;

    // wireless link only
    QByteArray essid() const;
    uint8_t linkQuality() const;
    uint8_t signalLevel() const;
    uint8_t noiseLevel() const;
    bool isWireless() const;

    // link stats
    qulonglong rxPackets() const;
    qulonglong rxBytes() const;
    qulonglong rxErrors() const;
    qulonglong rxDropped() const;
    qulonglong rxFIFO() const;
    qulonglong rxFrame() const;

    qulonglong txPackets() const;
    qulonglong txBytes() const;
    qulonglong txErrors() const;
    qulonglong txDropped() const;
    qulonglong txFIFO() const;
    qulonglong txCarrier() const;

    qulonglong collisions() const;

    qreal recv_bps() const;
    qreal sent_bps() const;

    void set_recv_bps(qreal recv_bps);
    void set_sent_bps(qreal sent_bps);

    QList<INet4Addr> addr4InfoList() const;
    QList<INet6Addr> addr6InfoList() const;



protected:
    void updateAddr4Info(const QList<INet4Addr> &addrList);
    void updateAddr6Info(const QList<INet6Addr> &addrList);
    void updateHWAddr(const QByteArray ifname);
    void updateLinkInfo(const NLLink *link);
    void updateWirelessInfo(); // ioctl
    void updateBrandInfo(); // udev

private:
    QSharedDataPointer<NetifInfoPrivate> d;

    friend class NetifInfoDB;
};

inline int NetifInfo::index() const
{
    return d->index;
}

inline QByteArray NetifInfo::ifname() const
{
    return d->ifname;
}

inline QByteArray NetifInfo::alias() const
{
    return d->alias;
}

inline uint NetifInfo::mtu() const
{
    return d->mtu;
}

inline uint NetifInfo::flags() const
{
    return d->flags;
}

inline uint NetifInfo::arpType() const
{
    return d->arp_type;
}

inline uint NetifInfo::txqlen() const
{
    return d->txqlen;
}

inline quint8 NetifInfo::operState() const
{
    return d->oper_stat;
}

inline quint8 NetifInfo::linkMode() const
{
    return d->link_mode;
}

inline quint8 NetifInfo::carrier() const
{
    return d->carrier;
}

inline uint NetifInfo::carrierChanges() const
{
    return d->carrier_changes;
}

inline uint NetifInfo::speed() const
{
    return d->speed;
}

inline QByteArray NetifInfo::connectionType() const
{
    return d->conn_type;
}

inline QByteArray NetifInfo::linkAddress() const
{
    return d->hw_addr;
}

inline QByteArray NetifInfo::linkBroadcast() const
{
    return d->hw_bcast;
}

inline QString NetifInfo::brand() const
{
    return d->brand;
}

inline QByteArray NetifInfo::essid() const
{
    if (d->iw_info)
        return d->iw_info->essid;
    else
        return {};
}

inline uint8_t NetifInfo::linkQuality() const
{
    if (d->iw_info)
        return d->iw_info->qual.qual;
    else
        return 0;
}

inline uint8_t NetifInfo::signalLevel() const
{
    if (d->iw_info)
        return d->iw_info->qual.level;
    else
        return 0;
}

inline bool NetifInfo::isWireless() const
{
    return d->isWireless;
}

inline uint8_t NetifInfo::noiseLevel() const
{
    if (d->iw_info)
        return d->iw_info->qual.noise;
    else
        return 0;
}

inline qulonglong NetifInfo::rxPackets() const
{
    return d->rx_packets;
}

inline qulonglong NetifInfo::rxBytes() const
{
    return d->rx_bytes;
}

inline qulonglong NetifInfo::rxErrors() const
{
    return d->rx_errors;
}

inline qulonglong NetifInfo::rxDropped() const
{
    return d->rx_dropped;
}

inline qulonglong NetifInfo::rxFIFO() const
{
    return d->rx_fifo;
}

inline qulonglong NetifInfo::rxFrame() const
{
    return d->rx_frame;
}

inline qulonglong NetifInfo::txPackets() const
{
    return d->tx_packets;
}

inline qulonglong NetifInfo::txBytes() const
{
    return d->tx_bytes;
}

inline qulonglong NetifInfo::txErrors() const
{
    return d->tx_errors;
}

inline qulonglong NetifInfo::txDropped() const
{
    return d->tx_dropped;
}

inline qulonglong NetifInfo::txFIFO() const
{
    return d->tx_fifo;
}

inline qulonglong NetifInfo::txCarrier() const
{
    return d->tx_carrier;
}

inline qulonglong NetifInfo::collisions() const
{
    return d->collisions;
}

inline QList<INet4Addr> NetifInfo::addr4InfoList() const
{
    return d->addr4infolst;
}

inline QList<INet6Addr> NetifInfo::addr6InfoList() const
{
    return d->addr6infolst;
}

inline void NetifInfo::updateAddr4Info(const QList<INet4Addr> &addrList)
{
    d->addr4infolst = addrList;
}

inline void NetifInfo::updateAddr6Info(const QList<INet6Addr> &addrList)
{
    d->addr6infolst = addrList;
}



inline qreal NetifInfo::recv_bps() const
{
    return d->recv_bps;
}

inline qreal NetifInfo::sent_bps() const
{
    return d->sent_bps;
}

} // namespace system
} // namespace core

Q_DECLARE_TYPEINFO(core::system::NetifInfo, Q_MOVABLE_TYPE);
using NetifInfoPtr = std::shared_ptr<class core::system::NetifInfo>;

#endif // NETIF_H
