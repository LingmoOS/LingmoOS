/*
 *   SPDX-FileCopyrightText: 2011, 2012 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

// Self
#include "ResourceScoreCache.h"
#include <kactivities-features.h>

// STD
#include <cmath>

// Utils
#include <utils/d_ptr_implementation.h>
#include <utils/qsqlquery_iterator.h>

// Local
#include "Database.h"
#include "DebugResources.h"
#include "StatsPlugin.h"
#include "Utils.h"

class ResourceScoreCache::Queries
{
private:
    Queries()
        : createResourceScoreCacheQuery(resourcesDatabase()->createQuery())
        , getResourceScoreCacheQuery(resourcesDatabase()->createQuery())
        , updateResourceScoreCacheQuery(resourcesDatabase()->createQuery())
        , getScoreAdditionQuery(resourcesDatabase()->createQuery())
    {
        Utils::prepare(*resourcesDatabase(),
                       createResourceScoreCacheQuery,
                       QStringLiteral("INSERT INTO ResourceScoreCache "
                                      "VALUES (:usedActivity, :initiatingAgent, :targettedResource, "
                                      "0, 0, " // type, score
                                      ":firstUpdate, " // lastUpdate
                                      ":firstUpdate)"));

        Utils::prepare(*resourcesDatabase(),
                       getResourceScoreCacheQuery,
                       QStringLiteral("SELECT cachedScore, lastUpdate, firstUpdate FROM ResourceScoreCache "
                                      "WHERE "
                                      ":usedActivity      = usedActivity AND "
                                      ":initiatingAgent   = initiatingAgent AND "
                                      ":targettedResource = targettedResource "));

        Utils::prepare(*resourcesDatabase(),
                       updateResourceScoreCacheQuery,
                       QStringLiteral("UPDATE ResourceScoreCache SET "
                                      "cachedScore = :cachedScore, "
                                      "lastUpdate  = :lastUpdate "
                                      "WHERE "
                                      ":usedActivity      = usedActivity AND "
                                      ":initiatingAgent   = initiatingAgent AND "
                                      ":targettedResource = targettedResource "));

        Utils::prepare(*resourcesDatabase(),
                       getScoreAdditionQuery,
                       QStringLiteral("SELECT start, end "
                                      "FROM ResourceEvent "
                                      "WHERE "
                                      ":usedActivity      = usedActivity AND "
                                      ":initiatingAgent   = initiatingAgent AND "
                                      ":targettedResource = targettedResource AND "
                                      "start > :start "
                                      "ORDER BY "
                                      "start ASC"));
    }

public:
    QSqlQuery createResourceScoreCacheQuery;
    QSqlQuery getResourceScoreCacheQuery;
    QSqlQuery updateResourceScoreCacheQuery;
    QSqlQuery getScoreAdditionQuery;

    static Queries &self();
};

ResourceScoreCache::Queries &ResourceScoreCache::Queries::self()
{
    static Queries queries;
    return queries;
}

class ResourceScoreCache::Private
{
public:
    QString activity;
    QString application;
    QString resource;

    inline qreal timeFactor(int days) const
    {
        // Exp is falling rather quickly, we are slowing it 32 times
        return std::exp(-days / 32.0);
    }

    inline qreal timeFactor(const QDateTime &fromTime, const QDateTime &toTime) const
    {
        return timeFactor(fromTime.daysTo(toTime));
    }
};

ResourceScoreCache::ResourceScoreCache(const QString &activity, const QString &application, const QString &resource)
{
    d->activity = activity;
    d->application = application;
    d->resource = resource;

    Q_ASSERT_X(!d->application.isEmpty(), "ResourceScoreCache::constructor", "Agent should not be empty");
    Q_ASSERT_X(!d->activity.isEmpty(), "ResourceScoreCache::constructor", "Activity should not be empty");
    Q_ASSERT_X(!d->resource.isEmpty(), "ResourceScoreCache::constructor", "Resource should not be empty");
}

ResourceScoreCache::~ResourceScoreCache()
{
}

void ResourceScoreCache::update()
{
    QDateTime lastUpdate;
    QDateTime firstUpdate;
    QDateTime currentTime = QDateTime::currentDateTime();
    qreal score = 0;

    DATABASE_TRANSACTION(*resourcesDatabase());

    qCDebug(KAMD_LOG_RESOURCES) << "Creating the cache for: " << d->resource;

    // This can fail if we have the cache already made
    auto isCacheNew = Utils::exec(*resourcesDatabase(),
                                  Utils::IgnoreError,
                                  Queries::self().createResourceScoreCacheQuery,
                                  ":usedActivity",
                                  d->activity,
                                  ":initiatingAgent",
                                  d->application,
                                  ":targettedResource",
                                  d->resource,
                                  ":firstUpdate",
                                  currentTime.toSecsSinceEpoch());

    // Getting the old score
    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                Queries::self().getResourceScoreCacheQuery,
                ":usedActivity",
                d->activity,
                ":initiatingAgent",
                d->application,
                ":targettedResource",
                d->resource);

    // Only and always one result
    for (const auto &result : Queries::self().getResourceScoreCacheQuery) {
        lastUpdate.setSecsSinceEpoch(result["lastUpdate"].toUInt());
        firstUpdate.setSecsSinceEpoch(result["firstUpdate"].toUInt());

        qCDebug(KAMD_LOG_RESOURCES) << "Already in database? " << (!isCacheNew);
        qCDebug(KAMD_LOG_RESOURCES) << "      First update : " << firstUpdate;
        qCDebug(KAMD_LOG_RESOURCES) << "       Last update : " << lastUpdate;

        if (isCacheNew) {
            // If we haven't had the cache before, set the score to 0
            firstUpdate = currentTime;
            score = 0;

        } else {
            // Adjusting the score depending on the time that passed since the
            // last update
            score = result["cachedScore"].toReal();
            score *= d->timeFactor(lastUpdate, currentTime);
        }
    }

    // Calculating the updated score
    // We are processing all events since the last cache update

    qCDebug(KAMD_LOG_RESOURCES) << "After the adjustment";
    qCDebug(KAMD_LOG_RESOURCES) << "     Current score : " << score;
    qCDebug(KAMD_LOG_RESOURCES) << "      First update : " << firstUpdate;
    qCDebug(KAMD_LOG_RESOURCES) << "       Last update : " << lastUpdate;

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                Queries::self().getScoreAdditionQuery,
                ":usedActivity",
                d->activity,
                ":initiatingAgent",
                d->application,
                ":targettedResource",
                d->resource,
                ":start",
                lastUpdate.toSecsSinceEpoch());

    uint lastEventStart = currentTime.toSecsSinceEpoch();

    for (const auto &result : Queries::self().getScoreAdditionQuery) {
        lastEventStart = result["start"].toUInt();

        const auto end = result["end"].toUInt();
        const auto intervalLength = end - lastEventStart;

        qCDebug(KAMD_LOG_RESOURCES) << "Interval length is " << intervalLength;

        if (intervalLength == 0) {
            // We have an Accessed event - otherwise, this wouldn't be 0
            score += d->timeFactor(QDateTime::fromSecsSinceEpoch(end), currentTime); // like it is open for 1 minute

        } else {
            score += d->timeFactor(QDateTime::fromSecsSinceEpoch(end), currentTime) * intervalLength / 60.0;
        }
    }

    qCDebug(KAMD_LOG_RESOURCES) << "         New score : " << score;

    // Updating the score

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                Queries::self().updateResourceScoreCacheQuery,
                ":usedActivity",
                d->activity,
                ":initiatingAgent",
                d->application,
                ":targettedResource",
                d->resource,
                ":cachedScore",
                score,
                ":lastUpdate",
                lastEventStart);

    // Notifying the world
    qCDebug(KAMD_LOG_RESOURCES) << "ResourceScoreUpdated:" << d->activity << d->application << d->resource;
    Q_EMIT QMetaObject::invokeMethod(StatsPlugin::self(),
                                   "ResourceScoreUpdated",
                                   Qt::QueuedConnection,
                                   Q_ARG(QString, d->activity),
                                   Q_ARG(QString, d->application),
                                   Q_ARG(QString, d->resource),
                                   Q_ARG(double, score),
                                   Q_ARG(uint, lastEventStart),
                                   Q_ARG(uint, firstUpdate.toSecsSinceEpoch()));
}
