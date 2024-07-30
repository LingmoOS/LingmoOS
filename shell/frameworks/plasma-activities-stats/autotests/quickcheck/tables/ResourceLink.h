/*
    SPDX-FileCopyrightText: 2015 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef RESOURCELINK_TABLE_H
#define RESOURCELINK_TABLE_H

#include <QString>

#include "common.h"

namespace ResourceLink
{
struct Item {
    QString usedActivity;
    QString initiatingAgent;
    QString targettedResource;

    inline std::tuple<const QString &, const QString &, const QString &> primaryKey() const
    {
        return std::tie(targettedResource, usedActivity, initiatingAgent);
    }
};

DECL_COLUMN(QString, usedActivity)
DECL_COLUMN(QString, initiatingAgent)
DECL_COLUMN(QString, targettedResource)

template<typename Range>
inline std::vector<Item> groupByResource(const Range &range)
{
    return groupBy(range, &Item::targettedResource, [](Item &acc, const Item &item) {
        acc.usedActivity += item.usedActivity + QLatin1Char(' ');
        acc.initiatingAgent += item.initiatingAgent + QLatin1Char(' ');
    });
}

} // namespace ResourceLink

#endif // RESOURCELINK_TABLE_H
