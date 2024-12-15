// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "daccountsmanager.h"
#include "daccountstypes.h"
#include "daccountsuser.h"

#include <QObject>

DACCOUNTS_USE_NAMESPACE

class AccountsDemo : public QObject
{
    Q_OBJECT
public:
    AccountsDemo();
    void run();

private:
    DAccountsManager manager;
    QSharedPointer<DAccountsUser> user{ nullptr };
};
