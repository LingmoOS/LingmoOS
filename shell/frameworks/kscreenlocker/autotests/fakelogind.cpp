/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "fakelogind.h"

FakeLogind::FakeLogind(QObject *parent)
    : QObject(parent)
    , m_session(new FakeLogindSession(QStringLiteral("/org/freedesktop/login1/session/auto"), this))
{
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/freedesktop/login1"), this, QDBusConnection::ExportScriptableContents);
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.freedesktop.login1"));
}

FakeLogind::~FakeLogind()
{
    QDBusConnection::sessionBus().unregisterObject(QStringLiteral("/org/freedesktop/login1"));
    QDBusConnection::sessionBus().unregisterService(QStringLiteral("org.freedesktop.login1"));
}

QDBusObjectPath FakeLogind::GetSession(const QString &session)
{
    Q_UNUSED(session)
    return QDBusObjectPath(m_session->path());
}

void FakeLogind::lock()
{
    m_session->lock();
}

void FakeLogind::unlock()
{
    m_session->unlock();
}

FakeLogindSession::FakeLogindSession(const QString &path, QObject *parent)
    : QObject(parent)
    , m_path(path)
{
    QDBusConnection::sessionBus().registerObject(m_path, this, QDBusConnection::ExportScriptableContents);
}

FakeLogindSession::~FakeLogindSession()
{
    QDBusConnection::sessionBus().unregisterObject(m_path);
}

void FakeLogindSession::lock()
{
    Q_EMIT Lock();
}

void FakeLogindSession::unlock()
{
    Q_EMIT Unlock();
}

#include "moc_fakelogind.cpp"
