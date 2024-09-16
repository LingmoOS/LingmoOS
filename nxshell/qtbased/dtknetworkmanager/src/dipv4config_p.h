// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPV4CONFIG_P_H
#define DIPV4CONFIG_P_H

#include "dbus/dipv4configinterface.h"
#include "dipv4config.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIPv4ConfigPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DIPv4ConfigPrivate(const quint64 id, DIPv4Config *parent = nullptr);
    ~DIPv4ConfigPrivate() override = default;

    DIPv4Config *q_ptr{nullptr};
    DIPv4ConfigInterface *m_ipv4{nullptr};
    Q_DECLARE_PUBLIC(DIPv4Config)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
