/*
 *   SPDX-FileCopyrightText: 2011, 2012, 2013, 2014, 2015 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

// Qt
#include <QObject>

// STL
#include <memory>

// Local
#include <Plugin.h>

class QSqlQuery;

/**
 * Communication with the outer world.
 *
 * - Handles configuration
 * - Filters the events based on the user's configuration.
 */
class ResourceLinking : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.ActivityManager.Resources.Linking")

public:
    explicit ResourceLinking(QObject *parent);

    void init();

public Q_SLOTS:
    /**
     * Links the resource to the activity
     * @param initiatingAgent application that requests the linking. Leave
     *     empty if the resource should be linked to the activity regardless
     *     of which application asks for it.
     * @param targettedResource resource to link to the activity. Can be a file
     *     or any other kind of URI. If it is not a globally recognizable URI,
     *     you should set the initiatingAgent to a specific application.
     * @param usedActivity Activity to link to. Leave empty to link to all
     *     activities.
     */
    void LinkResourceToActivity(QString initiatingAgent, QString targettedResource, QString usedActivity = QString());
    void UnlinkResourceFromActivity(QString initiatingAgent, QString targettedResource, QString usedActivity = QString());
    bool IsResourceLinkedToActivity(QString initiatingAgent, QString targettedResource, QString usedActivity = QString());

Q_SIGNALS:
    void ResourceLinkedToActivity(const QString &initiatingAgent, const QString &targettedResource, const QString &usedActivity);
    void ResourceUnlinkedFromActivity(const QString &initiatingAgent, const QString &targettedResource, const QString &usedActivity);

private Q_SLOTS:
    void onActivityAdded(const QString &activity);
    void onActivityRemoved(const QString &activity);
    void onCurrentActivityChanged(const QString &activity);

private:
    bool validateArguments(QString &initiatingAgent, QString &targettedResource, QString &usedActivity, bool checkFilesExist = true);

    QString currentActivity() const;

    std::unique_ptr<QSqlQuery> linkResourceToActivityQuery;
    std::unique_ptr<QSqlQuery> unlinkResourceFromAllActivitiesQuery;
    std::unique_ptr<QSqlQuery> unlinkResourceFromActivityQuery;
    std::unique_ptr<QSqlQuery> isResourceLinkedToActivityQuery;
};
