/*
 *   SPDX-FileCopyrightText: 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

// Self
#include "StatsPlugin.h"
#include <kactivities-features.h>

// Qt
#include <QDBusConnection>
#include <QFileSystemWatcher>
#include <QStringList>

// KDE
#include <kconfig.h>
#include <kfileitem.h>

// Boost
#include <boost/range/algorithm/binary_search.hpp>
#include <utils/range.h>

// Local
#include "../../Event.h"
#include "Database.h"
#include "ResourceLinking.h"
#include "ResourceScoreMaintainer.h"
#include "Utils.h"
#include "common/specialvalues.h"
#include "resourcescoringadaptor.h"

K_PLUGIN_CLASS(StatsPlugin)

StatsPlugin *StatsPlugin::s_instance = nullptr;

StatsPlugin::StatsPlugin(QObject *parent)
    : Plugin(parent)
    , m_activities(nullptr)
    , m_resources(nullptr)
    , m_resourceLinking(new ResourceLinking(this))
{
    s_instance = this;

    new ResourcesScoringAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/ActivityManager/Resources/Scoring"), this);

    setName(QStringLiteral("org.kde.ActivityManager.Resources.Scoring"));
}

bool StatsPlugin::init(QHash<QString, QObject *> &modules)
{
    Plugin::init(modules);

    if (!resourcesDatabase()) {
        return false;
    }

    m_activities = modules[QStringLiteral("activities")];
    m_resources = modules[QStringLiteral("resources")];

    m_resourceLinking->init();

    connect(m_resources, SIGNAL(ProcessedResourceEvents(EventList)), this, SLOT(addEvents(EventList)));
    connect(m_resources, SIGNAL(RegisteredResourceMimetype(QString, QString)), this, SLOT(saveResourceMimetype(QString, QString)));
    connect(m_resources, SIGNAL(RegisteredResourceTitle(QString, QString)), this, SLOT(saveResourceTitle(QString, QString)));

    connect(modules[QStringLiteral("config")], SIGNAL(pluginConfigChanged()), this, SLOT(loadConfiguration()));

    loadConfiguration();

    return true;
}

void StatsPlugin::loadConfiguration()
{
    auto conf = config();
    conf.config()->reparseConfiguration();

    m_blockedByDefault = conf.readEntry("blocked-by-default", false);
    m_blockAll = false;
    m_whatToRemember = (WhatToRemember)conf.readEntry("what-to-remember", (int)AllApplications);

    m_apps.clear();

    if (m_whatToRemember == SpecificApplications) {
        auto apps = conf.readEntry(m_blockedByDefault ? "allowed-applications" : "blocked-applications", QStringList());

        m_apps.unite({apps.begin(), apps.end()});
    }

    // Delete old events, as per configuration.
    // For people who do not restart their computers, we should do this from
    // time to time. Doing this twice a day should be more than enough.
    deleteOldEvents();
    m_deleteOldEventsTimer.setInterval(12 * 60 * 60 * 1000);
    connect(&m_deleteOldEventsTimer, &QTimer::timeout, this, &StatsPlugin::deleteOldEvents);

    // Loading URL filters
    m_urlFilters.clear();

    auto filters = conf.readEntry("url-filters",
                                  QStringList{"about:*", // Ignore about: stuff
                                              "*/.*", // Ignore hidden files
                                              "/", // Ignore root
                                              "/tmp/*"} // Ignore everything in /tmp
    );

    for (const auto &filter : filters) {
        m_urlFilters << Common::starPatternToRegex(filter);
    }

    // Loading the private activities
    m_otrActivities = conf.readEntry("off-the-record-activities", QStringList());
}

void StatsPlugin::deleteOldEvents()
{
    DeleteEarlierStats(QString(), config().readEntry("keep-history-for", 0));
}

void StatsPlugin::openResourceEvent(const QString &usedActivity,
                                    const QString &initiatingAgent,
                                    const QString &targettedResource,
                                    const QDateTime &start,
                                    const QDateTime &end)
{
    Q_ASSERT_X(!initiatingAgent.isEmpty(), "StatsPlugin::openResourceEvent", "Agent should not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(), "StatsPlugin::openResourceEvent", "Activity should not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(), "StatsPlugin::openResourceEvent", "Resource should not be empty");

    detectResourceInfo(targettedResource);

    Utils::prepare(*resourcesDatabase(),
                   openResourceEventQuery,
                   QStringLiteral("INSERT INTO ResourceEvent"
                                  "        (usedActivity,  initiatingAgent,  targettedResource,  start,  end) "
                                  "VALUES (:usedActivity, :initiatingAgent, :targettedResource, :start, :end)"));

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *openResourceEventQuery,
                ":usedActivity",
                usedActivity,
                ":initiatingAgent",
                initiatingAgent,
                ":targettedResource",
                targettedResource,
                ":start",
                start.toSecsSinceEpoch(),
                ":end",
                (end.isNull()) ? QVariant() : end.toSecsSinceEpoch());
}

void StatsPlugin::closeResourceEvent(const QString &usedActivity, const QString &initiatingAgent, const QString &targettedResource, const QDateTime &end)
{
    Q_ASSERT_X(!initiatingAgent.isEmpty(), "StatsPlugin::closeResourceEvent", "Agent should not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(), "StatsPlugin::closeResourceEvent", "Activity should not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(), "StatsPlugin::closeResourceEvent", "Resource should not be empty");

    Utils::prepare(*resourcesDatabase(),
                   closeResourceEventQuery,
                   QStringLiteral("UPDATE ResourceEvent "
                                  "SET end = :end "
                                  "WHERE "
                                  ":usedActivity      = usedActivity AND "
                                  ":initiatingAgent   = initiatingAgent AND "
                                  ":targettedResource = targettedResource AND "
                                  "end IS NULL"));

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *closeResourceEventQuery,
                ":usedActivity",
                usedActivity,
                ":initiatingAgent",
                initiatingAgent,
                ":targettedResource",
                targettedResource,
                ":end",
                end.toSecsSinceEpoch());
}

void StatsPlugin::detectResourceInfo(const QString &_uri)
{
    const QUrl uri = QUrl::fromUserInput(_uri);

    if (!uri.isLocalFile())
        return;

    const QString file = uri.toLocalFile();

    if (!QFile::exists(file))
        return;

    KFileItem item(uri);

    if (insertResourceInfo(file)) {
        saveResourceMimetype(file, item.mimetype(), true);

        const auto text = item.text();
        saveResourceTitle(file, text.isEmpty() ? _uri : text, true);
    }
}

bool StatsPlugin::insertResourceInfo(const QString &uri)
{
    Utils::prepare(*resourcesDatabase(),
                   getResourceInfoQuery,
                   QStringLiteral("SELECT targettedResource FROM ResourceInfo WHERE "
                                  "  targettedResource = :targettedResource "));

    getResourceInfoQuery->bindValue(":targettedResource", uri);
    Utils::exec(*resourcesDatabase(), Utils::FailOnError, *getResourceInfoQuery);

    if (getResourceInfoQuery->next()) {
        return false;
    }

    Utils::prepare(*resourcesDatabase(),
                   insertResourceInfoQuery,
                   QStringLiteral("INSERT INTO ResourceInfo( "
                                  "  targettedResource"
                                  ", title"
                                  ", autoTitle"
                                  ", mimetype"
                                  ", autoMimetype"
                                  ") VALUES ("
                                  "  :targettedResource"
                                  ", '' "
                                  ", 1 "
                                  ", '' "
                                  ", 1 "
                                  ")"));

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, *insertResourceInfoQuery, ":targettedResource", uri);

    return true;
}

void StatsPlugin::saveResourceTitle(const QString &uri, const QString &title, bool autoTitle)
{
    if (m_blockAll || m_whatToRemember == NoApplications) {
        return;
    }

    insertResourceInfo(uri);

    DATABASE_TRANSACTION(*resourcesDatabase());

    Utils::prepare(*resourcesDatabase(),
                   saveResourceTitleQuery,
                   QStringLiteral("UPDATE ResourceInfo SET "
                                  "  title = :title"
                                  ", autoTitle = :autoTitle "
                                  "WHERE "
                                  "targettedResource = :targettedResource "));

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *saveResourceTitleQuery,
                ":targettedResource",
                uri,
                ":title",
                title,
                ":autoTitle",
                (autoTitle ? "1" : "0"));
}

void StatsPlugin::saveResourceMimetype(const QString &uri, const QString &mimetype, bool autoMimetype)
{
    if (m_blockAll || m_whatToRemember == NoApplications) {
        return;
    }

    insertResourceInfo(uri);

    DATABASE_TRANSACTION(*resourcesDatabase());

    Utils::prepare(*resourcesDatabase(),
                   saveResourceMimetypeQuery,
                   QStringLiteral("UPDATE ResourceInfo SET "
                                  "  mimetype = :mimetype"
                                  ", autoMimetype = :autoMimetype "
                                  "WHERE "
                                  "targettedResource = :targettedResource "));

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *saveResourceMimetypeQuery,
                ":targettedResource",
                uri,
                ":mimetype",
                mimetype,
                ":autoMimetype",
                (autoMimetype ? "1" : "0"));
}

StatsPlugin *StatsPlugin::self()
{
    return s_instance;
}

bool StatsPlugin::acceptedEvent(const Event &event)
{
    using std::any_of;
    using std::bind;
    using namespace std::placeholders;

    return !(
        // If the URI is empty, we do not want to process it
        event.uri.isEmpty() ||

        // Skip if the current activity is OTR
        m_otrActivities.contains(currentActivity()) ||

        // Exclude URIs that match the ignored patterns
        any_of(m_urlFilters.cbegin(), m_urlFilters.cend(), [event] (const QRegularExpression &regex){ return regex.match(event.uri).hasMatch(); }) ||

        // if blocked by default, the list contains allowed applications
        //     ignore event if the list doesn't contain the application
        // if not blocked by default, the list contains blocked applications
        //     ignore event if the list contains the application
        (m_whatToRemember == SpecificApplications && m_blockedByDefault != boost::binary_search(m_apps, event.application)));
}

Event StatsPlugin::validateEvent(Event event)
{
    if (event.uri.startsWith(QStringLiteral("file://"))) {
        event.uri = QUrl(event.uri).toLocalFile();
    }

    if (event.uri.startsWith(QStringLiteral("/"))) {
        QFileInfo file(event.uri);

        event.uri = file.exists() ? file.canonicalFilePath() : QString();
    }

    return event;
}

QStringList StatsPlugin::listActivities() const
{
    return Plugin::retrieve<QStringList>(m_activities, "ListActivities");
}

QString StatsPlugin::currentActivity() const
{
    return Plugin::retrieve<QString>(m_activities, "CurrentActivity");
}

void StatsPlugin::addEvents(const EventList &events)
{
    using namespace kamd::utils;

    if (m_blockAll || m_whatToRemember == NoApplications) {
        return;
    }

    const auto &eventsToProcess = events | transformed(&StatsPlugin::validateEvent, this) | filtered(&StatsPlugin::acceptedEvent, this);

    if (eventsToProcess.begin() == eventsToProcess.end())
        return;

    DATABASE_TRANSACTION(*resourcesDatabase());

    for (const auto &event : eventsToProcess) {
        switch (event.type) {
        case Event::Accessed:
            openResourceEvent(currentActivity(), event.application, event.uri, event.timestamp, event.timestamp);
            ResourceScoreMaintainer::self()->processResource(event.uri, event.application);

            break;

        case Event::Opened:
            openResourceEvent(currentActivity(), event.application, event.uri, event.timestamp);

            break;

        case Event::Closed:
            closeResourceEvent(currentActivity(), event.application, event.uri, event.timestamp);
            ResourceScoreMaintainer::self()->processResource(event.uri, event.application);

            break;

        case Event::UserEventType:
            ResourceScoreMaintainer::self()->processResource(event.uri, event.application);
            break;

        default:
            // Nothing yet
            // TODO: Add focus and modification
            break;
        }
    }
}

void StatsPlugin::DeleteRecentStats(const QString &activity, int count, const QString &what)
{
    const auto usedActivity = activity.isEmpty() ? QVariant() : QVariant(activity);

    // If we need to delete everything,
    // no need to bother with the count and the date

    DATABASE_TRANSACTION(*resourcesDatabase());

    if (what == QStringLiteral("everything")) {
        // Instantiating these every time is not a big overhead
        // since this method is rarely executed.

        auto removeResourceInfoQuery = resourcesDatabase()->createQuery();
        removeResourceInfoQuery.prepare(
            "DELETE FROM ResourceInfo "
            "WHERE ResourceInfo.targettedResource IN ("
            "   SELECT ResourceEvent.targettedResource "
            "   FROM ResourceEvent "
            "   WHERE usedActivity = COALESCE(:usedActivity, usedActivity)"
            ")");

        auto removeEventsQuery = resourcesDatabase()->createQuery();
        removeEventsQuery.prepare(
            "DELETE FROM ResourceEvent "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity)");

        auto removeScoreCachesQuery = resourcesDatabase()->createQuery();
        removeScoreCachesQuery.prepare(
            "DELETE FROM ResourceScoreCache "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity)");

        Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeResourceInfoQuery, ":usedActivity", usedActivity);
        Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeEventsQuery, ":usedActivity", usedActivity);
        Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeScoreCachesQuery, ":usedActivity", usedActivity);

    } else {
        // Deleting a specified length of time

        auto since = QDateTime::currentDateTime();

        since = (what[0] == QLatin1Char('h')) ? since.addSecs(-count * 60 * 60)
            : (what[0] == QLatin1Char('d'))   ? since.addDays(-count)
            : (what[0] == QLatin1Char('m'))   ? since.addMonths(-count)
                                              : since;

        // Maybe we should decrease the scores for the previously
        // cached items. Thinking it is not that important -
        // if something was accessed before, and the user did not
        // remove the history, it is not really a secret.

        auto removeResourceInfoQuery = resourcesDatabase()->createQuery();
        removeResourceInfoQuery.prepare(
            "DELETE FROM ResourceInfo "
            "WHERE ResourceInfo.targettedResource IN ("
            "   SELECT ResourceEvent.targettedResource "
            "   FROM ResourceEvent "
            "   WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "   AND end > :since"
            ")");

        auto removeEventsQuery = resourcesDatabase()->createQuery();
        removeEventsQuery.prepare(
            "DELETE FROM ResourceEvent "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "AND end > :since");

        auto removeScoreCachesQuery = resourcesDatabase()->createQuery();
        removeScoreCachesQuery.prepare(
            "DELETE FROM ResourceScoreCache "
            "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
            "AND firstUpdate > :since");

        Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeResourceInfoQuery, ":usedActivity", usedActivity, ":since", since.toSecsSinceEpoch());

        Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeEventsQuery, ":usedActivity", usedActivity, ":since", since.toSecsSinceEpoch());

        Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeScoreCachesQuery, ":usedActivity", usedActivity, ":since", since.toSecsSinceEpoch());
    }

    Q_EMIT RecentStatsDeleted(activity, count, what);
}

void StatsPlugin::DeleteEarlierStats(const QString &activity, int months)
{
    if (months == 0) {
        return;
    }

    // Deleting a specified length of time

    DATABASE_TRANSACTION(*resourcesDatabase());

    const auto time = QDateTime::currentDateTime().addMonths(-months);
    const auto usedActivity = activity.isEmpty() ? QVariant() : QVariant(activity);

    auto removeResourceInfoQuery = resourcesDatabase()->createQuery();
    removeResourceInfoQuery.prepare(
        "DELETE FROM ResourceInfo "
        "WHERE ResourceInfo.targettedResource IN ("
        "   SELECT ResourceEvent.targettedResource "
        "   FROM ResourceEvent "
        "   WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
        "   AND start < :time"
        ")");

    auto removeEventsQuery = resourcesDatabase()->createQuery();
    removeEventsQuery.prepare(
        "DELETE FROM ResourceEvent "
        "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
        "AND start < :time");

    auto removeScoreCachesQuery = resourcesDatabase()->createQuery();
    removeScoreCachesQuery.prepare(
        "DELETE FROM ResourceScoreCache "
        "WHERE usedActivity = COALESCE(:usedActivity, usedActivity) "
        "AND lastUpdate < :time");

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeResourceInfoQuery, ":usedActivity", usedActivity, ":time", time.toSecsSinceEpoch());

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeEventsQuery, ":usedActivity", usedActivity, ":time", time.toSecsSinceEpoch());

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeScoreCachesQuery, ":usedActivity", usedActivity, ":time", time.toSecsSinceEpoch());

    Q_EMIT EarlierStatsDeleted(activity, months);
}

void StatsPlugin::DeleteStatsForResource(const QString &activity, const QString &client, const QString &resource)
{
    Q_ASSERT_X(!client.isEmpty(), "StatsPlugin::DeleteStatsForResource", "Agent should not be empty");
    Q_ASSERT_X(!activity.isEmpty(), "StatsPlugin::DeleteStatsForResource", "Activity should not be empty");
    Q_ASSERT_X(!resource.isEmpty(), "StatsPlugin::DeleteStatsForResource", "Resource should not be empty");
    Q_ASSERT_X(client != CURRENT_AGENT_TAG, "StatsPlugin::DeleteStatsForResource", "We can not handle CURRENT_AGENT_TAG here");

    DATABASE_TRANSACTION(*resourcesDatabase());

    // Check against sql injection
    if (activity.contains('\'') || client.contains('\''))
        return;

    const auto activityFilter =
        activity == ANY_ACTIVITY_TAG ? " 1 " : QStringLiteral(" usedActivity = '%1' ").arg(activity == CURRENT_ACTIVITY_TAG ? currentActivity() : activity);

    const auto clientFilter = client == ANY_AGENT_TAG ? " 1 " : QStringLiteral(" initiatingAgent = '%1' ").arg(client);

    auto removeResourceInfoQuery = resourcesDatabase()->createQuery();
    removeResourceInfoQuery.prepare(
        "DELETE FROM ResourceInfo "
        "WHERE targettedResource LIKE :targettedResource ESCAPE '\\'");

    auto removeEventsQuery = resourcesDatabase()->createQuery();
    removeEventsQuery.prepare(
        "DELETE FROM ResourceEvent "
        "WHERE "
        + activityFilter + " AND " + clientFilter + " AND " + "targettedResource LIKE :targettedResource ESCAPE '\\'");

    auto removeScoreCachesQuery = resourcesDatabase()->createQuery();
    removeScoreCachesQuery.prepare(
        "DELETE FROM ResourceScoreCache "
        "WHERE "
        + activityFilter + " AND " + clientFilter + " AND " + "targettedResource LIKE :targettedResource ESCAPE '\\'");

    const auto pattern = Common::starPatternToLike(resource);

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeResourceInfoQuery, ":targettedResource", pattern);

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeEventsQuery, ":targettedResource", pattern);

    Utils::exec(*resourcesDatabase(), Utils::FailOnError, removeScoreCachesQuery, ":targettedResource", pattern);

    Q_EMIT ResourceScoreDeleted(activity, client, resource);
}

bool StatsPlugin::isFeatureOperational(const QStringList &feature) const
{
    if (feature[0] == "isOTR") {
        if (feature.size() != 2)
            return true;

        const auto activity = feature[1];

        return activity == "activity" //
            || activity == "current" //
            || listActivities().contains(activity);

        return true;
    }

    return false;
}

// bool StatsPlugin::isFeatureEnabled(const QStringList &feature) const
// {
//     if (feature[0] == "isOTR") {
//         if (feature.size() != 2) return false;
//
//         auto activity = feature[1];
//
//         if (activity == "activity" || activity == "current") {
//             activity = currentActivity();
//         }
//
//         return m_otrActivities.contains(activity);
//     }
//
//     return false;
// }
//
// void StatsPlugin::setFeatureEnabled(const QStringList &feature, bool value)
// {
//     if (feature[0] == "isOTR") {
//         if (feature.size() != 2) return;
//
//         auto activity = feature[1];
//
//         if (activity == "activity" || activity == "current") {
//             activity = currentActivity();
//         }
//
//         if (!m_otrActivities.contains(activity)) {
//             m_otrActivities << activity;
//             config().writeEntry("off-the-record-activities", m_otrActivities);
//             config().sync();
//         }
//     }
// }

QDBusVariant StatsPlugin::featureValue(const QStringList &feature) const
{
    if (feature[0] == "isOTR") {
        if (feature.size() != 2)
            return QDBusVariant(false);

        auto activity = feature[1];

        if (activity == "activity" || activity == "current") {
            activity = currentActivity();
        }

        return QDBusVariant(m_otrActivities.contains(activity));
    }

    return QDBusVariant(false);
}

void StatsPlugin::setFeatureValue(const QStringList &feature, const QDBusVariant &value)
{
    if (feature[0] == "isOTR") {
        if (feature.size() != 2)
            return;

        auto activity = feature[1];

        if (activity == "activity" || activity == "current") {
            activity = currentActivity();
        }

        bool isOTR = value.variant().toBool();

        if (isOTR && !m_otrActivities.contains(activity)) {
            m_otrActivities << activity;

        } else if (!isOTR && m_otrActivities.contains(activity)) {
            m_otrActivities.removeAll(activity);
        }

        config().writeEntry("off-the-record-activities", m_otrActivities);
        config().sync();
    }
}

QStringList StatsPlugin::listFeatures(const QStringList &feature) const
{
    if (feature.isEmpty() || feature[0].isEmpty()) {
        return {"isOTR/"};

    } else if (feature[0] == "isOTR") {
        return listActivities();
    }

    return QStringList();
}

#include "StatsPlugin.moc"

#include "moc_StatsPlugin.cpp"
