// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "nl_link.h"

namespace core {
namespace system {

NLLink::NLLink(struct rtnl_link *link, bool hold)
    : m_link(link)
    , m_hold(hold)
{
    Q_ASSERT(m_link != nullptr);
}

NLLink::~NLLink()
{
    if (m_hold && m_link)
        rtnl_link_put(m_link);
}

} // namespace system
} // namespace core
