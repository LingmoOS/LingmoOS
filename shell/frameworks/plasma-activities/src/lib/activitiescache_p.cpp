/*
    SPDX-FileCopyrightText: 2013-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "activitiescache_p.h"
#include "manager_p.h"

#include <mutex>

#include <QString>

#include "mainthreadexecutor_p.h"

namespace KActivities
{
static QString nulluuid = QStringLiteral("00000000-0000-0000-0000-000000000000");

using kamd::utils::Mutable;

std::shared_ptr<ActivitiesCache> ActivitiesCache::self()
{
    static std::weak_ptr<ActivitiesCache> s_instance;
    static std::mutex singleton;
    std::lock_guard<std::mutex> singleton_lock(singleton);

    auto result = s_instance.lock();

    if (s_instance.expired()) {
        runInMainThread([&result] {
            result.reset(new ActivitiesCache());
            s_instance = result;
        });
    }

    return result;
}

ActivitiesCache::ActivitiesCache()
    : m_status(Consumer::NotRunning)
{
    // qDebug() << "ActivitiesCache: Creating a new instance";
    using org::kde::ActivityManager::Activities;

    auto activities = Manager::self()->activities();

    connect(activities, &Activities::ActivityAdded, this, &ActivitiesCache::updateActivity);
    connect(activities, &Activities::ActivityChanged, this, &ActivitiesCache::updateActivity);
    connect(activities, &Activities::ActivityRemoved, this, &ActivitiesCache::removeActivity);

    connect(activities, &Activities::ActivityStateChanged, this, &ActivitiesCache::updateActivityState);
    connect(activities, &Activities::ActivityNameChanged, this, &ActivitiesCache::setActivityName);
    connect(activities, &Activities::ActivityDescriptionChanged, this, &ActivitiesCache::setActivityDescription);
    connect(activities, &Activities::ActivityIconChanged, this, &ActivitiesCache::setActivityIcon);

    connect(activities, &Activities::CurrentActivityChanged, this, &ActivitiesCache::setCurrentActivity);

    connect(Manager::self(), &Manager::serviceStatusChanged, this, &ActivitiesCache::setServiceStatus);

    // These are covered by ActivityStateChanged
    // signal void org.kde.ActivityManager.Activities.ActivityStarted(QString activity)
    // signal void org.kde.ActivityManager.Activities.ActivityStopped(QString activity)

    setServiceStatus(Manager::self()->isServiceRunning());
}

void ActivitiesCache::setServiceStatus(bool status)
{
    // qDebug() << "Setting service status to:" << status;
    loadOfflineDefaults();

    if (status) {
        updateAllActivities();
    }
}

void ActivitiesCache::loadOfflineDefaults()
{
    m_status = Consumer::NotRunning;

    m_activities.clear();
    m_activities << ActivityInfo(nulluuid, QString(), QString(), QString(), Info::Running);
    m_currentActivity = nulluuid;

    Q_EMIT serviceStatusChanged(m_status);
    Q_EMIT activityListChanged();
}

ActivitiesCache::~ActivitiesCache()
{
    // qDebug() << "ActivitiesCache: Destroying the instance";
}

void ActivitiesCache::removeActivity(const QString &id)
{
    // qDebug() << "Removing the activity";

    // Since we are sorting the activities by name now,
    // we can not use lower_bound to search for an activity
    // with a specified id
    const auto where = find(id);

    if (where != m_activities.end() && where->id == id) {
        m_activities.erase(where);
        Q_EMIT activityRemoved(id);
        Q_EMIT activityListChanged();

    } else {
        // qFatal("Requested to delete an non-existent activity");
    }
}

void ActivitiesCache::updateAllActivities()
{
    // qDebug() << "Updating all";
    m_status = Consumer::Unknown;
    Q_EMIT serviceStatusChanged(m_status);

    // Loading the current activity
    auto call = Manager::self()->activities()->asyncCall(QStringLiteral("CurrentActivity"));

    onCallFinished(call, SLOT(setCurrentActivityFromReply(QDBusPendingCallWatcher *)));

    // Loading all the activities
    call = Manager::self()->activities()->asyncCall(QStringLiteral("ListActivitiesWithInformation"));

    onCallFinished(call, SLOT(setAllActivitiesFromReply(QDBusPendingCallWatcher *)));
}

void ActivitiesCache::updateActivity(const QString &id)
{
    // qDebug() << "Updating activity" << id;

    auto call = Manager::self()->activities()->asyncCall(QStringLiteral("ActivityInformation"), id);

    onCallFinished(call, SLOT(setActivityInfoFromReply(QDBusPendingCallWatcher *)));
}

void ActivitiesCache::updateActivityState(const QString &id, int state)
{
    auto where = getInfo<Mutable>(id);

    if (where && where->state != state) {
        auto isInvalid = [](int state) {
            return state == Info::Invalid || state == Info::Unknown;
        };
        auto isStopped = [](int state) {
            return state == Info::Stopped || state == Info::Starting;
        };
        auto isRunning = [](int state) {
            return state == Info::Running || state == Info::Stopping;
        };

        const bool runningStateChanged =
            (isInvalid(state) || isInvalid(where->state) || (isStopped(state) && isRunning(where->state)) || (isRunning(state) && isStopped(where->state)));

        where->state = state;

        if (runningStateChanged) {
            Q_EMIT runningActivityListChanged();
        }

        Q_EMIT activityStateChanged(id, state);

    } else {
        // qFatal("Requested to update the state of a non-existent activity");
    }
}

template<typename _Result, typename _Functor>
void ActivitiesCache::passInfoFromReply(QDBusPendingCallWatcher *watcher, _Functor f)
{
    QDBusPendingReply<_Result> reply = *watcher;

    if (!reply.isError()) {
        auto replyValue = reply.template argumentAt<0>();
        // qDebug() << "Got some reply" << replyValue;

        ((*this).*f)(replyValue);
    }

    watcher->deleteLater();
}

void ActivitiesCache::setActivityInfoFromReply(QDBusPendingCallWatcher *watcher)
{
    // qDebug() << "reply...";
    passInfoFromReply<ActivityInfo>(watcher, &ActivitiesCache::setActivityInfo);
}

void ActivitiesCache::setAllActivitiesFromReply(QDBusPendingCallWatcher *watcher)
{
    // qDebug() << "reply...";
    passInfoFromReply<ActivityInfoList>(watcher, &ActivitiesCache::setAllActivities);
}

void ActivitiesCache::setCurrentActivityFromReply(QDBusPendingCallWatcher *watcher)
{
    // qDebug() << "reply...";
    passInfoFromReply<QString>(watcher, &ActivitiesCache::setCurrentActivity);
}

void ActivitiesCache::setActivityInfo(const ActivityInfo &info)
{
    // qDebug() << "Setting activity info" << info.id;

    // Are we updating an existing activity, or adding a new one?
    const auto iter = find(info.id);
    const auto present = iter != m_activities.end();
    bool runningChanged = true;
    // If there is an activity with the specified id,
    // we are going to remove it, temporarily.
    if (present) {
        runningChanged = (*iter).state != info.state;
        m_activities.erase(iter);
    }

    // Now, we need to find where to insert the activity
    // and keep the cache sorted by name
    const auto where = lower_bound(info);

    m_activities.insert(where, info);

    if (present) {
        Q_EMIT activityChanged(info.id);
    } else {
        Q_EMIT activityAdded(info.id);
        Q_EMIT activityListChanged();
        if (runningChanged) {
            Q_EMIT runningActivityListChanged();
        }
    }
}
// clang-format off
#define CREATE_SETTER(WHAT, What)                                              \
    void ActivitiesCache::setActivity##WHAT(const QString &id,                 \
                                            const QString &value)              \
    {                                                                          \
        auto where = getInfo<Mutable>(id);                                     \
                                                                               \
        if (where) {                                                           \
            where->What = value;                                               \
            Q_EMIT activity##WHAT##Changed(id, value);                           \
        }                                                                      \
    }
// clang-format on

CREATE_SETTER(Name, name)
CREATE_SETTER(Description, description)
CREATE_SETTER(Icon, icon)

#undef CREATE_SETTER

void ActivitiesCache::setAllActivities(const ActivityInfoList &_activities)
{
    // qDebug() << "Setting all activities";

    m_activities.clear();

    const ActivityInfoList activities = _activities;

    for (const ActivityInfo &info : activities) {
        m_activities << info;
    }

    std::sort(m_activities.begin(), m_activities.end(), &infoLessThan);

    m_status = Consumer::Running;
    Q_EMIT serviceStatusChanged(m_status);
    Q_EMIT activityListChanged();
}

void ActivitiesCache::setCurrentActivity(const QString &activity)
{
    // qDebug() << "Setting current activity to" << activity;

    if (m_currentActivity == activity) {
        return;
    }

    m_currentActivity = activity;

    Q_EMIT currentActivityChanged(activity);
}

} // namespace KActivities

#include "moc_activitiescache_p.cpp"
