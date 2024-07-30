/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <QPointer>
#include <QSet>
#include <QStringList>

#include <QDBusContext>
#include <QDBusError>
#include <QDBusMessage>
#include <QDBusObjectPath>

#include <QSocketNotifier>

#include <KSharedConfig>

#include "controllers/batterycontroller.h"
#include "controllers/keyboardbrightnesscontroller.h"
#include "controllers/lidcontroller.h"
#include "controllers/screenbrightnesscontroller.h"
#include "controllers/suspendcontroller.h"
#include "powerdevilcore_export.h"

namespace KActivities
{
class Consumer;
} // namespace KActivities

class QDBusServiceWatcher;
class QTimer;
class KNotification;

namespace Solid
{
class Battery;
}

namespace PowerDevil
{
class Action;
class GlobalSettings;

struct WakeupInfo {
    QString service;
    QDBusObjectPath path;
    int cookie;
    qint64 timeout;
};

class POWERDEVILCORE_EXPORT Core : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_DISABLE_COPY(Core)
    Q_CLASSINFO("D-Bus Interface", "org.kde.Solid.PowerManagement")

public:
    explicit Core(QObject *parent);
    ~Core() override;

    void reloadProfile(int state);

    void emitRichNotification(const QString &evid, const QString &title, const QString &message = QString());

    void emitNotification(const QString &eventId, const QString &title, const QString &message, const QString &iconName);

    enum class ChargeNotificationFlag {
        None,
        NotifyWhenAcPluggedIn,
    };
    Q_DECLARE_FLAGS(ChargeNotificationFlags, ChargeNotificationFlag)
    Q_FLAG(ChargeNotificationFlags)

    bool emitBatteryChargePercentNotification(int currentPercent, int previousPercent, const QString &udi = QString(), ChargeNotificationFlags flags = {});

    SuspendController *suspendController();
    BatteryController *batteryController();
    LidController *lidController();
    KeyboardBrightnessController *keyboardBrightnessController();
    ScreenBrightnessController *screenBrightnessController();

    Action *action(const QString actionId);

    // More...

public Q_SLOTS:
    void loadCore();
    // Set of common action - useful for the DBus interface
    uint backendCapabilities();
    void refreshStatus();
    void reparseConfiguration();
    void onControllersReady();
    void refreshActions();

    QString currentProfile() const;
    void loadProfile(bool force = false);

    qulonglong batteryRemainingTime() const;
    qulonglong smoothedBatteryRemainingTime() const;

    bool isLidClosed() const;
    bool isLidPresent() const;
    bool isActionSupported(const QString &actionName);
    bool hasDualGpu() const;
    bool isBatteryConservationModeEnabled() const;
    int chargeStartThreshold() const;
    int chargeStopThreshold() const;

    // service - dbus interface to ping when wakeup is done
    // path - dbus path on service
    // cookie - data to pass back
    // silent - true if silent wakeup is needed
    uint scheduleWakeup(const QString &service, const QDBusObjectPath &path, qint64 timeout);
    void wakeup();
    void clearWakeup(int cookie);

Q_SIGNALS:
    void coreReady();
    void profileChanged(const QString &newProfile);
    void configurationReloaded();
    void batteryRemainingTimeChanged(qulonglong time);
    void smoothedBatteryRemainingTimeChanged(qulonglong time);
    void lidClosedChanged(bool closed);
    void batteryConservationModeChanged(bool enabled);
    void chargeStartThresholdChanged(int threshold);
    void chargeStopThresholdChanged(int threshold);

private:
    void unloadAllActiveActions();
    void registerActionTimeout(Action *action, std::chrono::milliseconds timeout);
    void unregisterActionTimeouts(Action *action);
    void handleLowBattery(int percent);
    void handleCriticalBattery(int percent);
    void updateBatteryNotifications(int percent);
    void triggerCriticalBatteryAction();

    void readChargeThreshold();
    void readBatteryConservationMode();

    /**
     * Computes the current global charge percentage.
     * Sum of all battery charges.
     */
    int currentChargePercent() const;

    friend class Action;

    bool m_hasDualGpu;
    bool m_batteryConservationModeEnabled = false;
    int m_chargeStartThreshold = 0;
    int m_chargeStopThreshold = 100;

    std::unique_ptr<SuspendController> m_suspendController;
    std::unique_ptr<BatteryController> m_batteryController;
    std::unique_ptr<LidController> m_lidController;
    std::unique_ptr<KeyboardBrightnessController> m_keyboardBrightnessController;
    std::unique_ptr<ScreenBrightnessController> m_screenBrightnessController;

    QDBusServiceWatcher *m_notificationsWatcher = nullptr;
    bool m_notificationsReady = false;

    GlobalSettings *m_globalSettings = nullptr;

    QString m_currentProfile;

    QHash<QString, int> m_batteriesPercent;
    QHash<QString, int> m_peripheralBatteriesPercent;
    QHash<QString, bool> m_batteriesCharged;

    QPointer<KNotification> m_lowBatteryNotification;
    QTimer *const m_criticalBatteryTimer;
    QPointer<KNotification> m_criticalBatteryNotification;

    KActivities::Consumer *const m_activityConsumer;

    // Idle time management
    QHash<Action *, QList<int>> m_registeredActionTimeouts;
    QSet<Action *> m_pendingResumeFromIdleActions;
    bool m_pendingWakeupEvent;

    // Scheduled wakeups and alarms
    QList<WakeupInfo> m_scheduledWakeups;
    int m_lastWakeupCookie = 0;
    int m_timerFd = -1;
    QSocketNotifier *m_timerFdSocketNotifier = nullptr;

    // Activity inhibition management
    QHash<QString, int> m_sessionActivityInhibit;
    QHash<QString, int> m_screenActivityInhibit;

    std::unordered_map<QString, std::unique_ptr<Action>> m_actionPool;
    QStringList m_activeActions;

private Q_SLOTS:
    void onAcAdapterStateChanged(BatteryController::AcAdapterState);
    void onBatteryChargePercentChanged(int, const QString &);
    void onBatteryChargeStateChanged(int, const QString &);
    void onBatteryRemainingTimeChanged(qulonglong);
    void onSmoothedBatteryRemainingTimeChanged(qulonglong);
    void onKIdleTimeoutReached(int, int);
    void onResumingFromIdle();
    void onDeviceAdded(const QString &udi);
    void onDeviceRemoved(const QString &udi);
    void onCriticalBatteryTimerExpired();
    void onNotificationTimeout();
    void onServiceRegistered(const QString &service);
    void onLidClosedChanged(bool closed);
    void onAboutToSuspend();
    // handlers for handling wakeup dbus call
    void resetAndScheduleNextWakeup();
    void timerfdEventHandler();
};

}
