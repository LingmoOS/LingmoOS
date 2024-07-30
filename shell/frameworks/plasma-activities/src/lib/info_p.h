/*
    SPDX-FileCopyrightText: 2010-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIESINFO_P_H
#define KACTIVITIESINFO_P_H

#include "info.h"
#include <memory>

#include "activitiescache_p.h"

namespace KActivities
{
class InfoPrivate
{
public:
    InfoPrivate(Info *info, const QString &activity);

    void activityStateChanged(const QString &, int) const;

    void added(const QString &) const;
    void removed(const QString &) const;
    void started(const QString &) const;
    void stopped(const QString &) const;
    void infoChanged(const QString &) const;
    void nameChanged(const QString &, const QString &) const;
    void descriptionChanged(const QString &, const QString &) const;
    void iconChanged(const QString &, const QString &) const;
    void setServiceStatus(Consumer::ServiceStatus status) const;
    void setCurrentActivity(const QString &currentActivity);

    Info *const q;
    std::shared_ptr<ActivitiesCache> cache;
    bool isCurrent;

    const QString id;
};

} // namespace KActivities

#endif // KACTIVITIESINFO_P_H
