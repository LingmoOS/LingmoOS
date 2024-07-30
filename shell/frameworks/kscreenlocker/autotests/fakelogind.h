/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QDBusConnection>
#include <QDBusObjectPath>
#include <QObject>

class FakeLogindSession;

class FakeLogind : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Manager")
public:
    explicit FakeLogind(QObject *parent = nullptr);
    ~FakeLogind() override;

    FakeLogindSession *session() const
    {
        return m_session;
    }

public Q_SLOTS:
    Q_SCRIPTABLE QDBusObjectPath GetSession(const QString &session);
    Q_SCRIPTABLE void lock();
    Q_SCRIPTABLE void unlock();

private:
    FakeLogindSession *m_session;
};

class FakeLogindSession : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.login1.Session")
public:
    explicit FakeLogindSession(const QString &path, QObject *parent = nullptr);
    ~FakeLogindSession() override;

    const QString &path()
    {
        return m_path;
    }

    void lock();
    void unlock();

Q_SIGNALS:
    Q_SCRIPTABLE void Lock();
    Q_SCRIPTABLE void Unlock();

private:
    QString m_path;
};
