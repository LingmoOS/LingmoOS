/*
 *   SPDX-FileCopyrightText: 2010 Dario Freddi <drf@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "powerdevilcore.h"

#include "powerdevil_debug.h"
#include "powerdevilaction.h"
#include "powerdevilenums.h"
#include "powerdevilmigrateconfig.h"
#include "powerdevilpolicyagent.h"
#include "powerdevilpowermanagement.h"

#include <PowerDevilActivitySettings.h>
#include <PowerDevilGlobalSettings.h>
#include <PowerDevilProfileSettings.h>

#include <Solid/Battery>
#include <Solid/Device>
#include <Solid/DeviceNotifier>

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <kauth_version.h>

#include <KIdleTime>
#include <KLocalizedString>
#include <KNotification>
#include <KPluginFactory>
#include <KPluginMetaData>

#include <PlasmaActivities/Consumer>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>
#include <QTimer>

#include <QDebug>

#include <Kirigami/Platform/TabletModeWatcher>
#include <algorithm>

#ifdef Q_OS_LINUX
#include <sys/timerfd.h>
#endif

using namespace Qt::StringLiterals;

namespace PowerDevil
{
Core::Core(QObject *parent)
    : QObject(parent)
    , m_hasDualGpu(false)
    , m_criticalBatteryTimer(new QTimer(this))
    , m_activityConsumer(new KActivities::Consumer(this))
    , m_pendingWakeupEvent(true)
{
    KAuth::Action discreteGpuAction(QStringLiteral("org.kde.powerdevil.discretegpuhelper.hasdualgpu"));
    discreteGpuAction.setHelperId(QStringLiteral("org.kde.powerdevil.discretegpuhelper"));
    KAuth::ExecuteJob *discreteGpuJob = discreteGpuAction.execute();
    connect(discreteGpuJob, &KJob::result, this, [this, discreteGpuJob] {
        if (discreteGpuJob->error()) {
            qCWarning(POWERDEVIL) << "org.kde.powerdevil.discretegpuhelper.hasdualgpu failed";
            qCDebug(POWERDEVIL) << discreteGpuJob->errorText();
            return;
        }
        m_hasDualGpu = discreteGpuJob->data()[QStringLiteral("hasdualgpu")].toBool();
    });
    discreteGpuJob->start();

    readChargeThreshold();
    readBatteryConservationMode();
}

Core::~Core()
{
    qCDebug(POWERDEVIL) << "Core unloading";
    // Unload all actions before exiting
    unloadAllActiveActions();
}

void Core::loadCore()
{
    m_suspendController = std::make_unique<SuspendController>();
    m_batteryController = std::make_unique<BatteryController>();
    m_lidController = std::make_unique<LidController>();
    m_keyboardBrightnessController = std::make_unique<KeyboardBrightnessController>();
    m_screenBrightnessController = std::make_unique<ScreenBrightnessController>();

    connect(m_screenBrightnessController.get(), &ScreenBrightnessController::detectionFinished, this, &Core::onControllersReady);
    connect(m_screenBrightnessController.get(), &ScreenBrightnessController::displayAdded, this, &Core::refreshActions);
    connect(m_screenBrightnessController.get(), &ScreenBrightnessController::displayRemoved, this, &Core::refreshActions);
    m_screenBrightnessController->detectDisplays();
}

void Core::onControllersReady()
{
    qCDebug(POWERDEVIL) << "Controllers ready, KDE Power Management system initialized";

    const bool isMobile = Kirigami::Platform::TabletModeWatcher::self()->isTabletMode();
    const bool isVM = PowerDevil::PowerManagement::instance()->isVirtualMachine();
    const bool canSuspend = m_suspendController->canSuspend();
    const bool canHibernate = m_suspendController->canHibernate();

    PowerDevil::migrateConfig(isMobile, isVM, canSuspend);
    m_globalSettings = new PowerDevil::GlobalSettings(canSuspend, canHibernate, this);

    // Get the battery devices ready
    {
        using namespace Solid;
        connect(DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &Core::onDeviceAdded);
        connect(DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &Core::onDeviceRemoved);

        // Force the addition of already existent batteries
        const auto devices = Device::listFromType(DeviceInterface::Battery, QString());
        for (const Device &device : devices) {
            onDeviceAdded(device.udi());
        }
    }

    connect(m_batteryController.get(), &BatteryController::acAdapterStateChanged, this, &Core::onAcAdapterStateChanged);
    connect(m_batteryController.get(), &BatteryController::batteryRemainingTimeChanged, this, &Core::onBatteryRemainingTimeChanged);
    connect(m_batteryController.get(), &BatteryController::smoothedBatteryRemainingTimeChanged, this, &Core::onSmoothedBatteryRemainingTimeChanged);
    connect(m_lidController.get(), &LidController::lidClosedChanged, this, &Core::onLidClosedChanged);
    connect(m_suspendController.get(), &SuspendController::aboutToSuspend, this, &Core::onAboutToSuspend);
    connect(KIdleTime::instance(), &KIdleTime::timeoutReached, this, &Core::onKIdleTimeoutReached);
    connect(KIdleTime::instance(), &KIdleTime::resumingFromIdle, this, &Core::onResumingFromIdle);
    connect(m_activityConsumer, &KActivities::Consumer::currentActivityChanged, this, [this]() {
        loadProfile();
    });

    // Set up the policy agent
    PowerDevil::PolicyAgent::instance()->init();
    // When inhibitions change, simulate user activity, see Bug 315438
    connect(PowerDevil::PolicyAgent::instance(),
            &PowerDevil::PolicyAgent::unavailablePoliciesChanged,
            this,
            [](PowerDevil::PolicyAgent::RequiredPolicies newPolicies) {
                Q_UNUSED(newPolicies);
                KIdleTime::instance()->simulateUserActivity();
            });

    // Bug 354250: Simulate user activity when session becomes inactive,
    // this keeps us from sending the computer to sleep when switching to an idle session.
    // (this is just being lazy as it will result in us clearing everything
    connect(PowerDevil::PolicyAgent::instance(), &PowerDevil::PolicyAgent::sessionActiveChanged, this, [this](bool active) {
        if (active) {
            // force reload profile so all actions re-register their idle timeouts
            loadProfile(true /*force*/);
        } else {
            // Bug 354250: Keep us from sending the computer to sleep when switching
            // to an idle session by removing all idle timeouts
            KIdleTime::instance()->removeAllIdleTimeouts();
            m_registeredActionTimeouts.clear();
        }
    });

    // Initialize the action pool, which will also load the needed startup actions.
    refreshActions();

    // Set up the critical battery timer
    m_criticalBatteryTimer->setSingleShot(true);
    m_criticalBatteryTimer->setInterval(60000);
    connect(m_criticalBatteryTimer, &QTimer::timeout, this, &Core::onCriticalBatteryTimerExpired);

    // wait until the notification system is set up before firing notifications
    // to avoid them showing ontop of ksplash...
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.freedesktop.Notifications"))) {
        onServiceRegistered(QString());
    } else {
        m_notificationsWatcher = new QDBusServiceWatcher(QStringLiteral("org.freedesktop.Notifications"),
                                                         QDBusConnection::sessionBus(),
                                                         QDBusServiceWatcher::WatchForRegistration,
                                                         this);
        connect(m_notificationsWatcher, &QDBusServiceWatcher::serviceRegistered, this, &Core::onServiceRegistered);

        // ...but fire them after 30s nonetheless to ensure they've been shown
        QTimer::singleShot(30000, this, &Core::onNotificationTimeout);
    }

#ifdef Q_OS_LINUX

    // try creating a timerfd which can wake system from suspend
    m_timerFd = timerfd_create(CLOCK_REALTIME_ALARM, TFD_CLOEXEC);

    // if that fails due to privilges maybe, try normal timerfd
    if (m_timerFd == -1) {
        qCDebug(POWERDEVIL)
            << "Unable to create a CLOCK_REALTIME_ALARM timer, trying CLOCK_REALTIME\n This would mean that wakeup requests won't wake device from suspend";
        m_timerFd = timerfd_create(CLOCK_REALTIME, TFD_CLOEXEC);
    }

    if (m_timerFd != -1) {
        m_timerFdSocketNotifier = new QSocketNotifier(m_timerFd, QSocketNotifier::Read, this);
        connect(m_timerFdSocketNotifier, &QSocketNotifier::activated, this, &Core::timerfdEventHandler);
        // we disable events reading for now
        m_timerFdSocketNotifier->setEnabled(false);
    } else {
        qCDebug(POWERDEVIL) << "Unable to create a CLOCK_REALTIME timer, scheduled wakeups won't be available";
    }

#endif

    // All systems up Houston, let's go!
    Q_EMIT coreReady();
    refreshStatus();
}

void Core::refreshActions()
{
    const QList<KPluginMetaData> offers = KPluginMetaData::findPlugins(QStringLiteral("powerdevil/action"));
    for (const KPluginMetaData &data : offers) {
        const QString actionId = data.value(QStringLiteral("X-KDE-PowerDevil-Action-ID"));
        if (m_actionPool.contains(actionId)) {
            continue;
        }
        std::unique_ptr<PowerDevil::Action> action{KPluginFactory::instantiatePlugin<PowerDevil::Action>(data, this).plugin};
        if (action && action->isSupported()) {
            if (data.value(QStringLiteral("X-KDE-PowerDevil-Action-RegistersDBusInterface"), false)) {
                QDBusConnection::sessionBus().registerObject(QStringLiteral("/org/kde/Solid/PowerManagement/Actions/") + actionId, action.get());
            }
            m_actionPool.emplace(actionId, std::move(action));
        }
    }

    // Remove now-unsupported actions
    for (auto it = m_actionPool.begin(); it != m_actionPool.end();) {
        Action *action = it->second.get();
        if (!action->isSupported()) {
            m_registeredActionTimeouts.remove(action);
            m_pendingResumeFromIdleActions.remove(action);
            it = m_actionPool.erase(it);
        } else {
            ++it;
        }
    }
}

bool Core::isActionSupported(const QString &actionName)
{
    Action *act = action(actionName);
    return act ? act->isSupported() : false;
}

void Core::refreshStatus()
{
    /* The configuration could have changed if this function was called, so
     * let's resync it.
     */
    reparseConfiguration();

    loadProfile(true);
}

void Core::reparseConfiguration()
{
    m_globalSettings->load();

    // Config reloaded
    Q_EMIT configurationReloaded();

    // Check if critical threshold might have changed and cancel the timer if necessary.
    if (currentChargePercent() > m_globalSettings->batteryCriticalLevel()) {
        m_criticalBatteryTimer->stop();
        if (m_criticalBatteryNotification) {
            m_criticalBatteryNotification->close();
        }
    }

    if (m_lowBatteryNotification && currentChargePercent() > m_globalSettings->batteryLowLevel()) {
        m_lowBatteryNotification->close();
    }

    readChargeThreshold();
    readBatteryConservationMode();
}

QString Core::currentProfile() const
{
    return m_currentProfile;
}

void Core::loadProfile(bool force)
{
    QString profileId;

    // Check the activity in which we are in
    QString activity = m_activityConsumer->currentActivity();
    qCDebug(POWERDEVIL) << "Currently using activity " << activity;

    PowerDevil::ActivitySettings activitySettings(activity);

    qCDebug(POWERDEVIL) << "Settings for loaded activity:";
    for (KConfigSkeletonItem *item : activitySettings.items()) {
        qCDebug(POWERDEVIL) << item->key() << "=" << item->property();
    }

    // let's load the current state's profile
    if (m_batteriesPercent.isEmpty()) {
        qCDebug(POWERDEVIL) << "No batteries found, loading AC";
        profileId = QStringLiteral("AC");
    } else {
        // Compute the previous and current global percentage
        const int percent = currentChargePercent();

        if (m_batteryController->acAdapterState() == BatteryController::Plugged) {
            profileId = QStringLiteral("AC");
            qCDebug(POWERDEVIL) << "Loading profile for plugged AC";
        } else if (percent <= m_globalSettings->batteryLowLevel()) {
            profileId = QStringLiteral("LowBattery");
            qCDebug(POWERDEVIL) << "Loading profile for low battery";
        } else {
            profileId = QStringLiteral("Battery");
            qCDebug(POWERDEVIL) << "Loading profile for unplugged AC";
        }
    }

    // Load settings for the current profile
    const bool isMobile = Kirigami::Platform::TabletModeWatcher::self()->isTabletMode();
    const bool isVM = PowerDevil::PowerManagement::instance()->isVirtualMachine();
    bool canSuspend = m_suspendController->canSuspend();

    PowerDevil::ProfileSettings profileSettings(profileId, isMobile, isVM, canSuspend);

    // Release any special inhibitions
    {
        QHash<QString, int>::iterator i = m_sessionActivityInhibit.begin();
        while (i != m_sessionActivityInhibit.end()) {
            PolicyAgent::instance()->ReleaseInhibition(i.value());
            i = m_sessionActivityInhibit.erase(i);
        }

        i = m_screenActivityInhibit.begin();
        while (i != m_screenActivityInhibit.end()) {
            PolicyAgent::instance()->ReleaseInhibition(i.value());
            i = m_screenActivityInhibit.erase(i);
        }
    }

    // Check: do we need to change profile at all?
    if (m_currentProfile == profileId && !force) {
        // No, let's leave things as they are
        qCDebug(POWERDEVIL) << "Skipping action reload routine as profile has not changed";

        // Do we need to force a wakeup?
        if (m_pendingWakeupEvent) {
            // Fake activity at this stage, when no timeouts are registered
            onResumingFromIdle();
            m_pendingWakeupEvent = false;
        }
    } else {
        // First of all, let's clean the old actions. This will also call the onProfileUnload callback
        unloadAllActiveActions();

        // Do we need to force a wakeup?
        if (m_pendingWakeupEvent) {
            // Fake activity at this stage, when no timeouts are registered
            onResumingFromIdle();
            m_pendingWakeupEvent = false;
        }

        // Cool, now let's load the needed actions. Mark the ones as active that want to be loaded
        for (const auto &[id, action] : m_actionPool) {
            if (action->loadAction(profileSettings)) {
                m_activeActions.append(id);
                action->onProfileLoad(m_currentProfile, profileId);
            }
        }

        // We are now on a different profile
        m_currentProfile = profileId;
        Q_EMIT profileChanged(m_currentProfile);
    }

    // Now... any special behaviors we'd like to consider?
    if (activitySettings.inhibitSuspend()) {
        qCDebug(POWERDEVIL) << "Activity triggers a suspend inhibition"; // debug hence not sleep
        // Trigger a special inhibition - if we don't have one yet
        if (!m_sessionActivityInhibit.contains(activity)) {
            int cookie = PolicyAgent::instance()->AddInhibition(PolicyAgent::InterruptSession,
                                                                i18n("Activity Manager"),
                                                                i18n("This activity's policies prevent the system from going to sleep"));

            m_sessionActivityInhibit.insert(activity, cookie);
        }
    }
    if (activitySettings.inhibitScreenManagement()) {
        qCDebug(POWERDEVIL) << "Activity triggers a screen management inhibition";
        // Trigger a special inhibition - if we don't have one yet
        if (!m_screenActivityInhibit.contains(activity)) {
            int cookie = PolicyAgent::instance()->AddInhibition(PolicyAgent::ChangeScreenSettings,
                                                                i18n("Activity Manager"),
                                                                i18n("This activity's policies prevent screen power management"));

            m_screenActivityInhibit.insert(activity, cookie);
        }
    }

    // If the lid is closed, retrigger the lid close signal
    // so that "switching profile then closing the lid" has the same result as
    // "closing lid then switching profile".
    if (m_lidController->isLidClosed()) {
        Q_EMIT m_lidController->lidClosedChanged(true);
    }
}

void Core::onDeviceAdded(const QString &udi)
{
    if (m_batteriesPercent.contains(udi) || m_peripheralBatteriesPercent.contains(udi)) {
        // We already know about this device
        return;
    }

    using namespace Solid;
    Device device(udi);
    Battery *b = qobject_cast<Battery *>(device.asDeviceInterface(DeviceInterface::Battery));

    if (!b) {
        return;
    }

    connect(b, &Battery::chargePercentChanged, this, &Core::onBatteryChargePercentChanged);
    connect(b, &Battery::chargeStateChanged, this, &Core::onBatteryChargeStateChanged);

    qCDebug(POWERDEVIL) << "Battery with UDI" << udi << "was detected";

    if (b->isPowerSupply()) {
        m_batteriesPercent[udi] = b->chargePercent();
        m_batteriesCharged[udi] = (b->chargeState() == Solid::Battery::FullyCharged);
    } else { // non-power supply batteries are treated separately
        m_peripheralBatteriesPercent[udi] = b->chargePercent();

        // notify the user about the empty mouse/keyboard when plugging it in; don't when
        // notifications aren't ready yet so we avoid showing them ontop of ksplash;
        // also we'll notify about all devices when notifications are ready anyway
        if (m_notificationsReady) {
            emitBatteryChargePercentNotification(b->chargePercent(), 1000 /* so current is always lower than previous */, udi);
        }
    }

    // If a new battery has been added, let's clear some pending suspend actions if the new global batteries percentage is
    // higher than the battery critical level. (See bug 329537)
    if (m_lowBatteryNotification && currentChargePercent() > m_globalSettings->batteryLowLevel()) {
        m_lowBatteryNotification->close();
    }

    if (currentChargePercent() > m_globalSettings->batteryCriticalLevel()) {
        if (m_criticalBatteryNotification) {
            m_criticalBatteryNotification->close();
        }

        if (m_criticalBatteryTimer->isActive()) {
            m_criticalBatteryTimer->stop();
            emitRichNotification(QStringLiteral("pluggedin"), //
                                 i18n("Extra Battery Added"),
                                 i18n("The computer will no longer go to sleep."));
        }
    }
}

void Core::onDeviceRemoved(const QString &udi)
{
    if (!m_batteriesPercent.contains(udi) && !m_peripheralBatteriesPercent.contains(udi)) {
        // We don't know about this device
        return;
    }

    using namespace Solid;
    Device device(udi);
    // No need to undo the connections from onDeviceAdded,
    // because the backend object has already been deleted.
    Q_ASSERT(!device.isValid());

    qCDebug(POWERDEVIL) << "Battery with UDI" << udi << "has been removed";

    m_batteriesPercent.remove(udi);
    m_peripheralBatteriesPercent.remove(udi);
    m_batteriesCharged.remove(udi);
}

void Core::emitNotification(const QString &eventId, const QString &title, const QString &message, const QString &iconName)
{
    KNotification::event(eventId, title, message, iconName, KNotification::CloseOnTimeout, QStringLiteral("powerdevil"));
}

void Core::emitRichNotification(const QString &evid, const QString &title, const QString &message)
{
    KNotification::event(evid, title, message, QPixmap(), KNotification::CloseOnTimeout, QStringLiteral("powerdevil"));
}

bool Core::emitBatteryChargePercentNotification(int currentPercent, int previousPercent, const QString &udi, Core::ChargeNotificationFlags flags)
{
    if (m_peripheralBatteriesPercent.contains(udi)) {
        // Show the notification just once on each normal->low transition
        if (currentPercent > m_globalSettings->peripheralBatteryLowLevel() || previousPercent <= m_globalSettings->peripheralBatteryLowLevel()) {
            return false;
        }

        using namespace Solid;
        Device device(udi);
        Battery *b = qobject_cast<Battery *>(device.asDeviceInterface(DeviceInterface::Battery));
        if (!b) {
            return false;
        }

        // if you leave the device out of reach or it has not been initialized yet
        // it won't be "there" and report 0%, don't show anything in this case
        if (!b->isPresent() || b->chargePercent() == 0) {
            return false;
        }

        // Bluetooth devices don't report charge state, so it's "NoCharge" in all cases for them
        if (b->chargeState() != Battery::Discharging && b->chargeState() != Battery::NoCharge) {
            return false;
        }

        {
            QString name = device.product();
            if (!device.vendor().isEmpty()) {
                name = i18nc("%1 is vendor name, %2 is product name", "%1 %2", device.vendor(), device.product());
            }

            QString title = i18nc("The battery in an external device", "Device Battery Low (%1% Remaining)", currentPercent);
            QString msg = i18nc("Placeholder is device name",
                                "The battery in \"%1\" is running low, and the device may turn off at any time. "
                                "Please recharge or replace the battery.",
                                name);
            QString icon = QStringLiteral("battery-caution");

            switch (b->type()) {
            case Battery::MouseBattery:
                title = i18n("Mouse Battery Low (%1% Remaining)", currentPercent);
                icon = QStringLiteral("input-mouse");
                break;
            case Battery::KeyboardBattery:
                title = i18n("Keyboard Battery Low (%1% Remaining)", currentPercent);
                icon = QStringLiteral("input-keyboard");
                break;
            case Battery::BluetoothBattery:
                title = i18n("Bluetooth Device Battery Low (%1% Remaining)", currentPercent);
                msg = i18nc("Placeholder is device name",
                            "The battery in Bluetooth device \"%1\" is running low, and the device may turn off at any time. "
                            "Please recharge or replace the battery.",
                            name);
                icon = QStringLiteral("preferences-system-bluetooth");
                break;
            default:
                break;
            }

            emitNotification(QStringLiteral("lowperipheralbattery"), title, msg, icon);
        }

        return true;
    }

    // Make sure a notificaton that's kept open updates its percentage live.
    updateBatteryNotifications(currentPercent);

    if (m_batteryController->acAdapterState() == BatteryController::Plugged && !flags.testFlag(ChargeNotificationFlag::NotifyWhenAcPluggedIn)) {
        return false;
    }

    if (currentPercent <= m_globalSettings->batteryCriticalLevel() && previousPercent > m_globalSettings->batteryCriticalLevel()) {
        handleCriticalBattery(currentPercent);
        return true;
    } else if (currentPercent <= m_globalSettings->batteryLowLevel() && previousPercent > m_globalSettings->batteryLowLevel()) {
        handleLowBattery(currentPercent);
        return true;
    }
    return false;
}

void Core::handleLowBattery(int percent)
{
    if (m_lowBatteryNotification) {
        return;
    }

    m_lowBatteryNotification = new KNotification(QStringLiteral("lowbattery"), KNotification::Persistent, nullptr);
    m_lowBatteryNotification->setComponentName(QStringLiteral("powerdevil"));
    updateBatteryNotifications(percent); // sets title
    if (m_batteryController->acAdapterState() == BatteryController::Plugged) {
        m_lowBatteryNotification->setText(i18n("Ensure that the power adapter is plugged in and provides enough power."));
    } else {
        m_lowBatteryNotification->setText(i18n("Plug in the computer."));
    }
    m_lowBatteryNotification->setUrgency(KNotification::CriticalUrgency);
    m_lowBatteryNotification->sendEvent();
}

void Core::handleCriticalBattery(int percent)
{
    if (m_lowBatteryNotification) {
        m_lowBatteryNotification->close();
    }

    // no parent, but it won't leak, since it will be closed both in case of timeout or direct action
    m_criticalBatteryNotification = new KNotification(QStringLiteral("criticalbattery"), KNotification::Persistent, nullptr);
    m_criticalBatteryNotification->setComponentName(QStringLiteral("powerdevil"));
    updateBatteryNotifications(percent); // sets title

    switch (static_cast<PowerButtonAction>(m_globalSettings->batteryCriticalAction())) {
    case PowerButtonAction::Shutdown: {
        m_criticalBatteryNotification->setText(i18n("Battery level critical. Your computer will shut down in 60 seconds."));
        auto action =
            m_criticalBatteryNotification->addAction(i18nc("@action:button Shut down without waiting for the battery critical timer", "Shut Down Now"));
        connect(action, &KNotificationAction::activated, this, &Core::triggerCriticalBatteryAction);
        m_criticalBatteryTimer->start();
        break;
    }
    case PowerButtonAction::Hibernate: {
        m_criticalBatteryNotification->setText(i18n("Battery level critical. Your computer will enter hibernation mode in 60 seconds."));
        auto action = m_criticalBatteryNotification->addAction(
            i18nc("@action:button Enter hibernation mode without waiting for the battery critical timer", "Hibernate Now"));
        connect(action, &KNotificationAction::activated, this, &Core::triggerCriticalBatteryAction);
        m_criticalBatteryTimer->start();
        break;
    }
    case PowerButtonAction::Sleep: {
        m_criticalBatteryNotification->setText(i18n("Battery level critical. Your computer will go to sleep in 60 seconds."));
        auto action =
            m_criticalBatteryNotification->addAction(i18nc("@action:button Suspend to ram without waiting for the battery critical timer", "Sleep Now"));
        connect(action, &KNotificationAction::activated, this, &Core::triggerCriticalBatteryAction);
        m_criticalBatteryTimer->start();
        break;
    }
    default:
        m_criticalBatteryNotification->setText(i18n("Please save your work."));
        // no timer, no actions
        break;
    }

    auto cancelAction =
        m_criticalBatteryNotification->addAction(i18nc("Cancel timeout that will automatically put system to sleep because of low battery", "Cancel"));
    connect(cancelAction, &KNotificationAction::activated, this, [this] {
        m_criticalBatteryTimer->stop();
        m_criticalBatteryNotification->close();
    });

    m_criticalBatteryNotification->sendEvent();
}

void Core::updateBatteryNotifications(int percent)
{
    if (m_lowBatteryNotification) {
        m_lowBatteryNotification->setTitle(i18n("Battery Low (%1% Remaining)", percent));
    }

    if (m_criticalBatteryNotification) {
        m_criticalBatteryNotification->setTitle(i18n("Battery Critical (%1% Remaining)", percent));
    }
}

void Core::onAcAdapterStateChanged(BatteryController::AcAdapterState state)
{
    qCDebug(POWERDEVIL);
    // Post request for faking an activity event - usually adapters don't plug themselves out :)
    m_pendingWakeupEvent = true;
    loadProfile();

    if (state == BatteryController::Plugged) {
        // If the AC Adaptor has been plugged in, let's clear some pending suspend actions
        if (m_lowBatteryNotification) {
            m_lowBatteryNotification->close();
        }

        if (m_criticalBatteryNotification) {
            m_criticalBatteryNotification->close();
        }

        if (m_criticalBatteryTimer->isActive()) {
            m_criticalBatteryTimer->stop();
            emitRichNotification(QStringLiteral("pluggedin"), //
                                 i18n("AC Adapter Plugged In"),
                                 i18n("The computer will no longer go to sleep."));
        } else {
            emitRichNotification(QStringLiteral("pluggedin"), i18n("Running on AC power"), i18n("The power adapter has been plugged in."));
        }
    } else if (state == BatteryController::Unplugged) {
        emitRichNotification(QStringLiteral("unplugged"), i18n("Running on Battery Power"), i18n("The power adapter has been unplugged."));
    }
}

void Core::onBatteryChargePercentChanged(int percent, const QString &udi)
{
    if (m_peripheralBatteriesPercent.contains(udi)) {
        const int previousPercent = m_peripheralBatteriesPercent.value(udi);
        m_peripheralBatteriesPercent[udi] = percent;

        if (percent < previousPercent) {
            emitBatteryChargePercentNotification(percent, previousPercent, udi);
        }
        return;
    }

    // Compute the previous and current global percentage
    const int previousPercent = currentChargePercent();
    const int currentPercent = previousPercent - (m_batteriesPercent[udi] - percent);

    // Update the battery percentage
    m_batteriesPercent[udi] = percent;

    if (currentPercent < previousPercent) {
        // When battery drains while still plugged in, warn nevertheless.
        if (emitBatteryChargePercentNotification(currentPercent, previousPercent, udi, ChargeNotificationFlag::NotifyWhenAcPluggedIn)) {
            // Only refresh status if a notification has actually been emitted
            loadProfile();
        }
    }
}

void Core::onBatteryChargeStateChanged(int state, const QString &udi)
{
    if (!m_batteriesCharged.contains(udi)) {
        return;
    }

    bool previousCharged = true;
    for (auto i = m_batteriesCharged.constBegin(); i != m_batteriesCharged.constEnd(); ++i) {
        if (!i.value()) {
            previousCharged = false;
            break;
        }
    }

    m_batteriesCharged[udi] = (state == Solid::Battery::FullyCharged);

    if (m_batteryController->acAdapterState() != BatteryController::Plugged) {
        return;
    }

    bool currentCharged = true;
    for (auto i = m_batteriesCharged.constBegin(); i != m_batteriesCharged.constEnd(); ++i) {
        if (!i.value()) {
            currentCharged = false;
            break;
        }
    }

    if (!previousCharged && currentCharged) {
        emitRichNotification(QStringLiteral("fullbattery"), i18n("Charging Complete"), i18n("Battery now fully charged."));
        loadProfile();
    }
}

void Core::onCriticalBatteryTimerExpired()
{
    if (m_criticalBatteryNotification) {
        m_criticalBatteryNotification->close();
    }

    // Do that only if we're not on AC
    if (m_batteryController->acAdapterState() == BatteryController::Unplugged) {
        triggerCriticalBatteryAction();
    }
}

void Core::triggerCriticalBatteryAction()
{
    PowerDevil::Action *helperAction = action(QStringLiteral("SuspendSession"));
    if (helperAction) {
        QVariantMap args;
        args[QStringLiteral("Type")] = QVariant::fromValue<uint>(m_globalSettings->batteryCriticalAction());
        args[QStringLiteral("Explicit")] = true;
        helperAction->trigger(args);
    }
}

void Core::onBatteryRemainingTimeChanged(qulonglong time)
{
    Q_EMIT batteryRemainingTimeChanged(time);
}

void Core::onSmoothedBatteryRemainingTimeChanged(qulonglong time)
{
    Q_EMIT smoothedBatteryRemainingTimeChanged(time);
}

void Core::onKIdleTimeoutReached(int identifier, int msec)
{
    // Find which action(s) requested this idle timeout
    for (auto i = m_registeredActionTimeouts.constBegin(), end = m_registeredActionTimeouts.constEnd(); i != end; ++i) {
        if (i.value().contains(identifier)) {
            i.key()->onIdleTimeout(std::chrono::milliseconds(msec));

            // And it will need to be awaken
            m_pendingResumeFromIdleActions.insert(i.key());
            break;
        }
    }

    // Catch the next resume event if some actions require it
    if (!m_pendingResumeFromIdleActions.isEmpty()) {
        KIdleTime::instance()->catchNextResumeEvent();
    }
}

void Core::onLidClosedChanged(bool closed)
{
    Q_EMIT lidClosedChanged(closed);
}

void Core::onAboutToSuspend()
{
    if (m_globalSettings->pausePlayersOnSuspend()) {
        qCDebug(POWERDEVIL) << "Pausing all media players before sleep";

        QDBusPendingCall listNamesCall = QDBusConnection::sessionBus().interface()->asyncCall(QStringLiteral("ListNames"));
        QDBusPendingCallWatcher *callWatcher = new QDBusPendingCallWatcher(listNamesCall, this);
        connect(callWatcher, &QDBusPendingCallWatcher::finished, this, [](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QStringList> reply = *watcher;
            watcher->deleteLater();

            if (reply.isError()) {
                qCWarning(POWERDEVIL) << "Failed to fetch list of DBus service names for pausing players on entering sleep" << reply.error().message();
                return;
            }

            const QStringList &services = reply.value();
            for (const QString &serviceName : services) {
                if (!serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2."))) {
                    continue;
                }

                if (serviceName.startsWith(QLatin1String("org.mpris.MediaPlayer2.kdeconnect.mpris_"))) {
                    // This is actually a player on another device exposed by KDE Connect
                    // We don't want to pause it
                    // See https://bugs.kde.org/show_bug.cgi?id=427209
                    continue;
                }

                qCDebug(POWERDEVIL) << "Pausing media player with service name" << serviceName;

                QDBusMessage pauseMsg = QDBusMessage::createMethodCall(serviceName,
                                                                       QStringLiteral("/org/mpris/MediaPlayer2"),
                                                                       QStringLiteral("org.mpris.MediaPlayer2.Player"),
                                                                       QStringLiteral("Pause"));
                QDBusConnection::sessionBus().asyncCall(pauseMsg);
            }
        });
    }
}

void Core::registerActionTimeout(Action *action, std::chrono::milliseconds timeout)
{
    // Register the timeout with KIdleTime
    int identifier = KIdleTime::instance()->addIdleTimeout(timeout);

    // Add the identifier to the action hash
    QList<int> timeouts = m_registeredActionTimeouts[action];
    timeouts.append(identifier);
    m_registeredActionTimeouts[action] = timeouts;
}

void Core::unregisterActionTimeouts(Action *action)
{
    // Clear all timeouts from the action
    const QList<int> timeoutsToClean = m_registeredActionTimeouts[action];

    for (int id : timeoutsToClean) {
        KIdleTime::instance()->removeIdleTimeout(id);
    }

    m_registeredActionTimeouts.remove(action);
}

int Core::currentChargePercent() const
{
    int chargePercent = 0;
    for (auto it = m_batteriesPercent.constBegin(); it != m_batteriesPercent.constEnd(); ++it) {
        chargePercent += it.value();
    }
    return chargePercent;
}

void Core::onResumingFromIdle()
{
    KIdleTime::instance()->simulateUserActivity();
    // Wake up the actions in which an idle action was triggered
    std::for_each(m_pendingResumeFromIdleActions.cbegin(), m_pendingResumeFromIdleActions.cend(), std::mem_fn(&PowerDevil::Action::onWakeupFromIdle));

    m_pendingResumeFromIdleActions.clear();
}

void Core::onNotificationTimeout()
{
    // cannot connect QTimer::singleShot directly to the other method
    onServiceRegistered(QString());
}

void Core::onServiceRegistered(const QString &service)
{
    Q_UNUSED(service);

    if (m_notificationsReady) {
        return;
    }

    bool needsRefresh = false;

    // show warning about low batteries right on session startup, force it to show
    // by making sure the "old" percentage (that magic number) is always higher than the current one
    if (emitBatteryChargePercentNotification(currentChargePercent(), 1000)) {
        needsRefresh = true;
    }

    // now also emit notifications for all peripheral batteries
    for (auto it = m_peripheralBatteriesPercent.constBegin(), end = m_peripheralBatteriesPercent.constEnd(); it != end; ++it) {
        if (emitBatteryChargePercentNotification(it.value() /*currentPercent*/, 1000, it.key() /*udi*/)) {
            needsRefresh = true;
        }
    }

    // need to refresh status to prevent the notification from showing again when charge percentage changes
    if (needsRefresh) {
        refreshStatus();
    }

    m_notificationsReady = true;

    if (m_notificationsWatcher) {
        delete m_notificationsWatcher;
        m_notificationsWatcher = nullptr;
    }
}

void Core::readChargeThreshold()
{
    KAuth::Action action(QStringLiteral("org.kde.powerdevil.chargethresholdhelper.getthreshold"));
    action.setHelperId(QStringLiteral("org.kde.powerdevil.chargethresholdhelper"));
    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCWarning(POWERDEVIL) << "org.kde.powerdevil.chargethresholdhelper.getthreshold failed" << job->errorText();
            return;
        }

        const auto data = job->data();

        const int chargeStartThreshold = data.value(QStringLiteral("chargeStartThreshold")).toInt();
        if (chargeStartThreshold != -1 && m_chargeStartThreshold != chargeStartThreshold) {
            m_chargeStartThreshold = chargeStartThreshold;
            Q_EMIT chargeStartThresholdChanged(chargeStartThreshold);
        }

        const int chargeStopThreshold = data.value(QStringLiteral("chargeStopThreshold")).toInt();
        if (chargeStopThreshold != -1 && m_chargeStopThreshold != chargeStopThreshold) {
            m_chargeStopThreshold = chargeStopThreshold;
            Q_EMIT chargeStopThresholdChanged(chargeStopThreshold);
        }

        qCDebug(POWERDEVIL) << "Charge thresholds: start at" << chargeStartThreshold << "- stop at" << chargeStopThreshold;
    });
    job->start();
}

void Core::readBatteryConservationMode()
{
    KAuth::Action action(QStringLiteral("org.kde.powerdevil.chargethresholdhelper.getconservationmode"));
    action.setHelperId(QStringLiteral("org.kde.powerdevil.chargethresholdhelper"));
    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this, job] {
        if (job->error()) {
            qCWarning(POWERDEVIL) << "org.kde.powerdevil.chargethresholdhelper.getconservationmode failed" << job->errorText();
            return;
        }

        const auto data = job->data();
        const bool enabled = data.value(QStringLiteral("batteryConservationModeEnabled")).toBool();
        if (m_batteryConservationModeEnabled != enabled) {
            m_batteryConservationModeEnabled = enabled;
            Q_EMIT batteryConservationModeChanged(enabled);
        }

        qCDebug(POWERDEVIL) << "Battery conservation mode is" << (enabled ? "enabled" : "disabled");
    });
    job->start();
}

SuspendController *Core::suspendController()
{
    return m_suspendController.get();
}

BatteryController *Core::batteryController()
{
    return m_batteryController.get();
}

Action *Core::action(const QString actionId)
{
    const auto it = m_actionPool.find(actionId);
    return it == m_actionPool.end() ? nullptr : it->second.get();
}

void Core::unloadAllActiveActions()
{
    for (const QString &action : std::as_const(m_activeActions)) {
        m_actionPool[action]->onProfileUnload();
        m_actionPool[action]->unloadAction();
    }
    m_activeActions.clear();
}

LidController *Core::lidController()
{
    return m_lidController.get();
}

bool Core::isLidClosed() const
{
    return m_lidController->isLidClosed();
}

bool Core::isLidPresent() const
{
    return m_lidController->isLidPresent();
}

bool Core::hasDualGpu() const
{
    return m_hasDualGpu;
}

bool Core::isBatteryConservationModeEnabled() const
{
    return m_batteryConservationModeEnabled;
}

int Core::chargeStartThreshold() const
{
    return m_chargeStartThreshold;
}

int Core::chargeStopThreshold() const
{
    return m_chargeStopThreshold;
}

uint Core::scheduleWakeup(const QString &service, const QDBusObjectPath &path, qint64 timeout)
{
    ++m_lastWakeupCookie;

    int cookie = m_lastWakeupCookie;
    // if some one is trying to time travel, deny them
    if (timeout < QDateTime::currentSecsSinceEpoch()) {
        sendErrorReply(QDBusError::InvalidArgs, u"You can not schedule wakeup in past"_s);
    } else {
#ifndef Q_OS_LINUX
        sendErrorReply(QDBusError::NotSupported, u"Scheduled wakeups are available only on Linux platforms"_s);
#else
        WakeupInfo wakeup{service, path, cookie, timeout};
        m_scheduledWakeups << wakeup;
        qCDebug(POWERDEVIL) << "Received request to wakeup at " << QDateTime::fromSecsSinceEpoch(timeout);
        resetAndScheduleNextWakeup();
#endif
    }
    return cookie;
}

void Core::wakeup()
{
    onResumingFromIdle();
    PowerDevil::Action *helperAction = action(QStringLiteral("DPMSControl"));
    if (helperAction) {
        QVariantMap args;
        // we pass empty string as type because when empty type is passed,
        // it turns screen on.
        args[u"Type"_s] = QString();
        helperAction->trigger(args);
    }
}

void Core::clearWakeup(int cookie)
{
    // if we do not have any timeouts return from here
    if (m_scheduledWakeups.isEmpty()) {
        return;
    }

    int oldListSize = m_scheduledWakeups.size();

    // depending on cookie, remove it from scheduled wakeups
    m_scheduledWakeups.erase(std::remove_if(m_scheduledWakeups.begin(),
                                            m_scheduledWakeups.end(),
                                            [cookie](WakeupInfo wakeup) {
                                                return wakeup.cookie == cookie;
                                            }),
                             m_scheduledWakeups.end());

    if (oldListSize == m_scheduledWakeups.size()) {
        sendErrorReply(QDBusError::InvalidArgs, u"Can not clear the invalid wakeup"_s);
        return;
    }

    // reset timerfd
    resetAndScheduleNextWakeup();
}

qulonglong Core::batteryRemainingTime() const
{
    return m_batteryController->batteryRemainingTime();
}

qulonglong Core::smoothedBatteryRemainingTime() const
{
    return m_batteryController->smoothedBatteryRemainingTime();
}

uint Core::backendCapabilities()
{
    return 1; // SignalResumeFromSuspend;
}

void Core::resetAndScheduleNextWakeup()
{
#ifdef Q_OS_LINUX
    // first we sort the wakeup list
    std::sort(m_scheduledWakeups.begin(), m_scheduledWakeups.end(), [](const WakeupInfo &lhs, const WakeupInfo &rhs) {
        return lhs.timeout < rhs.timeout;
    });

    // we don't want any of our wakeups to repeat'
    timespec interval = {0, 0};
    timespec nextWakeup;
    bool enableNotifier = false;
    // if we don't have any wakeups left, we call it a day and stop timer_fd
    if (m_scheduledWakeups.isEmpty()) {
        nextWakeup = {0, 0};
    } else {
        // now pick the first timeout from the list
        WakeupInfo wakeup = m_scheduledWakeups.first();
        nextWakeup = {wakeup.timeout, 0};
        enableNotifier = true;
    }
    if (m_timerFd != -1) {
        const itimerspec spec = {interval, nextWakeup};
        timerfd_settime(m_timerFd, TFD_TIMER_ABSTIME, &spec, nullptr);
    }
    m_timerFdSocketNotifier->setEnabled(enableNotifier);
#endif
}

void Core::timerfdEventHandler()
{
    // wakeup from the linux/rtc

    // Disable reading events from the timer_fd
    m_timerFdSocketNotifier->setEnabled(false);

    // At this point scheduled wakeup list should not be empty, but just in case
    if (m_scheduledWakeups.isEmpty()) {
        qWarning(POWERDEVIL) << "Wakeup was recieved but list is now empty! This should not happen!";
        return;
    }

    // first thing to do is, we pick the first wakeup from list
    WakeupInfo currentWakeup = m_scheduledWakeups.takeFirst();

    // Before doing anything further, lets set the next set of wakeup alarm
    resetAndScheduleNextWakeup();

    // Now current wakeup needs to be processed
    // prepare message for sending back to the consumer
    QDBusMessage msg = QDBusMessage::createMethodCall(currentWakeup.service,
                                                      currentWakeup.path.path(),
                                                      QStringLiteral("org.kde.PowerManagement"),
                                                      QStringLiteral("wakeupCallback"));
    msg << currentWakeup.cookie;
    // send it away
    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

KeyboardBrightnessController *Core::keyboardBrightnessController()
{
    return m_keyboardBrightnessController.get();
}

ScreenBrightnessController *Core::screenBrightnessController()
{
    return m_screenBrightnessController.get();
}
}

#include "moc_powerdevilcore.cpp"
