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

/**
 * Enum for the different ways to establish a lock.
 */
enum class EstablishLock {
    /**
     * Require password from the start. Use if invoked explicitly by the user.
     */
    Immediate,

    /**
     * Allow the user to log back in without a password for a configured grace time.
     */
    Delayed,

    /**
     * UI should default to showing the "switch user dialog".
     */
    DefaultToSwitchUser,
};

/**
 * @returns a string representation of the given EstablishLock value.
 **/
QString establishLockToString(EstablishLock establishLock);

class AbstractLocker;
class WaylandServer;

/**
 * @class KSldApp
 * @brief The KSldApp class represents the application responsible for screen locking.
 *
 * KSldApp provides functionality for acquiring and releasing screen locks, managing lock states,
 * handling user activity, and interacting with the lock window. It also includes methods for
 * configuring the lock screen behavior and managing power management inhibition.
 *
 * KSldApp implements the D-Bus interface "org.kde.ksld.App".
 *
 * @note KSldApp is primarily used internally by the KDE screen locker component.
 */
class KSCREENLOCKER_EXPORT KSldApp : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ksld.App")

public:
    /**
     * @brief The LockState enum represents the current state of the screen lock.
     */
    enum LockState {
        /** The screen is unlocked. */
        Unlocked,
        /** The screen is in the process of being locked. */
        AcquiringLock,
        /** The screen is locked. */
        Locked,
    };
    Q_ENUM(LockState)

    static KSldApp *self();

    explicit KSldApp(QObject *parent = nullptr);
    ~KSldApp() override;

    /**
     * @returns the current lock state.
     **/
    LockState lockState() const
    {
        return m_lockState;
    }

    /**
     * @returns the number of milliseconds passed since the screen has been locked.
     **/
    uint activeTime() const;

    /**
     * @brief Configures the KSldApp instance.
     *
     * This function is responsible for configuring the KSldApp instance by loading the screen saver settings,
     * setting up the idle support, and managing the lock and password requirements.
     *
     * @note This function assumes that the KScreenSaverSettings and KIdleTime instances have been properly initialized.
     */
    void configure();

    /**
     * @brief Handles user activity.
     *
     * This function is called when user activity is detected.
     *
     * If the grace time is still active or password requirement is disabled,
     * it unlocks the screen. If a lock window is present, it also notifies
     * the lock window about the user activity.
     */
    void userActivity();

    /**
     * @return true if the application is in the grace time, false otherwise.
     */
    bool isGraceTime() const;

    /**
     * @brief Sets the Wayland file descriptor.
     *
     * @param fd The Wayland file descriptor to set.
     */
    void setWaylandFd(int fd);

    /**
     * @brief Sets the environment for the greeter process.
     *
     * @param env The QProcessEnvironment object containing the environment
     * variables to be set for the greeter process.
     */
    void setGreeterEnvironment(const QProcessEnvironment &env);

    /**
     * Can be used by the lock window to remove the lock during grace time.
     **/
    void unlock();

    /**
     * @brief Increments the inhibit counter.
     *
     * The inhibit counter is used to keep track of how many times the application
     * has been inhibited from locking the screen.
     */
    void inhibit();

    /**
     * @brief Decrements the inhibit counter.
     *
     * The inhibit counter is used to keep track of how many times the application
     * has been inhibited from locking the screen.
     */
    void uninhibit();

    /**
     * @brief Locks the screen.
     *
     * If the screen is already locked or in the process of acquiring the lock, verify that the lock is established.
     * If the screen is unlocked, it attempts to establish the lock by calling the establishGrab() function.
     * If the lock cannot be established, it retries up to three times.
     * If the lock is successfully established, it shows the lock window, starts the lock process, and emits the lockStateChanged() signal.
     *
     * @param establishLock The type of lock to establish (eg. immediate or delayed).
     * @param attemptCount The number of attempts made to establish the lock.
     */
    void lock(EstablishLock establishLock, int attemptCount = 0);

    /**
     * @brief Initializes the KSldApp object.
     */
    void initialize();

    /**
     * @brief Handles events for the KSldApp class.
     *
     * Checks if the event is a key press event and if a global accelerator is set.
     * If both conditions are met, it calls the keyEvent function of the global accelerator
     * and sets the event as accepted if the key event is handled.
     *
     * @param event The event to be handled.
     * @return Returns false to indicate that the event was not handled by this function.
     */
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

    /**
     * @brief Returns whether software rendering is forced.
     */
    bool forceSoftwareRendering() const
    {
        return m_forceSoftwareRendering;
    }

    /**
     * @brief Sets whether software rendering should be forced.
     */
    void setForceSoftwareRendering(bool force)
    {
        m_forceSoftwareRendering = force;
    }

Q_SIGNALS:

    /**
     * Emitted when the screen is about to be locked.
     */
    void aboutToLock();

    /**
     * Emitted before the greeter is (re)started
     */
    void aboutToStartGreeter();

    /**
     * Emitted when the screen has been locked.
     */
    void locked();

    /**
     * Emitted when the screen has been unlocked.
     */
    void unlocked();

    /**
     * Emitted when the lock state has changed.
     */
    void lockStateChanged();

private Q_SLOTS:

    /**
     * @brief Cleans up resources used by KSldApp.
     *
     * This function terminates the lock process if it is running, deletes the lock process and lock window objects,
     * and restores the X screensaver parameters to their original values.
     */
    void cleanUp();

    /**
     * @brief Ends the grace time.
     */
    void endGraceTime();

    /**
     * @brief Suspends the application and locks the screen if necessary.
     *
     * This function is called when the system is about to suspend. If the application is not using logind,
     * it checks if the screen should be locked upon resuming from suspend. If so, it locks the screen immediately.
     */
    void solidSuspend();

public Q_SLOTS:

    /**
     * @brief Called when the lock screen is shown.
     *
     * Updates the lock state and emits signals to inform other components.
     */
    void lockScreenShown();

private:
    /**
     * @brief Initializes the X11 environment for the KSldApp.
     */
    void initializeX11();

    /**
     * @brief Establishes the grab for keyboard and mouse input.
     *
     * Ensures that only the lock window receives keyboard and mouse input.
     *
     * @return True if the grab is successfully established, false otherwise.
     */
    bool establishGrab();

    /**
     * @brief Starts the lock process.
     *
     * This function starts the lock process based on the given EstablishLock parameter.
     * It sets up the necessary environment variables and command line arguments for the lock process.
     * If the Wayland server fails to start, an error is emitted and the function returns.
     *
     * @param establishLock The type of lock to establish (eg. immediate or delayed).
     */
    void startLockProcess(EstablishLock establishLock);

    /**
     * @brief Shows the lock window.
     *
     * Creates the lock window if it doesn't exist and sets up the necessary
     * connections.
     */
    void showLockWindow();

    /**
     * @brief Hides the lock window.
     */
    void hideLockWindow();

    /**
     * @brief Performs the unlocking operation.
     *
     * This function is responsible for unlocking the screen. It releases the keyboard and pointer grabs,
     * hides the lock window, and cleans up state.
     */
    void doUnlock();

    /**
     * @brief The lock process has requested to unlock the screen.
     *
     * Cleans up some state and begins the unlocking process.
     */
    void lockProcessRequestedUnlock();

    /**
     * @brief Checks whether the FDO (FreeDesktop.org) power is inhibited or not.
     *
     * @return true if the FDO power is inhibited, false otherwise.
     */
    bool isFdoPowerInhibited() const;

    /**
     * @brief The current state of the lock.
     */
    LockState m_lockState;

    /**
     * The lock process used to lock the screen.
     **/
    QProcess *m_lockProcess;

    /**
     * The lock window used to display the lock screen.
     **/
    AbstractLocker *m_lockWindow;

    /**
     * The Wayland server instance used by the lock window.
     **/
    WaylandServer *m_waylandServer;

    /**
     * Timer to measure how long the screen is locked.
     * This information is required by DBus Interface.
     **/
    QElapsedTimer m_lockedTimer;

    /**
     * @brief The unique identifier for the idle timer.
     */
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

    /**
     * @brief The counter for inhibiting screen locking.
     *
     * This variable keeps track of the number of inhibitions on screen locking.
     */
    int m_inhibitCounter;

    /**
     * Provides handling for interactions with the logind service, such as
     * session management and power management.
     */
    LogindIntegration *m_logind;

    /**
     * @brief Provides handling for global accelerators.
     */
    GlobalAccel *m_globalAccel = nullptr;

    /**
     * @brief Whether the XInput2 extension is available.
     */
    bool m_hasXInput2 = false;

    /**
     * @brief Whether software rendering should be forced.
     */
    bool m_forceSoftwareRendering = false;

    /**
     * @brief Whether the application is running on X11.
     */
    bool m_isX11;

    /**
     * @brief Whether the application is running on Wayland.
     */
    bool m_isWayland;

    /**
     * @brief Counter for tracking the number of times the greeter has crashed.
     */
    int m_greeterCrashedCounter = 0;

    /**
     * @brief The environment for the greeter process.
     */
    QProcessEnvironment m_greeterEnv;

    /**
     * @brief Interface for power management inhibition.
     */
    PowerManagementInhibition *m_powerManagementInhibition;

    /**
     * Whether the lock screen should require a password to unlock.
     *
     * If this is false the lock screen will be dismissed on user activity,
     * without requiring a password. This allows users to use the lock screen
     * as a screen saver.
     **/
    bool m_requirePassword = true;

    /**
     * @brief The file descriptor for the Wayland connection.
     *
     * This file descriptor is used to establish the Wayland connection for the
     * lock window.
     */
    int m_waylandFd = -1;

    // for auto tests
    friend KSldTest;
};
} // namespace
