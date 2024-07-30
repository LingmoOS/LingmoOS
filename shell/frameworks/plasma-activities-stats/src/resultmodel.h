/*
    SPDX-FileCopyrightText: 2015, 2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KACTIVITIES_STATS_RESULTMODEL_H
#define KACTIVITIES_STATS_RESULTMODEL_H

// Qt
#include <QAbstractListModel>
#include <QObject>

// Local
#include "query.h"

class QModelIndex;
class QDBusPendingCallWatcher;

class KConfigGroup;

namespace KActivities
{
namespace Stats
{
class ResultModelPrivate;

/**
 * @class KActivities::Stats::ResultModel resultmodel.h <KActivities/Stats/ResultModel>
 *
 * Provides a model which displays the resources matching
 * the specified Query.
 */
class PLASMAACTIVITIESSTATS_EXPORT ResultModel : public QAbstractListModel
{
    Q_OBJECT

public:
    ResultModel(Query query, QObject *parent = nullptr);
    ResultModel(Query query, const QString &clientId, QObject *parent = nullptr);
    ~ResultModel() override;

    enum Roles {
        ResourceRole = Qt::UserRole,
        TitleRole = Qt::UserRole + 1,
        ScoreRole = Qt::UserRole + 2,
        FirstUpdateRole = Qt::UserRole + 3,
        LastUpdateRole = Qt::UserRole + 4,
        LinkStatusRole = Qt::UserRole + 5,
        LinkedActivitiesRole = Qt::UserRole + 6,
        MimeType = Qt::UserRole + 7, // @since 5.77
        Agent = Qt::UserRole + 8, // @since 6.0
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &item, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void fetchMore(const QModelIndex &parent) override;
    bool canFetchMore(const QModelIndex &parent) const override;

    void linkToActivity(const QUrl &resource,
                        const Terms::Activity &activity = Terms::Activity(QStringList()),
                        const Terms::Agent &agent = Terms::Agent(QStringList()));

    void unlinkFromActivity(const QUrl &resource,
                            const Terms::Activity &activity = Terms::Activity(QStringList()),
                            const Terms::Agent &agent = Terms::Agent(QStringList()));

public Q_SLOTS:
    /**
     * Removes the specified resource from the history
     */
    void forgetResource(const QString &resource);

    /**
     * Removes specified list of resources from the history
     */
    void forgetResources(const QList<QString> &resources);

    /**
     * Removes the specified resource from the history
     */
    void forgetResource(int row);

    /**
     * Clears the history of all resources that match the current
     * model query
     */
    void forgetAllResources();

    /**
     * Moves the resource to the specified position.
     *
     * Note that this only applies to the linked resources
     * since the recently/frequently used ones have
     * their natural order.
     *
     * @note This requires the clientId to be specified on construction.
     */
    void setResultPosition(const QString &resource, int position);

    /**
     * Sort the items by title.
     *
     * Note that this only affects the linked resources
     * since the recently/frequently used ones have
     * their natural order.
     *
     * @note This requires the clientId to be specified on construction.
     */
    void sortItems(Qt::SortOrder sortOrder);

private:
    friend class ResultModelPrivate;
    ResultModelPrivate *const d;
};

} // namespace Stats
} // namespace KActivities

#endif // KACTIVITIES_STATS_RESULTMODEL_H
