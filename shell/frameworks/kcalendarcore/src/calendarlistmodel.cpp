/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "calendarlistmodel.h"

#include <KCalendarCore/CalendarPlugin>
#include <KCalendarCore/CalendarPluginLoader>

using namespace KCalendarCore;

namespace KCalendarCore
{
class CalendarListModelPrivate
{
public:
    QList<Calendar::Ptr> calendars;
};
}

CalendarListModel::CalendarListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    if (CalendarPluginLoader::hasPlugin()) {
        d = std::make_unique<CalendarListModelPrivate>();
        d->calendars = CalendarPluginLoader::plugin()->calendars();
        connect(CalendarPluginLoader::plugin(), &CalendarPlugin::calendarsChanged, this, [this]() {
            beginResetModel();
            d->calendars = CalendarPluginLoader::plugin()->calendars();
            endResetModel();
        });
    }
}

CalendarListModel::~CalendarListModel() = default;

int CalendarListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !d) {
        return 0;
    }

    return d->calendars.size();
}

QVariant CalendarListModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index) || !d) {
        return {};
    }

    const auto &cal = d->calendars.at(index.row());
    switch (role) {
    case NameRole:
        return cal->name();
    case IconRole:
        return cal->icon();
    case CalendarRole:
        return QVariant::fromValue(cal.get());
    case AccessModeRole:
        return cal->accessMode();
    case IdRole:
        return cal->id();
    }

    return {};
}

QHash<int, QByteArray> CalendarListModel::roleNames() const
{
    auto n = QAbstractListModel::roleNames();
    n.insert(NameRole, "name");
    n.insert(IconRole, "icon");
    n.insert(CalendarRole, "calendar");
    n.insert(AccessModeRole, "accessMode");
    n.insert(IdRole, "id");
    return n;
}

#include "moc_calendarlistmodel.cpp"
