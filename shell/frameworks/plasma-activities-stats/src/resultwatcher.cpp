/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "resultwatcher.h"

// Qt
#include <QCoreApplication>
#include <QList>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>

// Local
#include "plasma-activities-stats-logsettings.h"
#include <common/database/Database.h>
#include <utils/debug_and_return.h>

// STL
#include <functional>
#include <iterator>
#include <limits>
#include <mutex>

// PlasmaActivities
#include <plasmaactivities/consumer.h>

#include "activitiessync_p.h"
#include "common/dbus/common.h"
#include "common/specialvalues.h"
#include "resourceslinking_interface.h"
#include "resourcesscoring_interface.h"
#include "utils/lazy_val.h"
#include "utils/qsqlquery_iterator.h"

#include <algorithm>

#define QDBG qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "PlasmaActivitiesStats(" << (void *)this << ")"

namespace KActivities
{
namespace Stats
{
// Main class

class ResultWatcherPrivate
{
public:
    mutable ActivitiesSync::ConsumerPtr activities;
    QList<QRegularExpression> urlFilters;

    ResultWatcherPrivate(ResultWatcher *parent, Query query)
        : linking(KAMD_DBUS_SERVICE, QStringLiteral("/ActivityManager/Resources/Linking"), QDBusConnection::sessionBus(), nullptr)
        , scoring(KAMD_DBUS_SERVICE, QStringLiteral("/ActivityManager/Resources/Scoring"), QDBusConnection::sessionBus(), nullptr)
        , q(parent)
        , query(query)
    {
        for (const auto &urlFilter : query.urlFilters()) {
            urlFilters << Common::starPatternToRegex(urlFilter);
        }

        m_resultInvalidationTimer.setSingleShot(true);
        m_resultInvalidationTimer.setInterval(200);
        QObject::connect(&m_resultInvalidationTimer, &QTimer::timeout, q, Q_EMIT & ResultWatcher::resultsInvalidated);
    }

    template<typename Collection, typename Predicate>
    inline bool any_of(const Collection &collection, Predicate &&predicate) const
    {
        const auto begin = collection.cbegin();
        const auto end = collection.cend();

        return begin == end || std::any_of(begin, end, std::forward<Predicate>(predicate));
    }

#define DEBUG_MATCHERS 0

    // Processing the list of activities as specified by the query.
    // If it contains :any, we are returning true, otherwise
    // we want to match a specific activity (be it the current
    // activity or not). The :global special value is not special here
    bool activityMatches(const QString &activity) const
    {
#if DEBUG_MATCHERS
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Activity " << activity << "matching against" << query.activities();
#endif

        return kamd::utils::debug_and_return(DEBUG_MATCHERS,
                                             " -> returning ",
                                             activity == ANY_ACTIVITY_TAG || any_of(query.activities(), [&](const QString &matcher) {
                                                 return matcher == ANY_ACTIVITY_TAG ? true
                                                     : matcher == CURRENT_ACTIVITY_TAG
                                                     ? (matcher == activity || activity == ActivitiesSync::currentActivity(activities))
                                                     : activity == matcher;
                                             }));
    }

    // Same as above, but for agents
    bool agentMatches(const QString &agent) const
    {
#if DEBUG_MATCHERS
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Agent " << agent << "matching against" << query.agents();
#endif

        return kamd::utils::debug_and_return(DEBUG_MATCHERS, " -> returning ", agent == ANY_AGENT_TAG || any_of(query.agents(), [&](const QString &matcher) {
                                                                                   return matcher == ANY_AGENT_TAG ? true
                                                                                       : matcher == CURRENT_AGENT_TAG
                                                                                       ? (matcher == agent || agent == QCoreApplication::applicationName())
                                                                                       : agent == matcher;
                                                                               }));
    }

    // Same as above, but for urls
    bool urlMatches(const QString &url) const
    {
#if DEBUG_MATCHERS
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Url " << url << "matching against" << urlFilters;
#endif

        return kamd::utils::debug_and_return(DEBUG_MATCHERS, " -> returning ", any_of(urlFilters, [&](const QRegularExpression &matcher) {
                                                 return matcher.match(url).hasMatch();
                                             }));
    }

    bool typeMatches(const QString &resource) const
    {
        // We don't necessarily need to retrieve the type from
        // the database. If we do, get it only once
        auto type = kamd::utils::make_lazy_val([&]() -> QString {
            using Common::Database;

            auto database = Database::instance(Database::ResourcesDatabase, Database::ReadOnly);

            if (!database) {
                return QString();
            }

            auto query = database->execQuery(QStringLiteral("SELECT mimetype FROM ResourceInfo WHERE "
                                                            "targettedResource = '")
                                             + resource + QStringLiteral("'"));

            for (const auto &item : query) {
                return item[0].toString();
            }

            return QString();
        });

#if DEBUG_MATCHERS
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Type "
                                       << "...type..."
                                       << "matching against" << query.types();
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "ANY_TYPE_TAG" << ANY_TYPE_TAG;
#endif

        return kamd::utils::debug_and_return(DEBUG_MATCHERS, " -> returning ", any_of(query.types(), [&](const QString &matcher) {
                                                 if (matcher == ANY_TYPE_TAG) {
                                                     return true;
                                                 }

                                                 const QString _type = type;
                                                 return matcher == ANY_TYPE_TAG
                                                     || (matcher == FILES_TYPE_TAG && !_type.isEmpty() && _type != QStringLiteral("inode/directory"))
                                                     || (matcher == DIRECTORIES_TYPE_TAG && _type == QLatin1String("inode/directory")) || matcher == type;
                                             }));
    }

    bool eventMatches(const QString &agent, const QString &resource, const QString &activity) const
    {
        // The order of checks is not arbitrary, it is sorted
        // from the cheapest, to the most expensive
        return kamd::utils::debug_and_return(DEBUG_MATCHERS,
                                             "event matches?",
                                             agentMatches(agent) && activityMatches(activity) && urlMatches(resource) && typeMatches(resource));
    }

    void onResourceLinkedToActivity(const QString &agent, const QString &resource, const QString &activity)
    {
#if DEBUG_MATCHERS
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Resource has been linked: " << agent << resource << activity;
#endif

        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) {
            return;
        }

        if (!eventMatches(agent, resource, activity)) {
            return;
        }

        // TODO: See whether it makes sense to have
        //       lastUpdate/firstUpdate here as well
        Q_EMIT q->resultLinked(resource);
    }

    void onResourceUnlinkedFromActivity(const QString &agent, const QString &resource, const QString &activity)
    {
#if DEBUG_MATCHERS
        qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Resource unlinked: " << agent << resource << activity;
#endif

        // The used resources do not really care about the linked ones
        if (query.selection() == Terms::UsedResources) {
            return;
        }

        if (!eventMatches(agent, resource, activity)) {
            return;
        }

        Q_EMIT q->resultUnlinked(resource);
    }

#undef DEBUG_MATCHERS

    void onResourceScoreUpdated(const QString &activity, const QString &agent, const QString &resource, double score, uint lastUpdate, uint firstUpdate)
    {
        Q_ASSERT_X(activity == QLatin1String("00000000-0000-0000-0000-000000000000") || !QUuid(activity).isNull(),
                   "ResultWatcher::onResourceScoreUpdated",
                   "The activity should be always specified here, no magic values");

        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) {
            return;
        }

        if (!eventMatches(agent, resource, activity)) {
            return;
        }

        Q_EMIT q->resultScoreUpdated(resource, score, lastUpdate, firstUpdate);
    }

    void onEarlierStatsDeleted(QString, int)
    {
        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) {
            return;
        }

        scheduleResultsInvalidation();
    }

    void onRecentStatsDeleted(QString, int, QString)
    {
        // The linked resources do not really care about the stats
        if (query.selection() == Terms::LinkedResources) {
            return;
        }

        scheduleResultsInvalidation();
    }

    void onStatsForResourceDeleted(const QString &activity, const QString &agent, const QString &resource)
    {
        if (query.selection() == Terms::LinkedResources) {
            return;
        }

        if (activityMatches(activity) && agentMatches(agent)) {
            if (resource.contains(QLatin1Char('*'))) {
                scheduleResultsInvalidation();

            } else if (typeMatches(resource)) {
                if (!m_resultInvalidationTimer.isActive()) {
                    // Remove a result only if we haven't an invalidation
                    // request scheduled
                    q->resultRemoved(resource);
                }
            }
        }
    }

    // Lets not send a lot of invalidation events at once
    QTimer m_resultInvalidationTimer;
    void scheduleResultsInvalidation()
    {
        QDBG << "Scheduling invalidation";
        m_resultInvalidationTimer.start();
    }

    org::kde::ActivityManager::ResourcesLinking linking;
    org::kde::ActivityManager::ResourcesScoring scoring;

    ResultWatcher *const q;
    Query query;
};

ResultWatcher::ResultWatcher(Query query, QObject *parent)
    : QObject(parent)
    , d(new ResultWatcherPrivate(this, query))
{
    using namespace org::kde::ActivityManager;
    using namespace std::placeholders;

    // There is no need for private slots, when we have bind

    // Connecting the linking service
    QObject::connect(&d->linking,
                     &ResourcesLinking::ResourceLinkedToActivity,
                     this,
                     std::bind(&ResultWatcherPrivate::onResourceLinkedToActivity, d, _1, _2, _3));
    QObject::connect(&d->linking,
                     &ResourcesLinking::ResourceUnlinkedFromActivity,
                     this,
                     std::bind(&ResultWatcherPrivate::onResourceUnlinkedFromActivity, d, _1, _2, _3));

    // Connecting the scoring service
    QObject::connect(&d->scoring,
                     &ResourcesScoring::ResourceScoreUpdated,
                     this,
                     std::bind(&ResultWatcherPrivate::onResourceScoreUpdated, d, _1, _2, _3, _4, _5, _6));
    QObject::connect(&d->scoring, &ResourcesScoring::ResourceScoreDeleted, this, std::bind(&ResultWatcherPrivate::onStatsForResourceDeleted, d, _1, _2, _3));
    QObject::connect(&d->scoring, &ResourcesScoring::RecentStatsDeleted, this, std::bind(&ResultWatcherPrivate::onRecentStatsDeleted, d, _1, _2, _3));
    QObject::connect(&d->scoring, &ResourcesScoring::EarlierStatsDeleted, this, std::bind(&ResultWatcherPrivate::onEarlierStatsDeleted, d, _1, _2));
}

ResultWatcher::~ResultWatcher()
{
    delete d;
}

void ResultWatcher::linkToActivity(const QUrl &resource, const Terms::Activity &activity, const Terms::Agent &agent)
{
    const auto activities = (!activity.values.isEmpty()) ? activity.values
        : (!d->query.activities().isEmpty())             ? d->query.activities()
                                                         : Terms::Activity::current().values;
    const auto agents = (!agent.values.isEmpty()) ? agent.values : (!d->query.agents().isEmpty()) ? d->query.agents() : Terms::Agent::current().values;

    for (const auto &activity : activities) {
        for (const auto &agent : agents) {
            d->linking.LinkResourceToActivity(agent, resource.toString(), activity);
        }
    }
}

void ResultWatcher::unlinkFromActivity(const QUrl &resource, const Terms::Activity &activity, const Terms::Agent &agent)
{
    const auto activities = !activity.values.isEmpty() ? activity.values
        : !d->query.activities().isEmpty()             ? d->query.activities()
                                                       : Terms::Activity::current().values;
    const auto agents = !agent.values.isEmpty() ? agent.values : !d->query.agents().isEmpty() ? d->query.agents() : Terms::Agent::current().values;

    for (const auto &activity : activities) {
        for (const auto &agent : agents) {
            qCDebug(PLASMA_ACTIVITIES_STATS_LOG) << "Unlink " << agent << resource << activity;
            d->linking.UnlinkResourceFromActivity(agent, resource.toString(), activity);
        }
    }
}

} // namespace Stats
} // namespace KActivities

#include "moc_resultwatcher.cpp"
