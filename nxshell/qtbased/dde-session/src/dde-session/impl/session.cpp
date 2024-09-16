// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "session.h"
#include "org_freedesktop_login1_Manager.h"

#include <QCoreApplication>

Session::Session(QObject *parent)
    : QObject(parent)
    , m_sessionPid(0)
{

}

void Session::Logout()
{
    qApp->quit();
}

uint Session::GetSessionPid()
{
    return m_sessionPid;
}

void Session::setSessionPid(uint pid)
{
    m_sessionPid = pid;
}

QString Session::GetSessionPath()
{
    return m_selfSessionPath;
}

void Session::setSessionPath()
{
    if (m_sessionPid <= 0) {
        return;
    }
    org::freedesktop::login1::Manager login1ManagerInter(
        "org.freedesktop.login1",
        "/org/freedesktop/login1",
        QDBusConnection::systemBus(),
        this);

    QDBusPendingReply<QDBusObjectPath> pathResult = login1ManagerInter.GetSessionByPID(m_sessionPid);
    pathResult.waitForFinished();
    if (!pathResult.isError()) {
        m_selfSessionPath = pathResult.value().path();
    }
}
