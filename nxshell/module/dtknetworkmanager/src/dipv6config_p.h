// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV6CONFIG_P_H
#define DIPV6CONFIG_P_H

#include "dbus/dipv6configinterface.h"
#include "dipv6config.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIPv6ConfigPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DIPv6ConfigPrivate(const quint64 id, DIPv6Config *parent = nullptr);
    ~DIPv6ConfigPrivate() override = default;

    DIPv6Config *q_ptr{nullptr};
    DIPv6ConfigInterface *m_ipv6{nullptr};
    Q_DECLARE_PUBLIC(DIPv6Config)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
