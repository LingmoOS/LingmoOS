/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "consumer.h"
#include "consumer_p.h"
#include "manager_p.h"

namespace KActivities
{
ConsumerPrivate::ConsumerPrivate()
    : cache(ActivitiesCache::self())
{
}

void ConsumerPrivate::setServiceStatus(Consumer::ServiceStatus status)
{
    Q_EMIT serviceStatusChanged(status);
}

Consumer::Consumer(QObject *parent)
    : QObject(parent)
    , d(new ConsumerPrivate())
{
    connect(d->cache.get(), &KActivities::ActivitiesCache::currentActivityChanged, this, &Consumer::currentActivityChanged);
    connect(d->cache.get(), &KActivities::ActivitiesCache::activityAdded, this, &Consumer::activityAdded);
    connect(d->cache.get(), &KActivities::ActivitiesCache::activityRemoved, this, &Consumer::activityRemoved);
    connect(d->cache.get(), &KActivities::ActivitiesCache::serviceStatusChanged, this, &Consumer::serviceStatusChanged);

    connect(d->cache.get(), &ActivitiesCache::activityListChanged, this, [=, this]() {
        Q_EMIT activitiesChanged(activities());
    });
    connect(d->cache.get(), &ActivitiesCache::runningActivityListChanged, this, [=, this]() {
        Q_EMIT runningActivitiesChanged(runningActivities());
    });

    // connect(d->cache.get(), SIGNAL(activityStateChanged(QString,int)),
    //         this, SIGNAL(activityStateChanged(QString,int)));
}

Consumer::~Consumer() = default;

QString Consumer::currentActivity() const
{
    return d->cache->m_currentActivity;
}

QStringList Consumer::activities(Info::State state) const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    for (const auto &info : std::as_const(d->cache->m_activities)) {
        if (info.state == state) {
            result << info.id;
        }
    }

    return result;
}

QStringList Consumer::activities() const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    for (const auto &info : std::as_const(d->cache->m_activities)) {
        result << info.id;
    }

    return result;
}

QStringList Consumer::runningActivities() const
{
    QStringList result;

    result.reserve(d->cache->m_activities.size());

    for (const auto &info : std::as_const(d->cache->m_activities)) {
        if (info.state == Info::Running || info.state == Info::Stopping) {
            result << info.id;
        }
    }

    return result;
}

Consumer::ServiceStatus Consumer::serviceStatus()
{
    return d->cache->m_status;
}

} // namespace KActivities

#include "moc_consumer.cpp"
#include "moc_consumer_p.cpp"
