// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DACTIVECONNECTION_P_H
#define DACTIVECONNECTION_P_H

#include "dactiveconnection.h"
#include "dbus/dactiveconnectioninterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DActiveConnectionPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DActiveConnectionPrivate(const quint64 activeConnId, DActiveConnection *parent = nullptr);
    ~DActiveConnectionPrivate() override;

    DActiveConnection *q_ptr{nullptr};
    DActiveConnectionInterface *m_activeConn{nullptr};
    Q_DECLARE_PUBLIC(DActiveConnection)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
