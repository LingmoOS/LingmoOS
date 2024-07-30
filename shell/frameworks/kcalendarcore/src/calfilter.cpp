/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2004 Bram Schoenmakers <bramschoenmakers@kde.nl>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the CalFilter class.

  @brief
  Provides a filter for calendars.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author Bram Schoenmakers \<bramschoenmakers@kde.nl\>
*/

#include "calfilter.h"

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::CalFilter::Private
{
public:
    Private()
    {
    }
    QString mName; // filter name
    QStringList mCategoryList;
    QStringList mEmailList;
    int mCriteria = 0;
    int mCompletedTimeSpan = 0;
    bool mEnabled = true;
};
//@endcond

CalFilter::CalFilter()
    : d(new KCalendarCore::CalFilter::Private)
{
}

CalFilter::CalFilter(const QString &name)
    : d(new KCalendarCore::CalFilter::Private)
{
    d->mName = name;
}

CalFilter::~CalFilter()
{
    delete d;
}

bool KCalendarCore::CalFilter::operator==(const CalFilter &filter) const
{
    return d->mName == filter.d->mName && d->mCriteria == filter.d->mCriteria && d->mCategoryList == filter.d->mCategoryList
        && d->mEmailList == filter.d->mEmailList && d->mCompletedTimeSpan == filter.d->mCompletedTimeSpan;
}

void CalFilter::apply(Event::List *eventList) const
{
    if (!d->mEnabled) {
        return;
    }

    auto it = std::remove_if(eventList->begin(), eventList->end(), [this](const Incidence::Ptr &incidence) {
        return !filterIncidence(incidence);
    });
    eventList->erase(it, eventList->end());
}

// TODO: avoid duplicating apply() code
void CalFilter::apply(Todo::List *todoList) const
{
    if (!d->mEnabled) {
        return;
    }

    auto it = std::remove_if(todoList->begin(), todoList->end(), [this](const Incidence::Ptr &incidence) {
        return !filterIncidence(incidence);
    });
    todoList->erase(it, todoList->end());
}

void CalFilter::apply(Journal::List *journalList) const
{
    if (!d->mEnabled) {
        return;
    }

    auto it = std::remove_if(journalList->begin(), journalList->end(), [this](const Incidence::Ptr &incidence) {
        return !filterIncidence(incidence);
    });
    journalList->erase(it, journalList->end());
}

bool CalFilter::filterIncidence(const Incidence::Ptr &incidence) const
{
    if (!d->mEnabled) {
        return true;
    }

    Todo::Ptr todo = incidence.dynamicCast<Todo>();
    if (todo) {
        if ((d->mCriteria & HideCompletedTodos) && todo->isCompleted()) {
            // Check if completion date is suffently long ago:
            if (todo->completed().addDays(d->mCompletedTimeSpan) < QDateTime::currentDateTimeUtc()) {
                return false;
            }
        }

        if ((d->mCriteria & HideInactiveTodos) && ((todo->hasStartDate() && QDateTime::currentDateTimeUtc() < todo->dtStart()) || todo->isCompleted())) {
            return false;
        }

        if (d->mCriteria & HideNoMatchingAttendeeTodos) {
            bool iAmOneOfTheAttendees = false;
            const Attendee::List &attendees = todo->attendees();
            if (!attendees.isEmpty()) {
                iAmOneOfTheAttendees = std::any_of(attendees.cbegin(), attendees.cend(), [this](const Attendee &att) {
                    return d->mEmailList.contains(att.email());
                });
            } else {
                // no attendees, must be me only
                iAmOneOfTheAttendees = true;
            }
            if (!iAmOneOfTheAttendees) {
                return false;
            }
        }
    }

    if (d->mCriteria & HideRecurring) {
        if (incidence->recurs() || incidence->hasRecurrenceId()) {
            return false;
        }
    }

    const QStringList incidenceCategories = incidence->categories();
    bool isFound = false;
    for (const auto &category : std::as_const(d->mCategoryList)) {
        if (incidenceCategories.contains(category)) {
            isFound = true;
            break;
        }
    }

    return (d->mCriteria & ShowCategories) ? isFound : !isFound;
}

void CalFilter::setName(const QString &name)
{
    d->mName = name;
}

QString CalFilter::name() const
{
    return d->mName;
}

void CalFilter::setEnabled(bool enabled)
{
    d->mEnabled = enabled;
}

bool CalFilter::isEnabled() const
{
    return d->mEnabled;
}

void CalFilter::setCriteria(int criteria)
{
    d->mCriteria = criteria;
}

int CalFilter::criteria() const
{
    return d->mCriteria;
}

void CalFilter::setCategoryList(const QStringList &categoryList)
{
    d->mCategoryList = categoryList;
}

QStringList CalFilter::categoryList() const
{
    return d->mCategoryList;
}

void CalFilter::setEmailList(const QStringList &emailList)
{
    d->mEmailList = emailList;
}

QStringList CalFilter::emailList() const
{
    return d->mEmailList;
}

void CalFilter::setCompletedTimeSpan(int timespan)
{
    d->mCompletedTimeSpan = timespan;
}

int CalFilter::completedTimeSpan() const
{
    return d->mCompletedTimeSpan;
}
