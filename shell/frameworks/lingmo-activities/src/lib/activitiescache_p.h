/*
    SPDX-FileCopyrightText: 2013-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_CACHE_P_H
#define ACTIVITIES_CACHE_P_H

#include <memory>

#include <QObject>

#include <common/dbus/org.kde.ActivityManager.Activities.h>
#include <utils/ptr_to.h>

#include "activities_interface.h"
#include "consumer.h"

namespace KActivities
{
class ActivitiesCache : public QObject
{
    Q_OBJECT

public:
    static std::shared_ptr<ActivitiesCache> self();

    ~ActivitiesCache() override;

Q_SIGNALS:
    void activityAdded(const QString &id);
    void activityChanged(const QString &id);
    void activityRemoved(const QString &id);

    void activityStateChanged(const QString &id, int state);
    void activityNameChanged(const QString &id, const QString &name);
    void activityDescriptionChanged(const QString &id, const QString &description);
    void activityIconChanged(const QString &id, const QString &icon);

    void currentActivityChanged(const QString &id);
    void serviceStatusChanged(Consumer::ServiceStatus status);
    void activityListChanged();
    void runningActivityListChanged();

private Q_SLOTS:
    void updateAllActivities();
    void loadOfflineDefaults();

    void updateActivity(const QString &id);
    void updateActivityState(const QString &id, int state);
    void removeActivity(const QString &id);

    void setActivityInfoFromReply(QDBusPendingCallWatcher *watcher);
    void setAllActivitiesFromReply(QDBusPendingCallWatcher *watcher);
    void setCurrentActivityFromReply(QDBusPendingCallWatcher *watcher);

    void setActivityName(const QString &id, const QString &name);
    void setActivityDescription(const QString &id, const QString &description);
    void setActivityIcon(const QString &id, const QString &icon);

    void setActivityInfo(const ActivityInfo &info);
    void setAllActivities(const ActivityInfoList &activities);
    void setCurrentActivity(const QString &activity);

    void setServiceStatus(bool status);

public:
    template<typename _Result, typename _Functor>
    void passInfoFromReply(QDBusPendingCallWatcher *watcher, _Functor f);

    static bool infoLessThan(const ActivityInfo &info, const ActivityInfo &other)
    {
        const auto comp = QString::compare(info.name, other.name, Qt::CaseInsensitive);
        return comp < 0 || (comp == 0 && info.id < other.id);
    }

    ActivityInfoList::iterator find(const QString &id)
    {
        return std::find_if(m_activities.begin(), m_activities.end(), [&id](const ActivityInfo &info) {
            return info.id == id;
        });
    }

    ActivityInfoList::iterator lower_bound(const ActivityInfo &info)
    {
        return std::lower_bound(m_activities.begin(), m_activities.end(), info, &infoLessThan);
    }

    template<int Policy = kamd::utils::Const>
    inline typename kamd::utils::ptr_to<ActivityInfo, Policy>::type getInfo(const QString &id)
    {
        const auto where = find(id);

        if (where != m_activities.end()) {
            return &(*where);
        }

        return nullptr;
    }

    template<typename TargetSlot>
    void onCallFinished(QDBusPendingCall &call, TargetSlot slot)
    {
        auto watcher = new QDBusPendingCallWatcher(call, this);

        connect(watcher, SIGNAL(finished(QDBusPendingCallWatcher *)), this, slot);
    }

    ActivitiesCache();

    QList<ActivityInfo> m_activities;
    QString m_currentActivity;
    Consumer::ServiceStatus m_status;
};

} // namespace KActivities

#endif /* ACTIVITIES_CACHE_P_H */
