/*
SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>
#include <QDBusUnixFileDescriptor>
#include <QObject>

class QDBusServiceWatcher;

class LogindIntegration : public QObject
{
    Q_OBJECT
public:
    explicit LogindIntegration(QObject *parent = nullptr);
    ~LogindIntegration() override;

    bool isConnected() const
    {
        return m_connected;
    }

    void inhibit();
    void uninhibit();

    bool isInhibited() const;

    /**
     * Notify logind of our current state
     */
    void setLocked(bool locked);
    bool isLocked() const;

Q_SIGNALS:
    /**
     * signalled when logind asks us to lock
     */
    void requestLock();
    void requestUnlock();
    void connectedChanged();
    void prepareForSleep(bool);
    void inhibited();

private:
    friend class LogindTest;
    /**
     * The DBusConnection argument is needed for the unit test. Logind uses the system bus
     * on which the unit test's fake logind cannot register to. Thus the unit test need to
     * be able to do everything over the session bus. This ctor allows the LogindTest to
     * create a LogindIntegration which listens on the session bus.
     **/
    explicit LogindIntegration(const QDBusConnection &connection, QObject *parent = nullptr);
    void logindServiceRegistered();
    void consolekitServiceRegistered();
    void commonServiceRegistered(QDBusPendingCallWatcher *watcher);
    QDBusConnection m_bus;
    QDBusServiceWatcher *m_logindServiceWatcher;
    bool m_connected;
    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
    const QString *m_service;
    const QString *m_path;
    QString m_sessionPath;
    const QString *m_managerInterface;
    const QString *m_sessionInterface;
};
