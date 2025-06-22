/*
    SPDX-FileCopyrightText: 1999 Martin R. Jones <mjones@kde.org>
    SPDX-FileCopyrightText: 2003 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2008 Chani Armitage <chanika@gmail.com>
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>

SPDX-License-Identifier: GPL-2.0-or-later
*/
#pragma once

#include <QElapsedTimer>
#include <QProcessEnvironment>

#include <KScreenLocker/kscreenlocker_export.h>

// forward declarations
class GlobalAccel;
class LogindIntegration;
class QTimer;
class KSldTest;
class PowerManagementInhibition;

namespace ScreenLocker
{
enum class EstablishLock {
    Immediate, /// Require password from the start. Use if invoked explicitly by the user
    Delayed, /// Allow the user to log back in without a password for a configured grace time.
    DefaultToSwitchUser, /// UI should default to showing the "switch user dialog"
};

class AbstractLocker;
class WaylandServer;

class KSCREENLOCKER_EXPORT KSldApp : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ksld.App")

public:
    enum LockState {
        Unlocked,
        AcquiringLock,
        Locked,
    };
    Q_ENUM(LockState)

    static KSldApp *self();

    explicit KSldApp(QObject *parent = nullptr);
    ~KSldApp() override;

    LockState lockState() const
    {
        return m_lockState;
    }

    /**
     * @returns the number of milliseconds passed since the screen has been locked.
     **/
    uint activeTime() const;

    void configure();

    void userActivity();
    bool isGraceTime() const;

    void setWaylandFd(int fd);

    void setGreeterEnvironment(const QProcessEnvironment &env);

    /**
     * Can be used by the lock window to remove the lock during grace time.
     **/
    void unlock();
    void inhibit();
    void uninhibit();

    void lock(EstablishLock establishLock, int attemptCount = 0);
    void initialize();

    bool event(QEvent *event) override;

    /**
     * For testing
     * @internal
     **/
    int idleId() const
    {
        return m_idleId;
    }
    /**
     * For testing
     * @internal
     **/
    void setIdleId(int idleId)
    {
        m_idleId = idleId;
    }
    /**
     * For testing
     * @internal
     **/
    void setGraceTime(int msec)
    {
        m_lockGrace = msec;
    }

    bool forceSoftwareRendering() const
    {
        return m_forceSoftwareRendering;
    }

    void setForceSoftwareRendering(bool force)
    {
        m_forceSoftwareRendering = force;
    }

Q_SIGNALS:
    void aboutToLock();
    void locked();
    void unlocked();
    void lockStateChanged();

private Q_SLOTS:
    void cleanUp();
    void endGraceTime();
    void solidSuspend();

public Q_SLOTS:
    void lockScreenShown();

private:
    void initializeX11();
    bool establishGrab();
    void startLockProcess(EstablishLock establishLock);
    void showLockWindow();
    void hideLockWindow();
    void doUnlock();
    bool isFdoPowerInhibited() const;

    LockState m_lockState;
    QProcess *m_lockProcess;
    AbstractLocker *m_lockWindow;
    WaylandServer *m_waylandServer;

    /**
     * Timer to measure how long the screen is locked.
     * This information is required by DBus Interface.
     **/
    QElapsedTimer m_lockedTimer;
    int m_idleId;
    /**
     * Number of milliseconds after locking in which user activity will result in screen being
     * unlocked without requiring a password.
     **/
    int m_lockGrace;
    /**
     * Controls whether user activity may remove the lock. Only enabled after idle timeout.
     **/
    bool m_inGraceTime;
    /**
     * Grace time ends when timer expires.
     **/
    QTimer *m_graceTimer;
    int m_inhibitCounter;
    LogindIntegration *m_logind;
    GlobalAccel *m_globalAccel = nullptr;
    bool m_hasXInput2 = false;
    bool m_forceSoftwareRendering = false;

    bool m_isX11;
    bool m_isWayland;
    int m_greeterCrashedCounter = 0;
    QProcessEnvironment m_greeterEnv;
    PowerManagementInhibition *m_powerManagementInhibition;

    int m_waylandFd = -1;

    // for auto tests
    friend KSldTest;
};
} // namespace
