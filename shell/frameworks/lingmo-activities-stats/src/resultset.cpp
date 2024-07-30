/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "resultset.h"

// Qt
#include <QCoreApplication>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QUrl>

// Local
#include "lingmo-activities-stats-logsettings.h"
#include <common/database/Database.h>
#include <common/specialvalues.h>
#include <utils/debug_and_return.h>
#include <utils/qsqlquery_iterator.h>

// STL
#include <functional>
#include <iterator>
#include <mutex>

// KActivities
#include "activitiessync_p.h"

#define DEBUG_QUERIES 0

namespace KActivities
{
namespace Stats
{
using namespace Terms;

class ResultSet_ResultPrivate
{
public:
    QString resource;
    QString title;
    QString mimetype;
    double score;
    uint lastUpdate;
    uint firstUpdate;
    ResultSet::Result::LinkStatus linkStatus;
    QStringList linkedActivities;
    QString agent;
};

ResultSet::Result::Result()
    : d(new ResultSet_ResultPrivate())
{
}

ResultSet::Result::Result(Result &&result)
    : d(result.d)
{
    result.d = nullptr;
}

ResultSet::Result::Result(const Result &result)
    : d(new ResultSet_ResultPrivate(*result.d))
{
}

ResultSet::Result &ResultSet::Result::operator=(Result result)
{
    std::swap(d, result.d);

    return *this;
}

ResultSet::Result::~Result()
{
    delete d;
}

#define CREATE_GETTER_AND_SETTER(Type, Name, Set)                                                                                                              \
    Type ResultSet::Result::Name() const                                                                                                                       \
    {                                                                                                                                                          \
        return d->Name;                                                                                                                                        \
    }                                                                                                                                                          \
                                                                                                                                                               \
    void ResultSet::Result::Set(Type Name)                                                                                                                     \
    {                                                                                                                                                          \
        d->Name = Name;                                                                                                                                        \
    }

CREATE_GETTER_AND_SETTER(QString, resource, setResource)
CREATE_GETTER_AND_SETTER(QString, title, setTitle)
CREATE_GETTER_AND_SETTER(QString, mimetype, setMimetype)
CREATE_GETTER_AND_SETTER(double, score, setScore)
CREATE_GETTER_AND_SETTER(uint, lastUpdate, setLastUpdate)
CREATE_GETTER_AND_SETTER(uint, firstUpdate, setFirstUpdate)
CREATE_GETTER_AND_SETTER(ResultSet::Result::LinkStatus, linkStatus, setLinkStatus)
CREATE_GETTER_AND_SETTER(QStringList, linkedActivities, setLinkedActivities)
CREATE_GETTER_AND_SETTER(QString, agent, setAgent)

#undef CREATE_GETTER_AND_SETTER

QUrl ResultSet::Result::url() const
{
    if (QDir::isAbsolutePath(d->resource)) {
        return QUrl::fromLocalFile(d->resource);
    } else {
        return QUrl(d->resource);
    }
}

class ResultSetPrivate
{
public:
    Common::Database::Ptr database;
    QSqlQuery query;
    Query queryDefinition;

    mutable ActivitiesSync::ConsumerPtr activities;

    void initQuery()
    {
        if (!database || query.isActive()) {
            return;
        }

        auto selection = queryDefinition.selection();

        query = database->execQuery(replaceQueryParameters( //
            selection == LinkedResources     ? linkedResourcesQuery()
                : selection == UsedResources ? usedResourcesQuery()
                : selection == AllResources  ? allResourcesQuery()
                                             : QString()));

        if (query.lastError().isValid()) {
            qCWarning(LINGMO_ACTIVITIES_STATS_LOG) << "[Error at ResultSetPrivate::initQuery]: " << query.lastError();
        }
    }

    QString agentClause(const QString &agent) const
    {
        if (agent == QLatin1String(":any")) {
            return QStringLiteral("1");
        }

        return QLatin1String("agent = '")
            + Common::escapeSqliteLikePattern(agent == QLatin1String(":current") ? QCoreApplication::instance()->applicationName() : agent)
            + QLatin1String("'");
    }

    QString activityClause(const QString &activity) const
    {
        if (activity == QLatin1String(":any")) {
            return QStringLiteral("1");
        }

        return QLatin1String("activity = '") + //
            Common::escapeSqliteLikePattern(activity == QLatin1String(":current") ? ActivitiesSync::currentActivity(activities) : activity)
            + QLatin1String("'");
    }

    inline QString starPattern(const QString &pattern) const
    {
        return Common::parseStarPattern(pattern, QStringLiteral("%"), [](QString str) {
            return str.replace(QLatin1String("%"), QLatin1String("\\%")).replace(QLatin1String("_"), QLatin1String("\\_"));
        });
    }

    QString urlFilterClause(const QString &urlFilter) const
    {
        if (urlFilter == QLatin1String("*")) {
            return QStringLiteral("1");
        }

        return QLatin1String("resource LIKE '") + Common::starPatternToLike(urlFilter) + QLatin1String("' ESCAPE '\\'");
    }

    QString mimetypeClause(const QString &mimetype) const
    {
        if (mimetype == ANY_TYPE_TAG || mimetype == QLatin1String("*")) {
            return QStringLiteral("1");

        } else if (mimetype == FILES_TYPE_TAG) {
            return QStringLiteral("mimetype != 'inode/directory' AND mimetype != ''");
        } else if (mimetype == DIRECTORIES_TYPE_TAG) {
            return QStringLiteral("mimetype = 'inode/directory'");
        }

        return QLatin1String("mimetype LIKE '") + Common::starPatternToLike(mimetype) + QLatin1String("' ESCAPE '\\'");
    }

    QString dateClause(QDate start, QDate end) const
    {
        if (end.isNull()) {
            // only date filtering
            return QLatin1String("DATE(re.start, 'unixepoch') = '") + start.toString(Qt::ISODate) + QLatin1String("' ");
        } else {
            // date range filtering
            return QLatin1String("DATE(re.start, 'unixepoch') >= '") + start.toString(Qt::ISODate) + QLatin1String("' AND DATE(re.start, 'unixepoch') <= '")
                + end.toString(Qt::ISODate) + QLatin1String("' ");
        }
    }
    QString titleClause(const QString titleFilter) const
    {
        if (titleFilter == QLatin1String("*")) {
            return QStringLiteral("1");
        }

        return QLatin1String("title LIKE '") + Common::starPatternToLike(titleFilter) + QLatin1String("' ESCAPE '\\'");
    }

    QString resourceEventJoinClause() const
    {
        return QStringLiteral(R"(
            LEFT JOIN
                ResourceEvent re
                ON  from_table.targettedResource = re.targettedResource
                AND from_table.usedActivity      = re.usedActivity
                AND from_table.initiatingAgent   = re.initiatingAgent
        )");
    }

    /**
     * Transforms the input list's elements with the f member method,
     * and returns the resulting list
     */
    template<typename F>
    inline QStringList transformedList(const QStringList &input, F f) const
    {
        using namespace std::placeholders;

        QStringList result;
        std::transform(input.cbegin(), input.cend(), std::back_inserter(result), std::bind(f, this, _1));

        return result;
    }

    QString limitOffsetSuffix() const
    {
        QString result;

        const int limit = queryDefinition.limit();
        if (limit > 0) {
            result += QLatin1String(" LIMIT ") + QString::number(limit);

            const int offset = queryDefinition.offset();
            if (offset > 0) {
                result += QLatin1String(" OFFSET ") + QString::number(offset);
            }
        }

        return result;
    }

    inline QString replaceQueryParameters(const QString &_query) const
    {
        // ORDER BY column
        auto ordering = queryDefinition.ordering();
        QString orderingColumn = QLatin1String("linkStatus DESC, ")
            + (ordering == HighScoredFirst            ? QLatin1String("score DESC,")
                   : ordering == RecentlyCreatedFirst ? QLatin1String("firstUpdate DESC,")
                   : ordering == RecentlyUsedFirst    ? QLatin1String("lastUpdate DESC,")
                   : ordering == OrderByTitle         ? QLatin1String("title ASC,")
                                                      : QLatin1String());

        // WHERE clause for filtering on agents
        QStringList agentsFilter = transformedList(queryDefinition.agents(), &ResultSetPrivate::agentClause);

        // WHERE clause for filtering on activities
        QStringList activitiesFilter = transformedList(queryDefinition.activities(), &ResultSetPrivate::activityClause);

        // WHERE clause for filtering on resource URLs
        QStringList urlFilter = transformedList(queryDefinition.urlFilters(), &ResultSetPrivate::urlFilterClause);

        // WHERE clause for filtering on resource mime
        QStringList mimetypeFilter = transformedList(queryDefinition.types(), &ResultSetPrivate::mimetypeClause);
        QStringList titleFilter = transformedList(queryDefinition.titleFilters(), &ResultSetPrivate::titleClause);

        QString dateColumn = QStringLiteral("1");
        QString resourceEventJoin;
        // WHERE clause for access date filtering and ResourceEvent table Join
        if (!queryDefinition.dateStart().isNull()) {
            dateColumn = dateClause(queryDefinition.dateStart(), queryDefinition.dateEnd());

            resourceEventJoin = resourceEventJoinClause();
        }

        auto queryString = _query;

        queryString.replace(QLatin1String("ORDER_BY_CLAUSE"), QLatin1String("ORDER BY $orderingColumn resource ASC"))
            .replace(QLatin1String("LIMIT_CLAUSE"), limitOffsetSuffix());

        const QString replacedQuery =
            queryString.replace(QLatin1String("$orderingColumn"), orderingColumn)
                .replace(QLatin1String("$agentsFilter"), agentsFilter.join(QStringLiteral(" OR ")))
                .replace(QLatin1String("$activitiesFilter"), activitiesFilter.join(QStringLiteral(" OR ")))
                .replace(QLatin1String("$urlFilter"), urlFilter.join(QStringLiteral(" OR ")))
                .replace(QLatin1String("$mimetypeFilter"), mimetypeFilter.join(QStringLiteral(" OR ")))
                .replace(QLatin1String("$resourceEventJoin"), resourceEventJoin)
                .replace(QLatin1String("$dateFilter"), dateColumn)
                .replace(QLatin1String("$titleFilter"), titleFilter.isEmpty() ? QStringLiteral("1") : titleFilter.join(QStringLiteral(" OR ")));
        return kamd::utils::debug_and_return(DEBUG_QUERIES, "Query: ", replacedQuery);
    }

    static const QString &linkedResourcesQuery()
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated, although, for this query,
        //       scores are not that important.
        static const QString queryString = QStringLiteral(R"(
            SELECT
                from_table.targettedResource as resource
              , SUM(rsc.cachedScore)         as score
              , MIN(rsc.firstUpdate)         as firstUpdate
              , MAX(rsc.lastUpdate)          as lastUpdate
              , from_table.usedActivity      as activity
              , from_table.initiatingAgent   as agent
              , COALESCE(ri.title, from_table.targettedResource) as title
              , ri.mimetype as mimetype
              , 2 as linkStatus

            FROM
                ResourceLink from_table
            LEFT JOIN
                ResourceScoreCache rsc
                ON  from_table.targettedResource = rsc.targettedResource
                AND from_table.usedActivity      = rsc.usedActivity
                AND from_table.initiatingAgent   = rsc.initiatingAgent
            LEFT JOIN
                ResourceInfo ri
                ON from_table.targettedResource = ri.targettedResource

            $resourceEventJoin

            WHERE
                ($agentsFilter)
                AND ($activitiesFilter)
                AND ($urlFilter)
                AND ($mimetypeFilter)
                AND ($dateFilter)
                AND ($titleFilter)

            GROUP BY resource, title

            ORDER_BY_CLAUSE
            LIMIT_CLAUSE
            )");

        return queryString;
    }

    static const QString &usedResourcesQuery()
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated
        static const QString queryString = QStringLiteral(R"(
            SELECT
                from_table.targettedResource as resource
              , SUM(from_table.cachedScore)  as score
              , MIN(from_table.firstUpdate)  as firstUpdate
              , MAX(from_table.lastUpdate)   as lastUpdate
              , from_table.usedActivity      as activity
              , from_table.initiatingAgent   as agent
              , COALESCE(ri.title, from_table.targettedResource) as title
              , ri.mimetype as mimetype
              , 1 as linkStatus

            FROM
                ResourceScoreCache from_table
            LEFT JOIN
                ResourceInfo ri
                ON from_table.targettedResource = ri.targettedResource

            $resourceEventJoin

            WHERE
                ($agentsFilter)
                AND ($activitiesFilter)
                AND ($urlFilter)
                AND ($mimetypeFilter)
                AND ($dateFilter)
                AND ($titleFilter)

            GROUP BY resource, title

            ORDER_BY_CLAUSE
            LIMIT_CLAUSE
            )");

        return queryString;
    }

    static const QString &allResourcesQuery()
    {
        // TODO: We need to correct the scores based on the time that passed
        //       since the cache was last updated, although, for this query,
        //       scores are not that important.
        static const QString queryString = QStringLiteral(R"(
            WITH
                LinkedResourcesResults AS (
                    SELECT from_table.targettedResource as resource
                         , rsc.cachedScore              as score
                         , rsc.firstUpdate              as firstUpdate
                         , rsc.lastUpdate               as lastUpdate
                         , from_table.usedActivity      as activity
                         , from_table.initiatingAgent   as agent
                         , 2 as linkStatus

                    FROM
                        ResourceLink from_table

                    LEFT JOIN
                        ResourceScoreCache rsc
                        ON  from_table.targettedResource = rsc.targettedResource
                        AND from_table.usedActivity      = rsc.usedActivity
                        AND from_table.initiatingAgent   = rsc.initiatingAgent

                    $resourceEventJoin

                    WHERE
                        ($agentsFilter)
                        AND ($activitiesFilter)
                        AND ($urlFilter)
                        AND ($mimetypeFilter)
                        AND ($dateFilter)
                        AND ($titleFilter)
                ),

                UsedResourcesResults AS (
                    SELECT from_table.targettedResource as resource
                         , from_table.cachedScore       as score
                         , from_table.firstUpdate       as firstUpdate
                         , from_table.lastUpdate        as lastUpdate
                         , from_table.usedActivity      as activity
                         , from_table.initiatingAgent   as agent
                         , 0 as linkStatus

                    FROM
                        ResourceScoreCache from_table

                    $resourceEventJoin

                    WHERE
                        ($agentsFilter)
                        AND ($activitiesFilter)
                        AND ($urlFilter)
                        AND ($mimetypeFilter)
                        AND ($dateFilter)
                        AND ($titleFilter)
                ),

                CollectedResults AS (
                    SELECT *
                    FROM LinkedResourcesResults

                    UNION

                    SELECT *
                    FROM UsedResourcesResults
                    WHERE resource NOT IN (SELECT resource FROM LinkedResourcesResults)
                )

                SELECT
                    resource
                  , SUM(score) as score
                  , MIN(firstUpdate) as firstUpdate
                  , MAX(lastUpdate) as lastUpdate
                  , activity
                  , agent
                  , COALESCE(ri.title, resource) as title
                  , ri.mimetype as mimetype
                  , linkStatus

                FROM CollectedResults cr

                LEFT JOIN
                    ResourceInfo ri
                    ON cr.resource = ri.targettedResource

                GROUP BY resource, title

                ORDER_BY_CLAUSE
                LIMIT_CLAUSE
            )");

        return queryString;
    }

    ResultSet::Result currentResult() const
    {
        ResultSet::Result result;

        if (!database || !query.isActive()) {
            return result;
        }

        result.setResource(query.value(QStringLiteral("resource")).toString());
        result.setTitle(query.value(QStringLiteral("title")).toString());
        result.setMimetype(query.value(QStringLiteral("mimetype")).toString());
        result.setScore(query.value(QStringLiteral("score")).toDouble());
        result.setLastUpdate(query.value(QStringLiteral("lastUpdate")).toUInt());
        result.setFirstUpdate(query.value(QStringLiteral("firstUpdate")).toUInt());
        result.setAgent(query.value(QStringLiteral("agent")).toString());

        result.setLinkStatus(static_cast<ResultSet::Result::LinkStatus>(query.value(QStringLiteral("linkStatus")).toUInt()));

        auto linkedActivitiesQuery = database->createQuery();

        linkedActivitiesQuery.prepare(QStringLiteral(R"(
            SELECT usedActivity
            FROM   ResourceLink
            WHERE  targettedResource = :resource
            )"));

        linkedActivitiesQuery.bindValue(QStringLiteral(":resource"), result.resource());
        linkedActivitiesQuery.exec();

        QStringList linkedActivities;
        for (const auto &item : linkedActivitiesQuery) {
            linkedActivities << item[0].toString();
        }

        result.setLinkedActivities(linkedActivities);
        // qDebug(LINGMO_ACTIVITIES_STATS_LOG) << result.resource() << "linked to activities" << result.linkedActivities();

        return result;
    }
};

ResultSet::ResultSet(Query queryDefinition)
    : d(new ResultSetPrivate())
{
    using namespace Common;

    d->database = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);

    if (!(d->database)) {
        qCWarning(LINGMO_ACTIVITIES_STATS_LOG) << "Lingmo Activities ERROR: There is no database. This probably means "
                                            "that you do not have the Activity Manager running, or that "
                                            "something else is broken on your system. Recent documents and "
                                            "alike will not work!";
    }

    d->queryDefinition = queryDefinition;

    d->initQuery();
}

ResultSet::ResultSet(ResultSet &&source)
    : d(nullptr)
{
    std::swap(d, source.d);
}

ResultSet::~ResultSet()
{
    delete d;
}

ResultSet::Result ResultSet::at(int index) const
{
    if (!d->query.isActive()) {
        return Result();
    }

    d->query.seek(index);

    return d->currentResult();
}

} // namespace Stats
} // namespace KActivities

#include "resultset_iterator.cpp"
