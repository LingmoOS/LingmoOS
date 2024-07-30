/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

// Self
#include "activitiesmodel.h"
#include "activitiesmodel_p.h"

// Qt
#include <QByteArray>
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDebug>
#include <QFutureWatcher>
#include <QHash>
#include <QModelIndex>

// Local
#include "utils/remove_if.h"

namespace KActivities
{
namespace Private
{
template<typename _Container>
struct ActivityPosition {
    ActivityPosition()
        : isValid(false)
        , index(0)
        , iterator()
    {
    }

    ActivityPosition(unsigned int index, typename _Container::const_iterator iterator)
        : isValid(true)
        , index(index)
        , iterator(iterator)
    {
    }

    operator bool() const
    {
        return isValid;
    }

    const bool isValid;
    const unsigned int index;
    const typename _Container::const_iterator iterator;

    typedef typename _Container::value_type ContainerElement;
};

/**
 * Returns whether the activity has a desired state.
 * If the state is 0, returns true
 */
template<typename T>
inline bool matchingState(ActivitiesModelPrivate::InfoPtr activity, const T &states)
{
    return states.empty() || states.contains(activity->state());
}

/**
 * Searches for the activity.
 * Returns an option(index, iterator) for the found activity.
 */
template<typename _Container>
inline ActivityPosition<_Container> activityPosition(const _Container &container, const QString &activityId)
{
    auto position = std::find_if(container.begin(), container.end(), [&](const typename ActivityPosition<_Container>::ContainerElement &activity) {
        return activity->id() == activityId;
    });

    return (position != container.end()) ? ActivityPosition<_Container>(position - container.begin(), position) : ActivityPosition<_Container>();
}

/**
 * Notifies the model that an activity was updated
 */
template<typename _Model, typename _Container>
inline void emitActivityUpdated(_Model *model, const _Container &container, const QString &activity, int role)
{
    auto position = Private::activityPosition(container, activity);

    if (position) {
        Q_EMIT model->q->dataChanged(model->q->index(position.index),
                                     model->q->index(position.index),
                                     role == Qt::DecorationRole ? QList<int>{role, ActivitiesModel::ActivityIconSource} : QList<int>{role});
    }
}

/**
 * Notifies the model that an activity was updated
 */
template<typename _Model, typename _Container>
inline void emitActivityUpdated(_Model *model, const _Container &container, QObject *activityInfo, int role)
{
    const auto activity = static_cast<Info *>(activityInfo);
    emitActivityUpdated(model, container, activity->id(), role);
}

}

ActivitiesModelPrivate::ActivitiesModelPrivate(ActivitiesModel *parent)
    : q(parent)
{
}

ActivitiesModel::ActivitiesModel(QObject *parent)
    : QAbstractListModel(parent)
    , d(new ActivitiesModelPrivate(this))
{
    // Initializing role names for qml
    connect(&d->activities, &Consumer::serviceStatusChanged, this, [this](Consumer::ServiceStatus status) {
        d->setServiceStatus(status);
    });

    connect(&d->activities, &Consumer::activityAdded, this, [this](const QString &activity) {
        d->onActivityAdded(activity);
    });
    connect(&d->activities, &Consumer::activityRemoved, this, [this](const QString &activity) {
        d->onActivityRemoved(activity);
    });
    connect(&d->activities, &Consumer::currentActivityChanged, this, [this](const QString &activity) {
        d->onCurrentActivityChanged(activity);
    });

    d->setServiceStatus(d->activities.serviceStatus());
}

ActivitiesModel::ActivitiesModel(QList<Info::State> shownStates, QObject *parent)
    : QAbstractListModel(parent)
    , d(new ActivitiesModelPrivate(this))
{
    d->shownStates = shownStates;

    // Initializing role names for qml
    connect(&d->activities, &Consumer::serviceStatusChanged, this, [this](Consumer::ServiceStatus status) {
        d->setServiceStatus(status);
    });

    connect(&d->activities, &Consumer::activityAdded, this, [this](const QString &activity) {
        d->onActivityAdded(activity);
    });
    connect(&d->activities, &Consumer::activityRemoved, this, [this](const QString &activity) {
        d->onActivityRemoved(activity);
    });
    connect(&d->activities, &Consumer::currentActivityChanged, this, [this](const QString &activity) {
        d->onCurrentActivityChanged(activity);
    });

    d->setServiceStatus(d->activities.serviceStatus());
}

ActivitiesModel::~ActivitiesModel() = default;

QHash<int, QByteArray> ActivitiesModel::roleNames() const
{
    return {{ActivityName, "name"},
            {ActivityState, "state"},
            {ActivityId, "id"},
            {ActivityIconSource, "iconSource"},
            {ActivityDescription, "description"},
            {ActivityBackground, "background"},
            {ActivityIsCurrent, "isCurrent"}};
}

void ActivitiesModelPrivate::setServiceStatus(Consumer::ServiceStatus)
{
    replaceActivities(activities.activities());
}

void ActivitiesModelPrivate::replaceActivities(const QStringList &activities)
{
    q->beginResetModel();

    knownActivities.clear();
    shownActivities.clear();

    for (const QString &activity : activities) {
        onActivityAdded(activity, false);
    }

    q->endResetModel();
}

void ActivitiesModelPrivate::onActivityAdded(const QString &id, bool notifyClients)
{
    auto info = registerActivity(id);

    showActivity(info, notifyClients);
}

void ActivitiesModelPrivate::onActivityRemoved(const QString &id)
{
    hideActivity(id);
    unregisterActivity(id);
}

void ActivitiesModelPrivate::onCurrentActivityChanged(const QString &id)
{
    Q_UNUSED(id);

    for (const auto &activity : shownActivities) {
        Private::emitActivityUpdated(this, shownActivities, activity->id(), ActivitiesModel::ActivityIsCurrent);
    }
}

ActivitiesModelPrivate::InfoPtr ActivitiesModelPrivate::registerActivity(const QString &id)
{
    auto position = Private::activityPosition(knownActivities, id);

    if (position) {
        return *(position.iterator);

    } else {
        auto activityInfo = std::make_shared<Info>(id);

        auto ptr = activityInfo.get();

        connect(ptr, &Info::nameChanged, this, &ActivitiesModelPrivate::onActivityNameChanged);
        connect(ptr, &Info::descriptionChanged, this, &ActivitiesModelPrivate::onActivityDescriptionChanged);
        connect(ptr, &Info::iconChanged, this, &ActivitiesModelPrivate::onActivityIconChanged);
        connect(ptr, &Info::stateChanged, this, &ActivitiesModelPrivate::onActivityStateChanged);

        knownActivities.insert(InfoPtr(activityInfo));

        return activityInfo;
    }
}

void ActivitiesModelPrivate::unregisterActivity(const QString &id)
{
    auto position = Private::activityPosition(knownActivities, id);

    if (position) {
        if (auto shown = Private::activityPosition(shownActivities, id)) {
            q->beginRemoveRows(QModelIndex(), shown.index, shown.index);
            shownActivities.removeAt(shown.index);
            q->endRemoveRows();
        }

        knownActivities.removeAt(position.index);
    }
}

void ActivitiesModelPrivate::showActivity(InfoPtr activityInfo, bool notifyClients)
{
    // Should it really be shown?
    if (!Private::matchingState(activityInfo, shownStates)) {
        return;
    }

    // Is it already shown?
    if (std::binary_search(shownActivities.cbegin(), shownActivities.cend(), activityInfo, InfoPtrComparator())) {
        return;
    }

    auto registeredPosition = Private::activityPosition(knownActivities, activityInfo->id());

    if (!registeredPosition) {
        qDebug() << "Got a request to show an unknown activity, ignoring";
        return;
    }

    const auto activityInfoPtr = *(registeredPosition.iterator);

    // In C++17, this would be:
    // const auto [iterator, index, found] = shownActivities.insert(...);
    const auto _result = shownActivities.insert(activityInfoPtr);
    // const auto iterator = std::get<0>(_result);
    const auto index = std::get<1>(_result);

    if (notifyClients) {
        q->beginInsertRows(QModelIndex(), index, index);
        q->endInsertRows();
    }
}

void ActivitiesModelPrivate::hideActivity(const QString &id)
{
    auto position = Private::activityPosition(shownActivities, id);

    if (position) {
        q->beginRemoveRows(QModelIndex(), position.index, position.index);
        shownActivities.removeAt(position.index);
        q->endRemoveRows();
    }
}

// clang-format off
#define CREATE_SIGNAL_EMITTER(What,Role)                                      \
    void ActivitiesModelPrivate::onActivity##What##Changed(const QString &)    \
    {                                                                          \
        Private::emitActivityUpdated(this, shownActivities, sender(), Role);   \
    }
// clang-format on

CREATE_SIGNAL_EMITTER(Name, Qt::DisplayRole)
CREATE_SIGNAL_EMITTER(Description, ActivitiesModel::ActivityDescription)
CREATE_SIGNAL_EMITTER(Icon, Qt::DecorationRole)

#undef CREATE_SIGNAL_EMITTER

void ActivitiesModelPrivate::onActivityStateChanged(Info::State state)
{
    if (shownStates.empty()) {
        Private::emitActivityUpdated(this, shownActivities, sender(), ActivitiesModel::ActivityState);

    } else {
        auto info = findActivity(sender());

        if (!info) {
            return;
        }

        if (shownStates.contains(state)) {
            showActivity(info, true);
        } else {
            hideActivity(info->id());
        }
    }
}

void ActivitiesModel::setShownStates(const QList<Info::State> &states)
{
    d->shownStates = states;

    d->replaceActivities(d->activities.activities());

    Q_EMIT shownStatesChanged(states);
}

QList<Info::State> ActivitiesModel::shownStates() const
{
    return d->shownStates;
}

int ActivitiesModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }

    return d->shownActivities.size();
}

QVariant ActivitiesModel::data(const QModelIndex &index, int role) const
{
    const int row = index.row();
    const auto &item = d->shownActivities.at(row);

    switch (role) {
    case Qt::DisplayRole:
    case ActivityName:
        return item->name();

    case ActivityId:
        return item->id();

    case ActivityState:
        return item->state();

    case Qt::DecorationRole:
    case ActivityIconSource: {
        const QString &icon = item->icon();

        // We need a default icon for activities
        return icon.isEmpty() ? QStringLiteral("activities") : icon;
    }

    case ActivityDescription:
        return item->description();

    case ActivityIsCurrent:
        return d->activities.currentActivity() == item->id();

    default:
        return QVariant();
    }
}

QVariant ActivitiesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    Q_UNUSED(role);

    return QVariant();
}

ActivitiesModelPrivate::InfoPtr ActivitiesModelPrivate::findActivity(QObject *ptr) const
{
    auto info = std::find_if(knownActivities.cbegin(), knownActivities.cend(), [ptr](const InfoPtr &info) {
        return ptr == info.get();
    });

    if (info == knownActivities.end()) {
        return nullptr;
    } else {
        return *info;
    }
}

} // namespace KActivities

#include "moc_activitiesmodel.cpp"
#include "moc_activitiesmodel_p.cpp"
