/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCALENDARCORE_CALENDARLISTMODEL_H
#define KCALENDARCORE_CALENDARLISTMODEL_H

#include "kcalendarcore_export.h"

#include <QAbstractListModel>

#include <memory>

namespace KCalendarCore
{

class CalendarListModelPrivate;

/** Model adaptor for KCalendarCore::CalendarPlugin::calendars().
 *
 *  @since 6.4
 */
class KCALENDARCORE_EXPORT CalendarListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit CalendarListModel(QObject *parent = nullptr);
    ~CalendarListModel();

    enum Role {
        NameRole = Qt::DisplayRole, ///< display name of the calendar
        IconRole = Qt::DecorationRole, ///< the calendar icon, when available
        CalendarRole = Qt::UserRole, ///< the KCalendarCore::Calendar calendar
        AccessModeRole, ///< the access mode of the calendar (see KCalendarCore::AccessMode)
        IdRole, ///< the internal calendar id
    };

    [[nodiscard]] int rowCount(const QModelIndex &parent = {}) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    std::unique_ptr<CalendarListModelPrivate> d;
};

}

#endif // KCALENDARCORE_CALENDARLISTMODEL_H
