/*
 *   SPDX-FileCopyrightText: 2011, 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

// Self
#include "ResourceLinking.h"
#include <kactivities-features.h>

// Qt
#include <QDBusConnection>
#include <QFileSystemWatcher>
#include <QSqlQuery>

// KDE
#include <kconfig.h>
#include <kdirnotify.h>

// Local
#include "Database.h"
#include "DebugResources.h"
#include "StatsPlugin.h"
#include "Utils.h"
#include "resourcelinkingadaptor.h"

ResourceLinking::ResourceLinking(QObject *parent)
    : QObject(parent)
{
    new ResourcesLinkingAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/ActivityManager/Resources/Linking"), this);
}

void ResourceLinking::init()
{
    auto activities = StatsPlugin::self()->activitiesInterface();

    connect(activities, SIGNAL(CurrentActivityChanged(QString)), this, SLOT(onCurrentActivityChanged(QString)));
    connect(activities, SIGNAL(ActivityAdded(QString)), this, SLOT(onActivityAdded(QString)));
    connect(activities, SIGNAL(ActivityRemoved(QString)), this, SLOT(onActivityRemoved(QString)));
}

void ResourceLinking::LinkResourceToActivity(QString initiatingAgent, QString targettedResource, QString usedActivity)
{
    qCDebug(KAMD_LOG_RESOURCES) << "Linking " << targettedResource << " to " << usedActivity << " from " << initiatingAgent;

    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        qCWarning(KAMD_LOG_RESOURCES) << "Invalid arguments" << initiatingAgent << targettedResource << usedActivity;
        return;
    }

    if (usedActivity == ":any") {
        usedActivity = ":global";
    }

    Q_ASSERT_X(!initiatingAgent.isEmpty(), "ResourceLinking::LinkResourceToActivity", "Agent should not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(), "ResourceLinking::LinkResourceToActivity", "Activity should not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(), "ResourceLinking::LinkResourceToActivity", "Resource should not be empty");

    Utils::prepare(*resourcesDatabase(),
                   linkResourceToActivityQuery,
                   QStringLiteral("INSERT OR REPLACE INTO ResourceLink"
                                  "        (usedActivity,  initiatingAgent,  targettedResource) "
                                  "VALUES ( "
                                  "COALESCE(:usedActivity,''),"
                                  "COALESCE(:initiatingAgent,''),"
                                  "COALESCE(:targettedResource,'')"
                                  ")"));

    DATABASE_TRANSACTION(*resourcesDatabase());

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *linkResourceToActivityQuery,
                ":usedActivity",
                usedActivity,
                ":initiatingAgent",
                initiatingAgent,
                ":targettedResource",
                targettedResource);

    if (!usedActivity.isEmpty()) {
        // qCDebug(KAMD_LOG_RESOURCES) << "Sending link event added: activities:/" << usedActivity;
        org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("activities:/") + usedActivity));

        if (usedActivity == StatsPlugin::self()->currentActivity()) {
            // qCDebug(KAMD_LOG_RESOURCES) << "Sending link event added: activities:/current";
            org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("activities:/current")));
        }
    }

    Q_EMIT ResourceLinkedToActivity(initiatingAgent, targettedResource, usedActivity);
}

void ResourceLinking::UnlinkResourceFromActivity(QString initiatingAgent, QString targettedResource, QString usedActivity)
{
    // qCDebug(KAMD_LOG_RESOURCES) << "Unlinking " << targettedResource << " from " << usedActivity << " from " << initiatingAgent;

    if (!validateArguments(initiatingAgent, targettedResource, usedActivity, false)) {
        qCWarning(KAMD_LOG_RESOURCES) << "Invalid arguments" << initiatingAgent << targettedResource << usedActivity;
        return;
    }

    Q_ASSERT_X(!initiatingAgent.isEmpty(), "ResourceLinking::UnlinkResourceFromActivity", "Agent should not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(), "ResourceLinking::UnlinkResourceFromActivity", "Activity should not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(), "ResourceLinking::UnlinkResourceFromActivity", "Resource should not be empty");

    QSqlQuery *query = nullptr;

    if (usedActivity == ":any") {
        Utils::prepare(*resourcesDatabase(),
                       unlinkResourceFromAllActivitiesQuery,
                       QStringLiteral("DELETE FROM ResourceLink "
                                      "WHERE "
                                      "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
                                      "(targettedResource = COALESCE(:targettedResource, '') OR "
                                      "(initiatingAgent = 'org.kde.plasma.favorites.applications' "
                                      "AND targettedResource = 'applications:' || COALESCE(:targettedResource, '')))"));
        query = unlinkResourceFromAllActivitiesQuery.get();
    } else {
        Utils::prepare(*resourcesDatabase(),
                       unlinkResourceFromActivityQuery,
                       QStringLiteral("DELETE FROM ResourceLink "
                                      "WHERE "
                                      "usedActivity      = COALESCE(:usedActivity     , '') AND "
                                      "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
                                      "(targettedResource = COALESCE(:targettedResource, '') OR "
                                      "(initiatingAgent = 'org.kde.plasma.favorites.applications'"
                                      "AND targettedResource =  'applications:' || COALESCE(:targettedResource, '')))"));
        query = unlinkResourceFromActivityQuery.get();
    }

    DATABASE_TRANSACTION(*resourcesDatabase());
    // BUG 385814, some existing entries don't have the applications:
    // prefix, so we remove it and check in the sql if they match
    // TODO Remove when we can expect all users to have a fresher install than 5.18
    if (initiatingAgent == QLatin1String("org.kde.plasma.favorites.applications")) {
        targettedResource = targettedResource.remove(QLatin1String("applications:"));
    }
    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *query,
                ":usedActivity",
                usedActivity,
                ":initiatingAgent",
                initiatingAgent,
                ":targettedResource",
                targettedResource);

    if (!usedActivity.isEmpty()) {
        // auto mangled = QString::fromUtf8(QUrl::toPercentEncoding(targettedResource));
        auto mangled = QString::fromLatin1(targettedResource.toUtf8().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

        // qCDebug(KAMD_LOG_RESOURCES) << "Sending link event removed: activities:/" << usedActivity << '/' << mangled;
        org::kde::KDirNotify::emitFilesRemoved({QUrl(QStringLiteral("activities:/") + usedActivity + '/' + mangled)});

        if (usedActivity == StatsPlugin::self()->currentActivity()) {
            // qCDebug(KAMD_LOG_RESOURCES) << "Sending link event removed: activities:/current/" << mangled;
            org::kde::KDirNotify::emitFilesRemoved({QUrl(QStringLiteral("activities:/current/") + mangled)});
        }
    }

    Q_EMIT ResourceUnlinkedFromActivity(initiatingAgent, targettedResource, usedActivity);
}

bool ResourceLinking::IsResourceLinkedToActivity(QString initiatingAgent, QString targettedResource, QString usedActivity)
{
    if (!validateArguments(initiatingAgent, targettedResource, usedActivity)) {
        return false;
    }

    Q_ASSERT_X(!initiatingAgent.isEmpty(), "ResourceLinking::IsResourceLinkedToActivity", "Agent should not be empty");
    Q_ASSERT_X(!usedActivity.isEmpty(), "ResourceLinking::IsResourceLinkedToActivity", "Activity should not be empty");
    Q_ASSERT_X(!targettedResource.isEmpty(), "ResourceLinking::IsResourceLinkedToActivity", "Resource should not be empty");

    Utils::prepare(*resourcesDatabase(),
                   isResourceLinkedToActivityQuery,
                   QStringLiteral("SELECT * FROM ResourceLink "
                                  "WHERE "
                                  "usedActivity      = COALESCE(:usedActivity     , '') AND "
                                  "initiatingAgent   = COALESCE(:initiatingAgent  , '') AND "
                                  "targettedResource = COALESCE(:targettedResource, '') "));

    Utils::exec(*resourcesDatabase(),
                Utils::FailOnError,
                *isResourceLinkedToActivityQuery,
                ":usedActivity",
                usedActivity,
                ":initiatingAgent",
                initiatingAgent,
                ":targettedResource",
                targettedResource);

    return isResourceLinkedToActivityQuery->next();
}

bool ResourceLinking::validateArguments(QString &initiatingAgent, QString &targettedResource, QString &usedActivity, bool checkFilesExist)
{
    // Validating targetted resource
    if (targettedResource.isEmpty()) {
        qCDebug(KAMD_LOG_RESOURCES) << "Resource is invalid -- empty";
        return false;
    }

    if (targettedResource.startsWith(QStringLiteral("file://"))) {
        targettedResource = QUrl(targettedResource).toLocalFile();
    }

    if (targettedResource.startsWith(QStringLiteral("/")) && checkFilesExist) {
        QFileInfo file(targettedResource);

        if (!file.exists()) {
            qCDebug(KAMD_LOG_RESOURCES) << "Resource is invalid -- the file does not exist";
            return false;
        }

        targettedResource = file.canonicalFilePath();
    }

    // Handling special values for the agent
    if (initiatingAgent.isEmpty()) {
        initiatingAgent = ":global";
    }

    // Handling special values for activities
    if (usedActivity == ":current") {
        usedActivity = StatsPlugin::self()->currentActivity();

    } else if (usedActivity.isEmpty()) {
        usedActivity = ":global";
    }

    // If the activity is not empty and the passed activity
    // does not exist, cancel the request
    if (!usedActivity.isEmpty() && usedActivity != ":global" && usedActivity != ":any" && !StatsPlugin::self()->listActivities().contains(usedActivity)) {
        qCDebug(KAMD_LOG_RESOURCES) << "Activity is invalid, it does not exist";
        return false;
    }

    // qCDebug(KAMD_LOG_RESOURCES) << "agent" << initiatingAgent
    //                             << "resource" << targettedResource
    //                             << "activity" << usedActivity;

    return true;
}

void ResourceLinking::onActivityAdded(const QString &activity)
{
    Q_UNUSED(activity);

    // Notify KIO
    // qCDebug(KAMD_LOG_RESOURCES) << "Added: activities:/  (" << activity << ")";
    org::kde::KDirNotify::emitFilesAdded(QUrl(QStringLiteral("activities:/")));
}

void ResourceLinking::onActivityRemoved(const QString &activity)
{
    // Notify KIO
    // qCDebug(KAMD_LOG_RESOURCES) << "Removed: activities:/" << activity;
    org::kde::KDirNotify::emitFilesRemoved({QUrl(QStringLiteral("activities:/") + activity)});

    // Remove statistics for the activity
}

void ResourceLinking::onCurrentActivityChanged(const QString &activity)
{
    Q_UNUSED(activity);

    // Notify KIO
    // qCDebug(KAMD_LOG_RESOURCES) << "Changed: activities:/current -> " << activity;
    org::kde::KDirNotify::emitFilesAdded({QUrl(QStringLiteral("activities:/current"))});
}

#include "moc_ResourceLinking.cpp"
