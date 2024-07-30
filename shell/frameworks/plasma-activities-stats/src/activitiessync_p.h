/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_SYNC_P_H
#define ACTIVITIES_SYNC_P_H

#include <memory>
#include <mutex>

#include <PlasmaActivities/Consumer>

namespace ActivitiesSync
{
typedef std::shared_ptr<KActivities::Consumer> ConsumerPtr;

QString currentActivity(ConsumerPtr &activities);

} // namespace ActivitiesSync

#endif // ACTIVITIES_SYNC_P_H
