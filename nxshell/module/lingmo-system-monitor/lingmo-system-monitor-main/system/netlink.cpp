// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "netlink.h"
#include "ddlog.h"
#include "nl_addr.h"
#include "nl_link.h"

#include <QDebug>

#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/addr.h>
#include <netlink/cache.h>
using namespace DDLog;
namespace core {
namespace system {

Netlink::Netlink()
{
    int rc = 0;
    m_sock = nl_socket_alloc();
    if (!m_sock) {
        qCWarning(app) << "Error: nl_socket_alloc failed";
        return;
    }

    rc = nl_connect(m_sock, NETLINK_ROUTE);
    if (rc) {
        qCWarning(app) << "Error: nl_connect failed";
        return;
    }

    rc = rtnl_link_alloc_cache(m_sock, AF_UNSPEC, &m_linkCache);
    if (rc) {
        qCWarning(app) << "Error: rtnl_link_alloc_cache failed";
        return;
    }

    rc = rtnl_addr_alloc_cache(m_sock, &m_addrCache);
    if (rc) {
        qCWarning(app) << "Error: rtnl_addr_alloc_cache failed";
    }
}

Netlink::~Netlink()
{
    nl_cache_free(m_linkCache);
    nl_cache_free(m_addrCache);
    nl_socket_free(m_sock);
}

LinkIterator Netlink::linkIterator()
{
    LinkIterator it(m_sock, m_linkCache);
    return it;
}

AddrIterator Netlink::addrIterator()
{
    AddrIterator it(m_sock, m_addrCache);
    return it;
}

}   // namespace system
}   // namespace core
