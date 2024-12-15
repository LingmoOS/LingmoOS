// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NETLINK_H
#define NETLINK_H

#include "nl_addr.h"
#include "nl_link.h"

#include <QtGlobal>
#include <QList>

#include <netlink/socket.h>
#include <netlink/cache.h>

#include <memory>

struct nl_cache;
struct nl_link;

namespace core {
namespace system {

class NLLink;
class NLAddr;

template<typename T, typename O>
class CacheIterator
{
public:
    bool hasNext()
    {
        return d->m_next != nullptr;
    }

    std::unique_ptr<T> next()
    {
        std::unique_ptr<T> tPtr;

        if (!d->m_next)
            return tPtr; // nullptr

        O *o = reinterpret_cast<O *>(d->m_next);
        tPtr.reset(new T(o));

        advance();

        return tPtr;
    }

public:
    ~CacheIterator()
    {
        delete d;
    }

private:
    struct context {
        struct nl_sock *m_sock;
        struct nl_cache *m_cache;
        struct nl_object *m_next;
    };
    CacheIterator(struct nl_sock *sock, struct nl_cache *cache)
        : d(new context {sock, cache, nullptr})
    {
        if (d->m_sock && d->m_cache) {
            nl_cache_refill(d->m_sock, d->m_cache);

            d->m_next = nl_cache_get_first(d->m_cache);
        }
    }

    void advance()
    {
        if (d->m_next)
            d->m_next = nl_cache_get_next(d->m_next);
    }

    struct context *d;

    friend class Netlink;
};
using LinkIterator = CacheIterator<NLLink, struct rtnl_link>;
using AddrIterator = CacheIterator<NLAddr, struct rtnl_addr>;

class Netlink
{
public:
    explicit Netlink();
    ~Netlink();

    LinkIterator linkIterator();
    AddrIterator addrIterator();

private:
    nl_sock *m_sock;
    nl_cache *m_linkCache;
    nl_cache *m_addrCache;
};

} // namespace system
} // namespace core

#endif // NETLINK_H
