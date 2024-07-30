/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESOURCESCORECACHE_TABLE_H
#define RESOURCESCORECACHE_TABLE_H

#include <QString>

#include <tuple>

#include "common.h"

namespace ResourceScoreCache
{
struct Item {
    QString usedActivity;
    QString initiatingAgent;
    QString targettedResource;

    double cachedScore;
    int firstUpdate;
    int lastUpdate;

    // defining the primary key

    inline std::tuple<const QString &, const QString &, const QString &> primaryKey() const
    {
        return std::tie(targettedResource, usedActivity, initiatingAgent);
    }
};

DECL_COLUMN(QString, usedActivity)
DECL_COLUMN(QString, initiatingAgent)
DECL_COLUMN(QString, targettedResource)

DECL_COLUMN(double, cachedScore)
DECL_COLUMN(int, lastUpdate)
DECL_COLUMN(int, firstUpdate)

template<typename Range>
inline std::vector<Item> groupByResource(const Range &range)
{
    return groupBy(range, &Item::targettedResource, [](Item &acc, const Item &item) {
        acc.cachedScore += item.cachedScore;
        if (acc.lastUpdate < item.lastUpdate) {
            acc.lastUpdate = item.lastUpdate;
        }
        if (acc.firstUpdate > item.firstUpdate) {
            acc.firstUpdate = item.firstUpdate;
        }
    });
}

}

#endif // RESOURCESCORECACHE_TABLE_H
