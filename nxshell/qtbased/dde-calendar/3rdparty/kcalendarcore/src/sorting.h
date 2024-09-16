/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2009 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
  SPDX-FileContributor: Alvaro Manera <alvaro.manera@nokia.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCALCORE_SORTING_H
#define KCALCORE_SORTING_H

#include "event.h"
#include "freebusy.h"
#include "journal.h"
#include "person.h"
#include "todo.h"

namespace KCalendarCore {
namespace Events {
Q_CORE_EXPORT bool startDateLessThan(const Event::Ptr &e1, const Event::Ptr &e2);

Q_CORE_EXPORT bool summaryLessThan(const Event::Ptr &e1, const Event::Ptr &e2);

Q_CORE_EXPORT bool summaryMoreThan(const Event::Ptr &e1, const Event::Ptr &e2);

Q_CORE_EXPORT bool startDateMoreThan(const Event::Ptr &e1, const Event::Ptr &e2);

Q_CORE_EXPORT bool endDateLessThan(const Event::Ptr &e1, const Event::Ptr &e2);

Q_CORE_EXPORT bool endDateMoreThan(const Event::Ptr &e1, const Event::Ptr &e2);
} // namespace Events

namespace Todos {
Q_CORE_EXPORT bool startDateLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool startDateMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool dueDateLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool dueDateMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool priorityLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool priorityMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool percentLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool percentMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool summaryLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool summaryMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool createdLessThan(const Todo::Ptr &t1, const Todo::Ptr &t2);

Q_CORE_EXPORT bool createdMoreThan(const Todo::Ptr &t1, const Todo::Ptr &t2);
} // namespace Todos

namespace Journals {
Q_CORE_EXPORT bool dateLessThan(const Journal::Ptr &j1, const Journal::Ptr &j2);

Q_CORE_EXPORT bool dateMoreThan(const Journal::Ptr &j1, const Journal::Ptr &j2);

Q_CORE_EXPORT bool summaryLessThan(const Journal::Ptr &j1, const Journal::Ptr &j2);

Q_CORE_EXPORT bool summaryMoreThan(const Journal::Ptr &j1, const Journal::Ptr &j2);
} // namespace Journals

namespace Incidences {
Q_CORE_EXPORT bool dateLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

Q_CORE_EXPORT bool dateMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

Q_CORE_EXPORT bool createdLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

Q_CORE_EXPORT bool createdMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

Q_CORE_EXPORT bool summaryLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

Q_CORE_EXPORT bool summaryMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

/**
 * Compare the categories (tags) of two incidences, as returned by categoriesStr().
 * If they are equal, return summaryLessThan().
 * @since 5.83
 */
Q_CORE_EXPORT bool categoriesLessThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);

/**
 * Compare the categories (tags) of two incidences, as returned by categoriesStr().
 * If they are equal, return summaryMoreThan().
 * @since 5.83
 */
Q_CORE_EXPORT bool categoriesMoreThan(const Incidence::Ptr &i1, const Incidence::Ptr &i2);
} // namespace Incidences

} // namespace KCalendarCore

#endif
