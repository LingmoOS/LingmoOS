// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nl_addr.h"

namespace core {
namespace system {

NLAddr::NLAddr(struct rtnl_addr *addr, bool hold)
    : m_addr(addr)
    , m_hold(hold)
{
    Q_ASSERT(m_addr != nullptr);
}

NLAddr::~NLAddr()
{
    if (m_hold)
        rtnl_addr_put(m_addr);
}

} // namespace system
} // namespace core
