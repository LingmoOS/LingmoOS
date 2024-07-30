/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>
*/

#pragma once

#include <chrono>

#include <QDBusServiceWatcher>
#include <QTimer>

using namespace std::chrono_literals;

class DBusServiceWatcher : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual void start();
    QList<QString> serviceNames() const;

Q_SIGNALS:
    void serviceUnregistered();
    void serviceRegistered();

private:
    // org.kde.StatusNotifierWatcher (kded5): SNI won't be registered
    // org.freedesktop.Notifications (lingmoshell): SNI won't be visualized
    const QList<QString> m_serviceNames{QStringLiteral("org.kde.StatusNotifierWatcher"), QStringLiteral("org.freedesktop.Notifications")};
    QDBusServiceWatcher *m_watcher = nullptr;
};

class IdleWatcher : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    virtual void start();

Q_SIGNALS:
    void idle();
    void notIdle();
};

// Somewhat horrible container to track automatic activation and close conditions.
// This tracks the availability of dbus interfaces that are required for visualization
// of the an SNI, if they are missing then drkonqi may auto activate the window.
// This also tracks system idleness as we want to prevent auto closing of the SNI
// when the user isn't at the system.
// If neither is applicable the SNI gets auto closed after a while.
class ActivationCloseTimer : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    void start(DBusServiceWatcher *dbusWatcher, IdleWatcher *idleWatcher);

    void setActivationTimeout(std::chrono::milliseconds timeout);
    void setCloseTimeout(std::chrono::milliseconds timeout);

Q_SIGNALS:
    void autoClose();
    void autoActivate();

private Q_SLOTS:
    void watchDBus(DBusServiceWatcher *watcher);
    void watchIdle(IdleWatcher *idleWatcher);

    void refresh();

private:
    int m_pendingActivations = 0;
    QTimer m_closeTimer;
    bool m_idle = false;
    std::chrono::milliseconds m_activationTimeout = 10s;
    std::chrono::milliseconds m_closeTimeout = 1min;
};
