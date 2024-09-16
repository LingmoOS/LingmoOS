// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACCESSPOINT_P_H
#define DACCESSPOINT_P_H

#include "dbus/daccesspointinterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DAccessPoint;

class DAccessPointPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DAccessPointPrivate(const quint64 apId, DAccessPoint *parent = nullptr);
    ~DAccessPointPrivate() override = default;

    DAccessPoint *q_ptr{nullptr};
    DAccessPointInterface *m_ap{nullptr};
    Q_DECLARE_PUBLIC(DAccessPoint)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
