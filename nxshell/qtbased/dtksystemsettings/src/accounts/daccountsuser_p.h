// SPDX-FileCopyrightText: 2022 Uniontech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dbus/dsystemuserinterface.h"
#include "dbus/duserinterface.h"

#include <QDir>

DACCOUNTS_BEGIN_NAMESPACE

class DAccountsUser;

class DAccountsUserPrivate : public QObject
{
    Q_OBJECT
public:
    explicit DAccountsUserPrivate(const quint64 uid, DAccountsUser *parent = nullptr);
    ~DAccountsUserPrivate() = default;

    QList<QByteArray> getImageFromDir(const QDir &dir) const;

    DAccountsUser *q_ptr;
    DUserInterface *m_dUserInter;
    DSystemUserInterface *m_dSystemUserInter;
    Q_DECLARE_PUBLIC(DAccountsUser)
};

DACCOUNTS_END_NAMESPACE
