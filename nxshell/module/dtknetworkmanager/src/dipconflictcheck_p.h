// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DIPCONFLICTCHECK_P_H
#define DIPCONFLICTCHECK_P_H

#include "dbus/dipconflictcheckinterface.h"

DNETWORKMANAGER_BEGIN_NAMESPACE

class DIPConflictCheck;

class DIPConflictCheckPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DIPConflictCheckPrivate(DIPConflictCheck *parent = nullptr);
    ~DIPConflictCheckPrivate() override = default;

    DIPConflictCheck *q_ptr{nullptr};
    DIPConflictCheckInterface *m_check{nullptr};
    Q_DECLARE_PUBLIC(DIPConflictCheck)
};

DNETWORKMANAGER_END_NAMESPACE

#endif
