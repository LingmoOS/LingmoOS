/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef ACTIVITIES_CONSUMER_P_H
#define ACTIVITIES_CONSUMER_P_H

#include "consumer.h"

#include <memory>

#include "activitiescache_p.h"

namespace KActivities
{
class ConsumerPrivate : public QObject
{
    Q_OBJECT

public:
    ConsumerPrivate();

    std::shared_ptr<ActivitiesCache> cache;

public Q_SLOTS:
    void setServiceStatus(Consumer::ServiceStatus status);

Q_SIGNALS:
    void serviceStatusChanged(Consumer::ServiceStatus status);
};

} // namespace KActivities

#endif // ACTIVITIES_CONSUMER_P_H
