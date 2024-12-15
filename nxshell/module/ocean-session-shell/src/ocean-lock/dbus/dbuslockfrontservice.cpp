// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbuslockfrontservice.h"

/*
 * Implementation of interface class DBusLockFront
 */

DBusLockFrontService::DBusLockFrontService(DBusLockAgent *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

DBusLockFrontService::~DBusLockFrontService()
{
}

bool DBusLockFrontService::Visible()
{
    return parent()->Visible();
}

void DBusLockFrontService::Show()
{
    parent()->Show();
}

void DBusLockFrontService::ShowUserList()
{
    parent()->ShowUserList();
}

void DBusLockFrontService::ShowAuth(bool active)
{
    parent()->ShowAuth(active);
}

void DBusLockFrontService::Suspend(bool enable)
{
    parent()->Suspend(enable);
}

void DBusLockFrontService::Hibernate(bool enable)
{
    parent()->Hibernate(enable);
}
