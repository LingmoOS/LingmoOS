/*
 *   SPDX-FileCopyrightText: 2011, 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

// Qt
#include <QObject>
#include <QSqlQuery>
#include <QTimer>
#include <QRegularExpression>
// STL
#include <memory>

// Local
#include <Plugin.h>

class ResourceLinking;

/**
 * Communication with the outer world.
 *
 * - Handles configuration
 * - Filters the events based on the user's configuration.
 */
class StatsPlugin : public Plugin
{
    Q_OBJECT
    // Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources.Scoring")

public:
    explicit StatsPlugin(QObject *parent);

    static StatsPlugin *self();

    bool init(QHash<QString, QObject *> &modules) override;

    QString currentActivity() const;
    QStringList listActivities() const;

    inline QObject *activitiesInterface() const
    {
        return m_activities;
    }

    bool isFeatureOperational(const QStringList &feature) const override;
    QStringList listFeatures(const QStringList &feature) const override;

    QDBusVariant featureValue(const QStringList &property) const override;
    void setFeatureValue(const QStringList &property, const QDBusVariant &value) override;

    //
    // D-BUS Interface methods
    //

public Q_SLOTS:
    void DeleteRecentStats(const QString &activity, int count, const QString &what);

    void DeleteEarlierStats(const QString &activity, int months);

    void DeleteStatsForResource(const QString &activity, const QString &client, const QString &resource);

Q_SIGNALS:
    void ResourceScoreUpdated(const QString &activity, const QString &client, const QString &resource, double score, uint lastUpdate, uint firstUpdate);
    void ResourceScoreDeleted(const QString &activity, const QString &client, const QString &resource);

    void RecentStatsDeleted(const QString &activity, int count, const QString &what);

    void EarlierStatsDeleted(const QString &activity, int months);

    //
    // End D-BUS Interface methods
    //

private Q_SLOTS:
    void addEvents(const EventList &events);
    void loadConfiguration();

    void openResourceEvent(const QString &usedActivity,
                           const QString &initiatingAgent,
                           const QString &targettedResource,
                           const QDateTime &start,
                           const QDateTime &end = QDateTime());

    void closeResourceEvent(const QString &usedActivity, const QString &initiatingAgent, const QString &targettedResource, const QDateTime &end);

    void saveResourceTitle(const QString &uri, const QString &title, bool autoTitle = false);
    void saveResourceMimetype(const QString &uri, const QString &mimetype, bool autoMimetype = false);
    bool insertResourceInfo(const QString &uri);
    void detectResourceInfo(const QString &uri);

    void deleteOldEvents();

private:
    inline bool acceptedEvent(const Event &event);
    inline Event validateEvent(Event event);

    enum WhatToRemember {
        AllApplications = 0,
        SpecificApplications = 1,
        NoApplications = 2,
    };

    QObject *m_activities;
    QObject *m_resources;

    QSet<QString> m_apps;
    QList<QRegularExpression> m_urlFilters;
    QStringList m_otrActivities;

    std::unique_ptr<QSqlQuery> openResourceEventQuery;
    std::unique_ptr<QSqlQuery> closeResourceEventQuery;

    std::unique_ptr<QSqlQuery> insertResourceInfoQuery;
    std::unique_ptr<QSqlQuery> getResourceInfoQuery;
    std::unique_ptr<QSqlQuery> saveResourceTitleQuery;
    std::unique_ptr<QSqlQuery> saveResourceMimetypeQuery;

    QTimer m_deleteOldEventsTimer;

    bool m_blockedByDefault : 1;
    bool m_blockAll : 1;
    WhatToRemember m_whatToRemember : 2;

    ResourceLinking *m_resourceLinking;

    static StatsPlugin *s_instance;
};
