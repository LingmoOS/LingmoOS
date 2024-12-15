// SPDX-FileCopyrightText: 2011 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dbusshutdownfrontservice.h"

DBusShutdownFrontService::DBusShutdownFrontService(DBusShutdownAgent *parent)
    : QDBusAbstractAdaptor(parent)
{
    setAutoRelaySignals(true);
}

DBusShutdownFrontService::~DBusShutdownFrontService()
{

}

void DBusShutdownFrontService::Show()
{
    parent()->show();
}

void DBusShutdownFrontService::Shutdown()
{
    parent()->Shutdown();
}

void DBusShutdownFrontService::Restart()
{
    parent()->Restart();
}

void DBusShutdownFrontService::Logout()
{
    parent()->Logout();
}

void DBusShutdownFrontService::Suspend()
{
    parent()->Suspend();
}

void DBusShutdownFrontService::Hibernate()
{
    parent()->Hibernate();
}

void DBusShutdownFrontService::SwitchUser()
{
    parent()->SwitchUser();
}

void DBusShutdownFrontService::Lock()
{
    parent()->Lock();
}
