/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef CLEANING_H
#define CLEANING_H

#include "query.h"
#include "terms.h"
#include <QString>

namespace KActivities
{
namespace Stats
{
/**
 * Forget the resource(s) for the specified activity and agent
 */
void PLASMAACTIVITIESSTATS_EXPORT forgetResource(Terms::Activity activity, Terms::Agent agent, const QString &resource);

enum PLASMAACTIVITIESSTATS_EXPORT TimeUnit {
    Hours,
    Days,
    Months,
};

/**
 * Forget recent stats for the specified activity and time
 */
void PLASMAACTIVITIESSTATS_EXPORT forgetRecentStats(Terms::Activity activity, int count, TimeUnit what);

/**
 * Forget events that are older than the specified number of months
 */
void PLASMAACTIVITIESSTATS_EXPORT forgetEarlierStats(Terms::Activity activity, int months);

/**
 * Forget resources that match the specified query
 */
void PLASMAACTIVITIESSTATS_EXPORT forgetResources(const Query &query);

} // namespace Stats
} // namespace KActivities

#endif // CLEANING_H
