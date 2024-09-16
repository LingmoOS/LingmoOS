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

    Event::List::Iterator it = eventList->begin();
    while (it != eventList->end()) {
        if (!filterIncidence(*it)) {
            it = eventList->erase(it);
        } else {
            ++it;
        }
    }
}

// TODO: avoid duplicating apply() code
void CalFilter::apply(Todo::List *todoList) const
{
    if (!d->mEnabled) {
        return;
    }

    Todo::List::Iterator it = todoList->begin();
    while (it != todoList->end()) {
        if (!filterIncidence(*it)) {
            it = todoList->erase(it);
        } else {
            ++it;
        }
    }
}

void CalFilter::apply(Journal::List *journalList) const
{
    if (!d->mEnabled) {
        return;
    }

    Journal::List::Iterator it = journalList->begin();
    while (it != journalList->end()) {
        if (!filterIncidence(*it)) {
            it = journalList->erase(it);
        } else {
            ++it;
        }
    }
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
            if (!todo->attendees().isEmpty()) {
                Attendee::List::ConstIterator it;
                for (it = attendees.begin(); it != attendees.end(); ++it) {
                    if (d->mEmailList.contains((*it).email())) {
                        iAmOneOfTheAttendees = true;
                        break;
                    }
                }
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

    if (d->mCriteria & ShowCategories) {
        for (QStringList::ConstIterator it = d->mCategoryList.constBegin(); it != d->mCategoryList.constEnd(); ++it) {
            QStringList incidenceCategories = incidence->categories();
            for (QStringList::ConstIterator it2 = incidenceCategories.constBegin(); it2 != incidenceCategories.constEnd(); ++it2) {
                if ((*it) == (*it2)) {
                    return true;
                }
            }
        }
        return false;
    } else {
        for (QStringList::ConstIterator it = d->mCategoryList.constBegin(); it != d->mCategoryList.constEnd(); ++it) {
            QStringList incidenceCategories = incidence->categories();
            for (QStringList::ConstIterator it2 = incidenceCategories.constBegin(); it2 != incidenceCategories.constEnd(); ++it2) {
                if ((*it) == (*it2)) {
                    return false;
                }
            }
        }
        return true;
    }
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
