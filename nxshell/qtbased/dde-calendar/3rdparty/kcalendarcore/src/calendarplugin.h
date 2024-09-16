/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2020 Nicolas Fella <nicolas.fella@gmx.de>
  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#pragma once

#include "calendar.h"

#include <QObject>

namespace KCalendarCore {
/**
  @brief
  A plugin that provides calendar data.

  It allows calendar applications to consume data provided by multiple
  sources, e.g. local ical files or remote calendars.

  @since 5.85

*/
class Q_CORE_EXPORT CalendarPlugin : public QObject
{
    Q_OBJECT
public:
    CalendarPlugin(QObject *parent, const QVariantList &args);

    /**
     * The set of calendars defined by this plugin.
     *
     * @return QVector of calendars.
     */
    virtual QVector<Calendar::Ptr> calendars() const = 0;

Q_SIGNALS:
    /**
     * Emitted when the set of calendars changed.
     */
    void calendarsChanged();

private:
    void *d;
};

} // namespace KCalendarCore
