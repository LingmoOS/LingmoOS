// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dbus/daccountsinterface.h"
#include "dbus/dsystemaccountsinterface.h"

DACCOUNTS_BEGIN_NAMESPACE

class DAccountsManager;

class DAccountsManagerPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DAccountsManagerPrivate(DAccountsManager *parent = nullptr);
    ~DAccountsManagerPrivate() = default;

    quint64 getUIDFromObjectPath(const QString &path) const;

    DAccountsManager *q_ptr;
    DAccountsInterface *m_dAccountsInter;
    DSystemAccountsInterface *m_dSystemAccountsInter;
    Q_DECLARE_PUBLIC(DAccountsManager)
};

DACCOUNTS_END_NAMESPACE
