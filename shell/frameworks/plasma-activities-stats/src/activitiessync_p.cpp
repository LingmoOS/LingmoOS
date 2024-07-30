/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "activitiessync_p.h"

#include <QCoreApplication>

namespace ActivitiesSync
{
typedef std::shared_ptr<KActivities::Consumer> ConsumerPtr;

ConsumerPtr instance()
{
    static std::mutex s_instanceMutex;
    static std::weak_ptr<KActivities::Consumer> s_instance;

    std::unique_lock<std::mutex> locker{s_instanceMutex};

    auto ptr = s_instance.lock();

    if (!ptr) {
        ptr = std::make_shared<KActivities::Consumer>();
        s_instance = ptr;
    }

    return ptr;
}

QString currentActivity(ConsumerPtr &activities)
{
    // We need to get the current activity synchonously,
    // this means waiting for the service to be available.
    // It should not introduce blockages since there usually
    // is a global activity cache in applications that care
    // about activities.

    if (!activities) {
        activities = instance();
    }

    while (activities->serviceStatus() == KActivities::Consumer::Unknown) {
        QCoreApplication::instance()->processEvents();
    }

    return activities->currentActivity();
}

} // namespace ActivitiesSync
