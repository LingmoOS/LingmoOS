/*
SPDX-FileCopyrightText: 1999 Martin R. Jones <mjones@kde.org>
SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QDBusContext>
#include <QDBusMessage>
#include <QObject>

class QDBusServiceWatcher;

namespace ScreenLocker
{
class InhibitRequest
{
public:
    QString dbusid;
    uint cookie;
    uint powerdevilcookie;
};

class KSldApp;
class Interface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.freedesktop.ScreenSaver")
public:
    explicit Interface(KSldApp *parent = nullptr);
    ~Interface() override;

public Q_SLOTS:
    /**
     * Lock the screen.
     */
    void Lock();

    /**
     * Like Lock() but immediately show the switch user dialog
     */
    void SwitchUser();

    /**
     * Simulate user activity
     */
    void SimulateUserActivity();
    /**
     * Request a change in the state of the screensaver.
     * Set to TRUE to request that the screensaver activate.
     * Active means that the screensaver has blanked the
     * screen and may run a graphical theme.  This does
     * not necessary mean that the screen is locked.
     */
    bool SetActive(bool state);

    /// Returns the value of the current state of activity (See setActive)
    bool GetActive();

    /**
     * Returns the number of seconds that the screensaver has
     * been active.  Returns zero if the screensaver is not active.
     */
    uint GetActiveTime();

    /**
     * Returns the number of seconds that the session has
     * been idle.  Returns zero if the session is not idle.
     */
    uint GetSessionIdleTime();

    /**
     * Request that saving the screen due to system idleness
     * be blocked until UnInhibit is called or the
     * calling process exits.
     * The cookie is a random number used to identify the request
     */
    uint Inhibit(const QString &application_name, const QString &reason_for_inhibit);
    /// Cancel a previous call to Inhibit() identified by the cookie.
    void UnInhibit(uint cookie);

    /**
     * Request that running themes while the screensaver is active
     * be blocked until UnThrottle is called or the
     * calling process exits.
     * The cookie is a random number used to identify the request
     */
    uint Throttle(const QString &application_name, const QString &reason_for_inhibit);
    /// Cancel a previous call to Throttle() identified by the cookie.
    void UnThrottle(uint cookie);

    // org.kde.screensvar
    void configure();

Q_SIGNALS:
    // DBus signals
    void ActiveChanged(bool state);

    void AboutToLock();

private Q_SLOTS:
    void slotLocked();
    void slotUnlocked();
    void serviceUnregistered(const QString &name);

private:
    void sendLockReplies();

    KSldApp *m_daemon;
    QDBusServiceWatcher *m_serviceWatcher;
    QList<InhibitRequest> m_requests;
    uint m_next_cookie;
    QList<QDBusMessage> m_lockReplies;
};
}
