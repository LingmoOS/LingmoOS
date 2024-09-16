// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NL_LINK_H
#define NL_LINK_H

#include <QByteArray>

#include <netlink/route/link.h>

namespace core {
namespace system {

class NLLink
{
public:
    explicit NLLink(struct rtnl_link *link, bool hold = false);
    ~NLLink();

    static QByteArray formatFlags(uint flags);
    static QByteArray formatCarrier(uint8_t carrier);
    static QByteArray formatOperState(uint8_t operStat);
    static QByteArray formatLinkMode(uint8_t linkMode);

    int ifindex() const;
    QByteArray ifname() const;
    QByteArray alias() const;
    uint mtu() const;
    uint flags() const;
    uint arp_type() const;
    uint txqlen() const;
    uint carrier_changes() const;
    uint8_t carrier() const;
    uint8_t oper_stat() const;
    uint8_t link_mode() const;
    QByteArray addr() const;
    QByteArray bcast() const;

    unsigned long long rx_packets() const;
    unsigned long long rx_bytes() const;
    unsigned long long rx_errors() const;
    unsigned long long rx_dropped() const;
    unsigned long long rx_fifo() const;
    unsigned long long rx_frame() const;

    unsigned long long tx_packets() const;
    unsigned long long tx_bytes() const;
    unsigned long long tx_errors() const;
    unsigned long long tx_dropped() const;
    unsigned long long tx_fifo() const;
    unsigned long long tx_carrier() const;

    unsigned long long collisions() const;

private:
    struct rtnl_link *m_link;
    bool m_hold;
};

inline QByteArray NLLink::formatFlags(uint flags)
{
    QByteArray buffer {};
    const size_t bufsiz = 256;
    QByteArray tmpbuf(bufsiz, '\0');
    rtnl_link_flags2str(int(flags), tmpbuf.data(), bufsiz);
    buffer.append(QByteArray::number(flags)).append('<').append(tmpbuf.toUpper()).append('>');
    return buffer;
}

inline QByteArray NLLink::formatCarrier(uint8_t carrier)
{
    const size_t bufsiz = 16;
    QByteArray buf(bufsiz, '\0');
    rtnl_link_carrier2str(carrier, buf.data(), bufsiz);
    return buf;
}

inline QByteArray NLLink::formatOperState(uint8_t operStat)
{
    const size_t bufsiz = 256;
    QByteArray buf(bufsiz, '\0');
    rtnl_link_operstate2str(operStat, buf.data(), bufsiz);
    return buf;
}

inline QByteArray NLLink::formatLinkMode(uint8_t linkMode)
{
    const size_t bufsiz = 16;
    QByteArray buf(bufsiz, '\0');
    rtnl_link_mode2str(linkMode, buf.data(), bufsiz);
    return buf;
}

inline int NLLink::ifindex() const
{
    return rtnl_link_get_ifindex(m_link);
}

inline QByteArray NLLink::ifname() const
{
    QByteArray buffer {};
    buffer.append(rtnl_link_get_name(m_link));
    return buffer;
}

inline QByteArray NLLink::alias() const
{
    QByteArray buffer {};
    buffer.append(rtnl_link_get_ifalias(m_link));
    return buffer;
}

inline uint NLLink::mtu() const
{
    return rtnl_link_get_mtu(m_link);
}

inline uint NLLink::flags() const
{
    return rtnl_link_get_flags(m_link);
}

inline uint NLLink::arp_type() const
{
    return rtnl_link_get_arptype(m_link);
}

inline uint NLLink::txqlen() const
{
    return rtnl_link_get_txqlen(m_link);
}

inline uint NLLink::carrier_changes() const
{
    uint cnt = 0;
    // cnt will be modified only if success
    rtnl_link_get_carrier_changes(m_link, &cnt);
    return cnt;
}

inline uint8_t NLLink::carrier() const
{
    return rtnl_link_get_carrier(m_link);
}

inline uint8_t NLLink::oper_stat() const
{
    return rtnl_link_get_operstate(m_link);
}

inline uint8_t NLLink::link_mode() const
{
    return rtnl_link_get_linkmode(m_link);
}

inline QByteArray NLLink::addr() const
{
    QByteArray buffer {};
    auto *hwaddr = rtnl_link_get_addr(m_link);
    if (hwaddr) {
        const size_t bufsiz = 128;
        char tmpbuf[bufsiz];
        nl_addr2str(hwaddr, tmpbuf, bufsiz);
        tmpbuf[bufsiz - 1] = '\0';
        buffer.append(tmpbuf);
    }
    return buffer;
}

inline QByteArray NLLink::bcast() const
{
    QByteArray buffer {};
    auto *bcast = rtnl_link_get_broadcast(m_link);
    if (bcast) {
        const size_t bufsiz = 128;
        char tmpbuf[bufsiz];
        nl_addr2str(bcast, tmpbuf, bufsiz);
        tmpbuf[bufsiz - 1] = '\0';
        buffer.append(tmpbuf);
    }
    return buffer;
}
// https://www.infradead.org/~tgr/libnl/doc/api/group__link.html#gabe368ecaa2f41e8392325b8060862ba6
inline unsigned long long NLLink::rx_packets() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_RX_PACKETS);
}

inline unsigned long long NLLink::rx_bytes() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_RX_BYTES);
}

inline unsigned long long NLLink::rx_errors() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_RX_ERRORS);
}

inline unsigned long long NLLink::rx_dropped() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_RX_DROPPED);
}

inline unsigned long long NLLink::rx_fifo() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_RX_FIFO_ERR);
}

inline unsigned long long NLLink::rx_frame() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_RX_FRAME_ERR);
}

inline unsigned long long NLLink::tx_packets() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_TX_PACKETS);
}

inline unsigned long long NLLink::tx_bytes() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_TX_BYTES);
}

inline unsigned long long NLLink::tx_errors() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_TX_ERRORS);
}

inline unsigned long long NLLink::tx_dropped() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_TX_DROPPED);
}

inline unsigned long long NLLink::tx_fifo() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_TX_FIFO_ERR);
}

inline unsigned long long NLLink::tx_carrier() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_TX_CARRIER_ERR);
}

inline unsigned long long NLLink::collisions() const
{
    return rtnl_link_get_stat(m_link, RTNL_LINK_COLLISIONS);
}

} // namespace system
} // namespace core

#endif // NL_LINK_H
