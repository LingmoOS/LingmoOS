/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_RESULTWATCHER
#define KACTIVITIES_STATS_RESULTWATCHER

#include <QObject>

#include "query.h"
#include "resultset.h"

namespace KActivities
{
namespace Stats
{
class ResultWatcherPrivate;

/**
 * @class KActivities::Stats::ResultWatcher resultwatcher.h <KActivities/Stats/ResultWatcher>
 *
 * A very thin class that sends signals when new resources matching
 * a predefined query are available.
 */
class PLASMAACTIVITIESSTATS_EXPORT ResultWatcher : public QObject
{
    Q_OBJECT

public:
    explicit ResultWatcher(Query query, QObject *parent = nullptr);
    ~ResultWatcher() override;

Q_SIGNALS:
    /**
     * Emitted when a result has been added or updated. This either means
     * a new resource has appeared in the result set, or that
     * a previously existing one has some of the attributes changed.
     * @param result new data for the resource defined by result.resource
     */
    void resultScoreUpdated(const QString &resource, double score, uint lastUpdate, uint firstUpdate);

    /**
     * Emitted when a result has been added or updated. This either means
     * a new resource has appeared in the result set, or that
     * a previously existing one has some of the attributes changed.
     * @param result new data for the resource defined by result.resource
     */
    void resultRemoved(const QString &resource);

    /**
     * Emitted when a result has been linked to the activity
     */
    void resultLinked(const QString &resource);

    /**
     * Emitted when a result has been unlinked from the activity
     */
    void resultUnlinked(const QString &resource);

    /**
     * Emitted when the title of a resource has been changed.
     * @param resource URL of the resource that has a new title
     * @param title new title of the resource
     * @note This signal will be emitted even for the resources that
     * do not match the specified query. This is because the class is
     * lightweight, and it does not keep track of which resources match
     * the query to be able to filter this signal.
     */
    void resourceTitleChanged(const QString &resource, const QString &title);

    /**
     * Emitted when the mimetype of a resource has been changed.
     * @param resource URL of the resource that has a new mimetype
     * @param mimetype new mimetype of the resource
     * @note This signal will be emitted even for the resources that
     * do not match the specified query. This is because the class is
     * lightweight, and it does not keep track of which resources match
     * the query to be able to filter this signal.
     */
    void resourceMimetypeChanged(const QString &resource, const QString &mimetype);

    /**
     * Emitted when the client should forget about all the results it
     * knew about and reload them. This can happen when the user clears
     * the history, or when there are more significant changes to the data.
     */
    void resultsInvalidated();

public:
    void linkToActivity(const QUrl &resource,
                        const Terms::Activity &activity = Terms::Activity(QStringList()),
                        const Terms::Agent &agent = Terms::Agent(QStringList()));

    void unlinkFromActivity(const QUrl &resource,
                            const Terms::Activity &activity = Terms::Activity(QStringList()),
                            const Terms::Agent &agent = Terms::Agent(QStringList()));

private:
    ResultWatcherPrivate *const d;
};

} // namespace Stats
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTWATCHER
