// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "inhibitor.h"

Inhibitor::Inhibitor(const QString &appId, uint topLevelXid, const QString &reason, uint flags, uint index)
    : QObject (nullptr)
    , m_appId(appId)
    , m_reason(reason)
    , m_index(index)
    , m_topLevelXid(topLevelXid)
    , m_flags(flags)
{

}
