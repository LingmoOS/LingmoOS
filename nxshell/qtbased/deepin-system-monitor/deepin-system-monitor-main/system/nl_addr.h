// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NL_ADDR_H
#define NL_ADDR_H

#include <QByteArray>
#include <QList>
#include <QDebug>

#include <netlink/route/addr.h>
#include <netlink/addr.h>

namespace core {
namespace system {

/**
 * @brief Wrapper class around the rtnl_addr api
 */
#define LMOVE(m,n) ((m)<<(n))
#define RMOVE(m,n) ((m)>>(n))

class NLAddr
{
public:
    explicit NLAddr(struct rtnl_addr *addr, bool hold = false);
    ~NLAddr();

    int ifindex() const;
    int family() const;
    int prefixlen() const;
    int scope() const;

    void local();
    QByteArray peer() const;
    QByteArray broadcast() const;
    QByteArray multicast() const;
    QByteArray anycast() const;
    QByteArray netaddr() const;
    QByteArray netmask() const;

private:
    struct rtnl_addr *m_addr;
    bool m_hold;

    QByteArray netAddr;
    QByteArray netMask;
};

inline int NLAddr::ifindex() const
{
    return rtnl_addr_get_ifindex(m_addr);
}

inline int NLAddr::family() const
{
    return rtnl_addr_get_family(m_addr);
}

inline int NLAddr::prefixlen() const
{
    return rtnl_addr_get_prefixlen(m_addr);
}

inline int NLAddr::scope() const
{
    return rtnl_addr_get_scope(m_addr);
}

inline void NLAddr::local()
{
    QByteArray buffer {};
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];

    auto *addrPtr = rtnl_addr_get_local(m_addr);
    nl_addr2str(addrPtr, tmpbuf, bufsiz);
    tmpbuf[bufsiz - 1] = '\0';
    buffer.append(tmpbuf);

    netAddr = buffer;
    const QList<QByteArray> &arrayBuffer = buffer.split('/');
    if (arrayBuffer.size() == 2) {
        int n = arrayBuffer.last().toInt();
        char mask[32];
        unsigned int addr = LMOVE(0xffffffff, 32 - n);
        sprintf(mask, "%u.%u.%u.%u", RMOVE(addr, 24), RMOVE(addr, 16) & 0xff, RMOVE(addr, 8) & 0xff, addr & 0xff);
        netAddr = arrayBuffer.first();
        netMask = QByteArray(mask);
    }
}

inline QByteArray NLAddr::netaddr() const
{
    return netAddr;
}

inline QByteArray NLAddr::netmask() const
{
    return netMask;
}

inline QByteArray NLAddr::peer() const
{
    QByteArray buffer {};
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];

    auto *addr = rtnl_addr_get_peer(m_addr);
    nl_addr2str(addr, tmpbuf, bufsiz);
    tmpbuf[bufsiz - 1] = '\0';
    buffer.append(tmpbuf);
    return buffer;
}

inline QByteArray NLAddr::broadcast() const
{
    QByteArray buffer {};
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];

    auto *addr = rtnl_addr_get_broadcast(m_addr);
    nl_addr2str(addr, tmpbuf, bufsiz);
    tmpbuf[bufsiz - 1] = '\0';
    buffer.append(tmpbuf);
    return buffer;
}

inline QByteArray NLAddr::multicast() const
{
    QByteArray buffer {};
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];

    auto *addr = rtnl_addr_get_multicast(m_addr);
    nl_addr2str(addr, tmpbuf, bufsiz);
    tmpbuf[bufsiz - 1] = '\0';
    buffer.append(tmpbuf);
    return buffer;
}

inline QByteArray NLAddr::anycast() const
{
    QByteArray buffer {};
    const size_t bufsiz = 128;
    char tmpbuf[bufsiz];

    auto *addr = rtnl_addr_get_anycast(m_addr);
    nl_addr2str(addr, tmpbuf, bufsiz);
    tmpbuf[bufsiz - 1] = '\0';
    buffer.append(tmpbuf);
    return buffer;
}

} // namespace system
} // namespace core

#endif // NL_ADDR_H
