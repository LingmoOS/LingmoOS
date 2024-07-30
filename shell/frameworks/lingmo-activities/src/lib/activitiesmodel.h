/*
    SPDX-FileCopyrightText: 2012, 2013, 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ACTIVITIES_ACTIVITIESMODEL_H
#define ACTIVITIES_ACTIVITIESMODEL_H

// Qt
#include <QAbstractListModel>
#include <QObject>

// STL
#include <memory>

// Local
#include "info.h"

class QModelIndex;
class QDBusPendingCallWatcher;

namespace KActivities
{
class ActivitiesModelPrivate;

/**
 * Data model that shows existing activities
 */
class LINGMO_ACTIVITIES_EXPORT ActivitiesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QList<Info::State> shownStates READ shownStates WRITE setShownStates NOTIFY shownStatesChanged)

public:
    explicit ActivitiesModel(QObject *parent = nullptr);

    /**
     * Constructs the model and sets the shownStates
     */
    ActivitiesModel(QList<Info::State> shownStates, QObject *parent = nullptr);
    ~ActivitiesModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    QHash<int, QByteArray> roleNames() const override;

    enum Roles {
        ActivityId = Qt::UserRole, ///< UUID of the activity
        ActivityName = Qt::UserRole + 1, ///< Activity name
        ActivityDescription = Qt::UserRole + 2, ///< Activity description
        ActivityIconSource = Qt::UserRole + 3, ///< Activity icon source name
        ActivityState = Qt::UserRole + 4, ///< The current state of the activity @see Info::State
        ActivityBackground = Qt::UserRole + 5, ///< Activity wallpaper (currently unsupported)
        ActivityIsCurrent = Qt::UserRole + 6, ///< Is this activity the current one current

        UserRole = Qt::UserRole + 32, ///< To be used by models that inherit this one
    };

public Q_SLOTS:
    /**
     * The model can filter the list of activities based on their state.
     * This method sets which states should be shown.
     */
    void setShownStates(const QList<Info::State> &shownStates);

    /**
     * The model can filter the list of activities based on their state.
     * This method returns which states are currently shown.
     */
    QList<Info::State> shownStates() const;

Q_SIGNALS:
    void shownStatesChanged(const QList<Info::State> &state);

private:
    friend class ActivitiesModelPrivate;
    const std::unique_ptr<ActivitiesModelPrivate> d;
};

} // namespace KActivities

#endif // ACTIVITIES_ACTIVITIESMODEL_H
