/*
 *   SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic@kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

// Self
#include "Activities.h"
#include "Activities_p.h"
#include <kactivities-features.h>

// Qt
#include <QDBusConnection>
#include <QFile>
#include <QMetaObject>
#include <QStandardPaths>
#include <QUuid>

// KDE
#include <kauthorized.h>
#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Utils
#include <utils/d_ptr_implementation.h>
#include <utils/range.h>

// Local
#include "DebugActivities.h"
#include "activitiesadaptor.h"
#include "common/dbus/common.h"
#include "ksmserver/KSMServer.h"

// Private
#define ACTIVITY_MANAGER_CONFIG_FILE_NAME QStringLiteral("kactivitymanagerdrc")

namespace
{
inline bool nameBasedOrdering(const ActivityInfo &info, const ActivityInfo &other)
{
    const auto comp = QString::compare(info.name, other.name, Qt::CaseInsensitive);
    return comp < 0 || (comp == 0 && info.id < other.id);
}
}

Activities::Private::Private(Activities *parent)
    : config(QStringLiteral("kactivitymanagerdrc"))
    , q(parent)
{
    // qCDebug(KAMD_ACTIVITIES) << "Using this configuration file:"
    //     << config.name()
    //     << config.locationType()
    //     << QStandardPaths::standardLocations(config.locationType())
    //     ;

    // Reading activities from the config file.
    // Saving only the running activities means that if we have any
    // errors in the config, we might end up with all activities
    // stopped

    const auto defaultState = !mainConfig().hasKey("runningActivities") ? Activities::Running
        : !mainConfig().hasKey("stoppedActivities")                     ? Activities::Stopped
                                                                        : Activities::Running;

    const auto runningActivities = mainConfig().readEntry("runningActivities", QStringList());
    const auto stoppedActivities = mainConfig().readEntry("stoppedActivities", QStringList());

    // Do we have a running activity?
    bool atLeastOneRunning = false;

    for (const auto &activity : activityNameConfig().keyList()) {
        auto state = runningActivities.contains(activity) ? Activities::Running : stoppedActivities.contains(activity) ? Activities::Stopped : defaultState;

        activities[activity] = state;

        if (state == Activities::Running) {
            atLeastOneRunning = true;
        }
    }

    // Is this our first start?
    if (activities.isEmpty()) {
        // We need to add this only after the service has been properly started
        KConfigGroup cg(KSharedConfig::openConfig(QStringLiteral("kdeglobals")), QStringLiteral("Activities"));
        // NOTE: config key still singular for retrocompatibility
        const QStringList names = cg.readEntry("defaultActivityName", QStringList{i18n("Default")});

        for (const auto &name : names) {
            QMetaObject::invokeMethod(q, "AddActivity", Qt::QueuedConnection, Q_ARG(QString, name));
        }

    } else if (!atLeastOneRunning) {
        // If we have no running activities, but we have activities,
        // we are in a problem. This should not happen unless the
        // configuration file is in a big problem and told us there
        // are no running activities, and enlists all of them as stopped.
        // In that case, we will pretend all of them are running
        qCWarning(KAMD_LOG_ACTIVITIES) << "The config file enlisted all activities as stopped";
        for (auto activityKey = activities.keyBegin(); activityKey != activities.keyEnd(); ++activityKey) {
            activities[*activityKey] = Activities::Running;
        }
    }
}

void Activities::Private::updateSortedActivityList()
{
    QList<ActivityInfo> a;
    for (auto activityKey = activities.keyBegin(); activityKey != activities.keyEnd(); ++activityKey) {
        a.append(q->ActivityInformation(*activityKey));
    }

    std::sort(a.begin(), a.end(), &nameBasedOrdering);

    QWriteLocker lock(&activitiesLock);
    sortedActivities = a;
}

void Activities::Private::loadLastActivity()
{
    // This is called from constructor, no need for locking

    // If there are no public activities, try to load the last used activity
    const auto lastUsedActivity = mainConfig().readEntry("currentActivity", QString());

    setCurrentActivity((lastUsedActivity.isEmpty() && activities.size() > 0) ? activities.keys().at(0) : lastUsedActivity);
}

Activities::Private::~Private()
{
    configSync();
}

bool Activities::Private::setCurrentActivity(const QString &activity)
{
    {
        // There is nothing expensive in this block, not a problem to lock
        QWriteLocker lock(&activitiesLock);

        // Should we change the activity at all?
        if (currentActivity == activity) {
            return true;
        }

        // If the activity is empty, this means we are entering a limbo state
        if (activity.isEmpty()) {
            currentActivity.clear();
            Q_EMIT q->CurrentActivityChanged(currentActivity);
            return true;
        }

        // Does the requested activity exist?
        if (!activities.contains(activity)) {
            return false;
        }
    }

    // Start activity
    q->StartActivity(activity);

    // Saving the current activity, and notifying
    // clients of the change
    currentActivity = activity;

    mainConfig().writeEntry("currentActivity", activity);

    scheduleConfigSync();

    Q_EMIT q->CurrentActivityChanged(activity);

    return true;
}

bool Activities::Private::previousActivity()
{
    const auto a = q->ListActivities(Activities::Running);

    for (int i = 0; i < a.count(); ++i) {
        if (a[i] == currentActivity) {
            return setCurrentActivity(a[(i + a.size() - 1) % a.size()]);
        }
    }

    return false;
}

bool Activities::Private::nextActivity()
{
    const auto a = q->ListActivities(Activities::Running);

    for (int i = 0; i < a.count(); ++i) {
        if (a[i] == currentActivity) {
            return setCurrentActivity(a[(i + 1) % a.size()]);
        }
    }

    return false;
}

QString Activities::Private::addActivity(const QString &name)
{
    QString activity;

    if (name.isEmpty()) {
        Q_ASSERT(!name.isEmpty());
        return activity;
    }

    int activitiesCount = 0;

    {
        QWriteLocker lock(&activitiesLock);

        // Ensuring a new Uuid. The loop should usually end after only
        // one iteration
        while (activity.isEmpty() || activities.contains(activity)) {
            activity = QUuid::createUuid().toString().mid(1, 36);
        }

        // Saves the activity info to the config

        activities[activity] = Invalid;
        activitiesCount = activities.size();
    }

    setActivityState(activity, Running);

    q->SetActivityName(activity, name);

    updateSortedActivityList();

    Q_EMIT q->ActivityAdded(activity);

    scheduleConfigSync();

    if (activitiesCount == 1) {
        q->SetCurrentActivity(activity);
    }

    return activity;
}

void Activities::Private::removeActivity(const QString &activity)
{
    Q_ASSERT(!activity.isEmpty());

    // Sanity checks
    {
        QWriteLocker lock(&activitiesLock);

        if (!activities.contains(activity)) {
            return;
        }

        // Is somebody trying to remove the last activity?
        if (activities.size() == 1) {
            return;
        }
    }

    // If the activity is running, stash it
    q->StopActivity(activity);

    setActivityState(activity, Activities::Invalid);

    bool currentActivityDeleted = false;

    {
        QWriteLocker lock(&activitiesLock);
        // Removing the activity
        activities.remove(activity);

        for (int i = 0; i < sortedActivities.count(); ++i) {
            if (sortedActivities[i].id == activity) {
                sortedActivities.remove(i);
                break;
            }
        }

        // If the removed activity was the current one,
        // set another activity as current
        currentActivityDeleted = (currentActivity == activity);
    }

    activityNameConfig().deleteEntry(activity);
    activityDescriptionConfig().deleteEntry(activity);
    activityIconConfig().deleteEntry(activity);

    if (currentActivityDeleted) {
        ensureCurrentActivityIsRunning();
    }

    Q_EMIT q->ActivityRemoved(activity);

    QMetaObject::invokeMethod(q, "ActivityRemoved", Qt::QueuedConnection, Q_ARG(QString, activity));

    QMetaObject::invokeMethod(this, "configSync", Qt::QueuedConnection);
}

void Activities::Private::scheduleConfigSync()
{
    static const auto shortInterval = 1000;

    // If the timer is not running, or has a longer interval than we need,
    // start it
    // Note: If you want to add multiple different delays for different
    // events based on the importance of how quickly something needs
    // to be synced to the config, don't. Since the QTimer lives in a
    // separate thread, we have to communicate with it in via
    // queued connections, which means that we don't know whether
    // the timer was already started when this method was invoked,
    // we do not know whether the interval is properly set etc.
    if (!configSyncTimer.isActive()) {
        QMetaObject::invokeMethod(&configSyncTimer, "start", Qt::QueuedConnection, Q_ARG(int, shortInterval));
    }
}

void Activities::Private::configSync()
{
    // Stop the timer and reset the interval to zero
    QMetaObject::invokeMethod(&configSyncTimer, "stop", Qt::QueuedConnection);
    config.sync();
}

void Activities::Private::setActivityState(const QString &activity, Activities::State state)
{
    bool configNeedsUpdating = false;

    {
        QWriteLocker lock(&activitiesLock);

        Q_ASSERT(activities.contains(activity));

        if (activities.value(activity) == state) {
            return;
        }

        // Treating 'Starting' as 'Running', and 'Stopping' as 'Stopped'
        // as far as the config file is concerned
        configNeedsUpdating = ((activities[activity] & 4) != (state & 4));

        activities[activity] = state;
    }

    switch (state) {
    case Activities::Running:
        Q_EMIT q->ActivityStarted(activity);
        break;

    case Activities::Stopped:
        Q_EMIT q->ActivityStopped(activity);
        break;

    default:
        break;
    }

    Q_EMIT q->ActivityStateChanged(activity, state);

    if (configNeedsUpdating) {
        QReadLocker lock(&activitiesLock);

        mainConfig().writeEntry("runningActivities", activities.keys(Activities::Running) + activities.keys(Activities::Starting));
        mainConfig().writeEntry("stoppedActivities", activities.keys(Activities::Stopped) + activities.keys(Activities::Stopping));
        scheduleConfigSync();
    }

    updateSortedActivityList();
}

void Activities::Private::ensureCurrentActivityIsRunning()
{
    // If the current activity is not running,
    // make some other activity current

    const auto runningActivities = q->ListActivities(Activities::Running);

    if (!runningActivities.contains(currentActivity) && runningActivities.size() > 0) {
        setCurrentActivity(runningActivities.first());
    }
}

void Activities::Private::activitySessionStateChanged(const QString &activity, int status)
{
    QString currentActivity = this->currentActivity;

    {
        QReadLocker lock(&activitiesLock);
        if (!activities.contains(activity)) {
            return;
        }
    }

    switch (status) {
    case KSMServer::Started:
    case KSMServer::FailedToStop:
        setActivityState(activity, Activities::Running);
        break;

    case KSMServer::Stopped:
        setActivityState(activity, Activities::Stopped);

        if (currentActivity == activity) {
            ensureCurrentActivityIsRunning();
        }

        break;
    }

    QMetaObject::invokeMethod(this, "configSync", Qt::QueuedConnection);
}

// Main

Activities::Activities(QObject *parent)
    : Module(QStringLiteral("activities"), parent)
    , d(this)
{
    qCDebug(KAMD_LOG_ACTIVITIES) << "Starting the KDE Activity Manager daemon" << QDateTime::currentDateTime();

    // Basic initialization ////////////////////////////////////////////////////

    // Initializing D-Bus service

    new ActivitiesAdaptor(this);
    QDBusConnection::sessionBus().registerObject(KAMD_DBUS_OBJECT_PATH("Activities"), this);

    // Initializing config

    qCDebug(KAMD_LOG_ACTIVITIES) << "Config timer connecting...";
    d->connect(&d->configSyncTimer, SIGNAL(timeout()), SLOT(configSync()), Qt::QueuedConnection);

    d->configSyncTimer.setSingleShot(true);

    d->ksmserver = new KSMServer(this);
    d->connect(d->ksmserver, SIGNAL(activitySessionStateChanged(QString, int)), SLOT(activitySessionStateChanged(QString, int)));

    d->updateSortedActivityList();
    // Loading the last used activity, if possible
    d->loadLastActivity();
}

Activities::~Activities()
{
}

QString Activities::CurrentActivity() const
{
    QReadLocker lock(&d->activitiesLock);
    return d->currentActivity;
}

bool Activities::SetCurrentActivity(const QString &activity)
{
    // Public method can not put us in a limbo state
    if (activity.isEmpty()) {
        return false;
    }

    return d->setCurrentActivity(activity);
}

bool Activities::PreviousActivity()
{
    return d->previousActivity();
}

bool Activities::NextActivity()
{
    return d->nextActivity();
}

QString Activities::AddActivity(const QString &name)
{
    // We do not care about authorization if this is the first start
    if (!d->activities.isEmpty() && !KAuthorized::authorize(QStringLiteral("plasma-desktop/add_activities"))) {
        return QString();
    }

    return d->addActivity(name);
}

void Activities::RemoveActivity(const QString &activity)
{
    if (!KAuthorized::authorize(QStringLiteral("plasma-desktop/add_activities"))) {
        return;
    }

    d->removeActivity(activity);
}

QStringList Activities::ListActivities() const
{
    QReadLocker lock(&d->activitiesLock);

    QStringList s;
    for (const auto &a : d->sortedActivities) {
        s << a.id;
    }
    return s;
}

QStringList Activities::ListActivities(int state) const
{
    QReadLocker lock(&d->activitiesLock);

    QStringList s;
    for (const auto &a : d->sortedActivities) {
        if (a.state == (State)state) {
            s << a.id;
        }
    }
    return s;
}

QList<ActivityInfo> Activities::ListActivitiesWithInformation() const
{
    using namespace kamd::utils;

    // Mapping activity ids to info

    return as_collection<QList<ActivityInfo>>(ListActivities() | transformed(&Activities::ActivityInformation, this));
}

ActivityInfo Activities::ActivityInformation(const QString &activity) const
{
    return ActivityInfo{activity, ActivityName(activity), ActivityDescription(activity), ActivityIcon(activity), ActivityState(activity)};
}

#define CREATE_GETTER_AND_SETTER(What)                                                                                                                         \
    QString Activities::Activity##What(const QString &activity) const                                                                                          \
    {                                                                                                                                                          \
        QReadLocker lock(&d->activitiesLock);                                                                                                                  \
        return d->activities.contains(activity) ? d->activity##What(activity) : QString();                                                                     \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void Activities::SetActivity##What(const QString &activity, const QString &value)                                                                          \
    {                                                                                                                                                          \
        {                                                                                                                                                      \
            QReadLocker lock(&d->activitiesLock);                                                                                                              \
            if (value == d->activity##What(activity) || !d->activities.contains(activity)) {                                                                   \
                return;                                                                                                                                        \
            }                                                                                                                                                  \
        }                                                                                                                                                      \
                                                                                                                                                               \
        d->activity##What##Config().writeEntry(activity, value);                                                                                               \
        d->scheduleConfigSync();                                                                                                                               \
                                                                                                                                                               \
        Q_EMIT Activity##What##Changed(activity, value);                                                                                                         \
        Q_EMIT ActivityChanged(activity);                                                                                                                        \
    }

CREATE_GETTER_AND_SETTER(Name)
CREATE_GETTER_AND_SETTER(Description)
CREATE_GETTER_AND_SETTER(Icon)

#undef CREATE_GETTER_AND_SETTER

// Main

void Activities::StartActivity(const QString &activity)
{
    {
        QReadLocker lock(&d->activitiesLock);
        if (!d->activities.contains(activity) || d->activities[activity] != Stopped) {
            return;
        }
    }

    d->setActivityState(activity, Starting);
    d->ksmserver->startActivitySession(activity);
}

void Activities::StopActivity(const QString &activity)
{
    {
        QReadLocker lock(&d->activitiesLock);
        if (!d->activities.contains(activity) //
            || d->activities[activity] == Stopped //
            || d->activities.size() == 1 //
            || d->activities.keys(Activities::Running).size() <= 1) {
            return;
        }
    }

    d->setActivityState(activity, Stopping);
    d->ksmserver->stopActivitySession(activity);
}

int Activities::ActivityState(const QString &activity) const
{
    QReadLocker lock(&d->activitiesLock);
    return d->activities.contains(activity) ? d->activities[activity] : Invalid;
}

#include "moc_Activities.cpp"

#include "moc_Activities_p.cpp"
