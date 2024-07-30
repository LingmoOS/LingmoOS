/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2000-2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>
  SPDX-FileCopyrightText: 2006 David Jarvie <djarvie@kde.org>
  SPDX-FileCopyrightText: 2021 Boris Shmarin <b.shmarin@omp.ru>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Calendar class.

  @brief
  Represents the main calendar class.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
  @author David Jarvie \<djarvie@kde.org\>
*/
#include "calendar.h"
#include "calendar_p.h"
#include "calfilter.h"
#include "icaltimezones_p.h"
#include "sorting.h"
#include "visitor.h"

#include "kcalendarcore_debug.h"


extern "C" {
#include <libical/icaltimezone.h>
}

#include <algorithm>
#include <set>

using namespace KCalendarCore;

/**
  Template for a class that implements a visitor for adding an Incidence
  to a resource supporting addEvent(), addTodo() and addJournal() calls.
*/
template<class T>
class AddVisitor : public Visitor
{
public:
    AddVisitor(T *r)
        : mResource(r)
    {
    }

    bool visit(const Event::Ptr &e) override
    {
        return mResource->addEvent(e);
    }
    bool visit(const Todo::Ptr &t) override
    {
        return mResource->addTodo(t);
    }
    bool visit(const Journal::Ptr &j) override
    {
        return mResource->addJournal(j);
    }
    bool visit(const FreeBusy::Ptr &) override
    {
        return false;
    }

private:
    T *mResource;
};

/**
  Template for a class that implements a visitor for deleting an Incidence
  from a resource supporting deleteEvent(), deleteTodo() and deleteJournal()
  calls.
*/
template<class T>
class DeleteVisitor : public Visitor
{
public:
    DeleteVisitor(T *r)
        : mResource(r)
    {
    }

    bool visit(const Event::Ptr &e) override
    {
        mResource->deleteEvent(e);
        return true;
    }
    bool visit(const Todo::Ptr &t) override
    {
        mResource->deleteTodo(t);
        return true;
    }
    bool visit(const Journal::Ptr &j) override
    {
        mResource->deleteJournal(j);
        return true;
    }
    bool visit(const FreeBusy::Ptr &) override
    {
        return false;
    }

private:
    T *mResource;
};
//@endcond

Calendar::Calendar(const QTimeZone &timeZone)
    : d(new KCalendarCore::Calendar::Private)
{
    if (timeZone.isValid()) {
        d->mTimeZone = timeZone;
    } else {
        d->mTimeZone = QTimeZone::systemTimeZone();
    }
}

Calendar::Calendar(const QByteArray &timeZoneId)
    : d(new KCalendarCore::Calendar::Private)
{
    setTimeZoneId(timeZoneId);
}

Calendar::~Calendar()
{
    delete d;
}

Person Calendar::owner() const
{
    return d->mOwner;
}

void Calendar::setOwner(const Person &owner)
{
    if (owner != d->mOwner) {
        d->mOwner = owner;
        setModified(true);
        Q_EMIT ownerChanged();
    }
}

void Calendar::setTimeZone(const QTimeZone &timeZone)
{
    if (timeZone.isValid()) {
        d->mTimeZone = timeZone;
    } else {
        d->mTimeZone = QTimeZone::systemTimeZone();
    }

    doSetTimeZone(d->mTimeZone);
}

QTimeZone Calendar::timeZone() const
{
    return d->mTimeZone;
}

void Calendar::setTimeZoneId(const QByteArray &timeZoneId)
{
    d->mTimeZone = d->timeZoneIdSpec(timeZoneId);

    doSetTimeZone(d->mTimeZone); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall
}

//@cond PRIVATE
QTimeZone Calendar::Private::timeZoneIdSpec(const QByteArray &timeZoneId)
{
    if (timeZoneId == QByteArrayLiteral("UTC")) {
        return QTimeZone::utc();
    }
    auto tz = QTimeZone(timeZoneId);
    if (tz.isValid()) {
        return tz;
    }
    return QTimeZone::systemTimeZone();
}
//@endcond

QByteArray Calendar::timeZoneId() const
{
    return d->mTimeZone.id();
}

void Calendar::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    setTimeZone(newZone);

    int i;
    int end;
    Event::List ev = events();
    for (i = 0, end = ev.count(); i < end; ++i) {
        ev[i]->shiftTimes(oldZone, newZone);
    }

    Todo::List to = todos();
    for (i = 0, end = to.count(); i < end; ++i) {
        to[i]->shiftTimes(oldZone, newZone);
    }

    Journal::List jo = journals();
    for (i = 0, end = jo.count(); i < end; ++i) {
        jo[i]->shiftTimes(oldZone, newZone);
    }
}

void Calendar::setFilter(CalFilter *filter)
{
    if (filter) {
        d->mFilter = filter;
    } else {
        d->mFilter = d->mDefaultFilter;
    }
    Q_EMIT filterChanged();
}

CalFilter *Calendar::filter() const
{
    return d->mFilter;
}

QStringList Calendar::categories() const
{
    const Incidence::List rawInc = rawIncidences();
    QStringList uniqueCategories;
    QStringList thisCats;
    // @TODO: For now just iterate over all incidences. In the future,
    // the list of categories should be built when reading the file.
    for (const Incidence::Ptr &inc : rawInc) {
        thisCats = inc->categories();
        for (const auto &cat : std::as_const(thisCats)) {
            if (!uniqueCategories.contains(cat)) {
                uniqueCategories.append(cat);
            }
        }
    }
    return uniqueCategories;
}

Incidence::List Calendar::incidences(const QDate &date) const
{
    return mergeIncidenceList(events(date), todos(date), journals(date));
}

Incidence::List Calendar::incidences() const
{
    return mergeIncidenceList(events(), todos(), journals());
}

Incidence::List Calendar::rawIncidences() const
{
    return mergeIncidenceList(rawEvents(), rawTodos(), rawJournals());
}

Incidence::List Calendar::instances(const Incidence::Ptr &incidence) const
{
    if (incidence) {
        Event::List elist;
        Todo::List tlist;
        Journal::List jlist;

        if (incidence->type() == Incidence::TypeEvent) {
            elist = eventInstances(incidence);
        } else if (incidence->type() == Incidence::TypeTodo) {
            tlist = todoInstances(incidence);
        } else if (incidence->type() == Incidence::TypeJournal) {
            jlist = journalInstances(incidence);
        }
        return mergeIncidenceList(elist, tlist, jlist);
    } else {
        return Incidence::List();
    }
}

Event::List Calendar::sortEvents(Event::List &&eventList, EventSortField sortField, SortDirection sortDirection)
{
    switch (sortField) {
    case EventSortUnsorted:
        break;

    case EventSortStartDate:
        if (sortDirection == SortDirectionAscending) {
            std::sort(eventList.begin(), eventList.end(), Events::startDateLessThan);
        } else {
            std::sort(eventList.begin(), eventList.end(), Events::startDateMoreThan);
        }
        break;

    case EventSortEndDate:
        if (sortDirection == SortDirectionAscending) {
            std::sort(eventList.begin(), eventList.end(), Events::endDateLessThan);
        } else {
            std::sort(eventList.begin(), eventList.end(), Events::endDateMoreThan);
        }
        break;

    case EventSortSummary:
        if (sortDirection == SortDirectionAscending) {
            std::sort(eventList.begin(), eventList.end(), Events::summaryLessThan);
        } else {
            std::sort(eventList.begin(), eventList.end(), Events::summaryMoreThan);
        }
        break;
    }

    return eventList;
}

Event::List Calendar::events(const QDate &date, const QTimeZone &timeZone, EventSortField sortField, SortDirection sortDirection) const
{
    Event::List el = rawEventsForDate(date, timeZone, sortField, sortDirection);
    d->mFilter->apply(&el);
    return el;
}

Event::List Calendar::events(const QDateTime &dt) const
{
    Event::List el = rawEventsForDate(dt.date(), dt.timeZone());
    d->mFilter->apply(&el);
    return el;
}

Event::List Calendar::events(const QDate &start, const QDate &end, const QTimeZone &timeZone, bool inclusive) const
{
    Event::List el = rawEvents(start, end, timeZone, inclusive);
    d->mFilter->apply(&el);
    return el;
}

Event::List Calendar::events(EventSortField sortField, SortDirection sortDirection) const
{
    Event::List el = rawEvents(sortField, sortDirection);
    d->mFilter->apply(&el);
    return el;
}

bool Calendar::addIncidence(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return false;
    }

    AddVisitor<Calendar> v(this);
    return incidence->accept(v, incidence);
}

bool Calendar::deleteIncidence(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return false;
    }

    if (beginChange(incidence)) {
        DeleteVisitor<Calendar> v(this);
        const bool result = incidence->accept(v, incidence);
        endChange(incidence);
        return result;
    } else {
        return false;
    }
}

Incidence::Ptr Calendar::createException(const Incidence::Ptr &incidence, const QDateTime &recurrenceId, bool thisAndFuture)
{
    Q_ASSERT(recurrenceId.isValid());
    if (!incidence || !incidence->recurs() || !recurrenceId.isValid()) {
        return Incidence::Ptr();
    }

    Incidence::Ptr newInc(incidence->clone());
    const QDateTime current = QDateTime::currentDateTimeUtc();
    newInc->setCreated(current);
    newInc->setLastModified(current);
    newInc->setRevision(0);
    // Recurring exceptions are not support for now
    newInc->clearRecurrence();

    newInc->setRecurrenceId(recurrenceId);
    newInc->setThisAndFuture(thisAndFuture);
    newInc->setDtStart(recurrenceId);

    // Calculate and set the new end of the incidence
    QDateTime end = incidence->dateTime(IncidenceBase::RoleEnd);

    if (end.isValid()) {
        if (incidence->allDay()) {
            qint64 offset = incidence->dtStart().daysTo(recurrenceId);
            end = end.addDays(offset);
        } else {
            qint64 offset = incidence->dtStart().secsTo(recurrenceId);
            end = end.addSecs(offset);
        }
        newInc->setDateTime(end, IncidenceBase::RoleEnd);
    }
    return newInc;
}

Incidence::Ptr Calendar::incidence(const QString &uid, const QDateTime &recurrenceId) const
{
    Incidence::Ptr i = event(uid, recurrenceId);
    if (i) {
        return i;
    }

    i = todo(uid, recurrenceId);
    if (i) {
        return i;
    }

    i = journal(uid, recurrenceId);
    return i;
}

Incidence::List Calendar::incidencesFromSchedulingID(const QString &sid) const
{
    Incidence::List result;
    const Incidence::List incidences = rawIncidences();
    std::copy_if(incidences.cbegin(), incidences.cend(), std::back_inserter(result), [&sid](const Incidence::Ptr &in) {
        return in->schedulingID() == sid;
    });
    return result;
}

Incidence::Ptr Calendar::incidenceFromSchedulingID(const QString &uid) const
{
    const Incidence::List incidences = rawIncidences();
    const auto itEnd = incidences.cend();
    auto it = std::find_if(incidences.cbegin(), itEnd, [&uid](const Incidence::Ptr &in) {
        return in->schedulingID() == uid;
    });

    return it != itEnd ? *it : Incidence::Ptr();
}

Todo::List Calendar::sortTodos(Todo::List &&todoList, TodoSortField sortField, SortDirection sortDirection)
{
    // Note that To-dos may not have Start DateTimes nor due DateTimes.
    switch (sortField) {
    case TodoSortUnsorted:
        break;

    case TodoSortStartDate:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Todos::startDateLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Todos::startDateMoreThan);
        }
        break;

    case TodoSortDueDate:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Todos::dueDateLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Todos::dueDateMoreThan);
        }
        break;

    case TodoSortPriority:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Todos::priorityLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Todos::priorityMoreThan);
        }
        break;

    case TodoSortPercentComplete:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Todos::percentLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Todos::percentMoreThan);
        }
        break;

    case TodoSortSummary:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Todos::summaryLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Todos::summaryMoreThan);
        }
        break;

    case TodoSortCreated:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Todos::createdLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Todos::createdMoreThan);
        }
        break;

    case TodoSortCategories:
        if (sortDirection == SortDirectionAscending) {
            std::sort(todoList.begin(), todoList.end(), Incidences::categoriesLessThan);
        } else {
            std::sort(todoList.begin(), todoList.end(), Incidences::categoriesMoreThan);
        }
        break;
    }

    return todoList;
}

Todo::List Calendar::todos(TodoSortField sortField, SortDirection sortDirection) const
{
    Todo::List tl = rawTodos(sortField, sortDirection);
    d->mFilter->apply(&tl);
    return tl;
}

Todo::List Calendar::todos(const QDate &date) const
{
    Todo::List el = rawTodosForDate(date);
    d->mFilter->apply(&el);
    return el;
}

Todo::List Calendar::todos(const QDate &start, const QDate &end, const QTimeZone &timeZone, bool inclusive) const
{
    Todo::List tl = rawTodos(start, end, timeZone, inclusive);
    d->mFilter->apply(&tl);
    return tl;
}

Journal::List Calendar::sortJournals(Journal::List &&journalList, JournalSortField sortField, SortDirection sortDirection)
{
    switch (sortField) {
    case JournalSortUnsorted:
        break;

    case JournalSortDate:
        if (sortDirection == SortDirectionAscending) {
            std::sort(journalList.begin(), journalList.end(), Journals::dateLessThan);
        } else {
            std::sort(journalList.begin(), journalList.end(), Journals::dateMoreThan);
        }
        break;

    case JournalSortSummary:
        if (sortDirection == SortDirectionAscending) {
            std::sort(journalList.begin(), journalList.end(), Journals::summaryLessThan);
        } else {
            std::sort(journalList.begin(), journalList.end(), Journals::summaryMoreThan);
        }
        break;
    }

    return journalList;
}

Journal::List Calendar::journals(JournalSortField sortField, SortDirection sortDirection) const
{
    Journal::List jl = rawJournals(sortField, sortDirection);
    d->mFilter->apply(&jl);
    return jl;
}

Journal::List Calendar::journals(const QDate &date) const
{
    Journal::List el = rawJournalsForDate(date);
    d->mFilter->apply(&el);
    return el;
}

Calendar::CalendarObserver::~CalendarObserver()
{
}

void Calendar::CalendarObserver::calendarModified(bool modified, Calendar *calendar)
{
    Q_UNUSED(modified);
    Q_UNUSED(calendar);
}

void Calendar::CalendarObserver::calendarIncidenceAdded(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
}

void Calendar::CalendarObserver::calendarIncidenceChanged(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
}

void Calendar::CalendarObserver::calendarIncidenceAboutToBeDeleted(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
}

void Calendar::CalendarObserver::calendarIncidenceDeleted(const Incidence::Ptr &incidence, const Calendar *calendar)
{
    Q_UNUSED(incidence);
    Q_UNUSED(calendar);
}

void Calendar::CalendarObserver::calendarIncidenceAdditionCanceled(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
}

void Calendar::registerObserver(CalendarObserver *observer)
{
    if (!observer) {
        return;
    }

    if (!d->mObservers.contains(observer)) {
        d->mObservers.append(observer);
    } else {
        d->mNewObserver = true;
    }
}

void Calendar::unregisterObserver(CalendarObserver *observer)
{
    if (!observer) {
        return;
    } else {
        d->mObservers.removeAll(observer);
    }
}

void Calendar::setModified(bool modified)
{
    if (modified != d->mModified || d->mNewObserver) {
        d->mNewObserver = false;
        for (CalendarObserver *observer : std::as_const(d->mObservers)) {
            observer->calendarModified(modified, this);
        }
        d->mModified = modified;
    }
}

bool Calendar::isModified() const
{
    return d->mModified;
}

void Calendar::incidenceUpdated(const QString &uid, const QDateTime &recurrenceId)
{
    Incidence::Ptr inc = incidence(uid, recurrenceId);

    if (!inc) {
        return;
    }

    inc->setLastModified(QDateTime::currentDateTimeUtc());
    // we should probably update the revision number here,
    // or internally in the Event itself when certain things change.
    // need to verify with ical documentation.

    notifyIncidenceChanged(inc);

    setModified(true);
}

void Calendar::doSetTimeZone(const QTimeZone &timeZone)
{
    Q_UNUSED(timeZone);
}

void Calendar::notifyIncidenceAdded(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return;
    }

    if (!d->mObserversEnabled) {
        return;
    }

    for (CalendarObserver *observer : std::as_const(d->mObservers)) {
        observer->calendarIncidenceAdded(incidence);
    }

    for (auto role : {IncidenceBase::RoleStartTimeZone, IncidenceBase::RoleEndTimeZone}) {
        const auto dt = incidence->dateTime(role);
        if (dt.isValid() && dt.timeZone() != QTimeZone::utc()) {
            if (!d->mTimeZones.contains(dt.timeZone())) {
                d->mTimeZones.push_back(dt.timeZone());
            }
        }
    }
}

void Calendar::notifyIncidenceChanged(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return;
    }

    if (!d->mObserversEnabled) {
        return;
    }

    for (CalendarObserver *observer : std::as_const(d->mObservers)) {
        observer->calendarIncidenceChanged(incidence);
    }
}

void Calendar::notifyIncidenceAboutToBeDeleted(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return;
    }

    if (!d->mObserversEnabled) {
        return;
    }

    for (CalendarObserver *observer : std::as_const(d->mObservers)) {
        observer->calendarIncidenceAboutToBeDeleted(incidence);
    }
}

void Calendar::notifyIncidenceDeleted(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return;
    }

    if (!d->mObserversEnabled) {
        return;
    }

    for (CalendarObserver *observer : std::as_const(d->mObservers)) {
        observer->calendarIncidenceDeleted(incidence, this);
    }
}

void Calendar::notifyIncidenceAdditionCanceled(const Incidence::Ptr &incidence)
{
    if (!incidence) {
        return;
    }

    if (!d->mObserversEnabled) {
        return;
    }

    for (CalendarObserver *observer : std::as_const(d->mObservers)) {
        observer->calendarIncidenceAdditionCanceled(incidence);
    }
}

void Calendar::customPropertyUpdated()
{
    setModified(true);
}

void Calendar::setProductId(const QString &id)
{
    d->mProductId = id;
}

QString Calendar::productId() const
{
    return d->mProductId;
}

/** static */
Incidence::List Calendar::mergeIncidenceList(const Event::List &events, const Todo::List &todos, const Journal::List &journals)
{
    Incidence::List incidences;
    incidences.reserve(events.count() + todos.count() + journals.count());

    int i;
    int end;
    for (i = 0, end = events.count(); i < end; ++i) {
        incidences.append(events[i]);
    }

    for (i = 0, end = todos.count(); i < end; ++i) {
        incidences.append(todos[i]);
    }

    for (i = 0, end = journals.count(); i < end; ++i) {
        incidences.append(journals[i]);
    }

    return incidences;
}

bool Calendar::beginChange(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
    return true;
}

bool Calendar::endChange(const Incidence::Ptr &incidence)
{
    Q_UNUSED(incidence);
    return true;
}

void Calendar::setObserversEnabled(bool enabled)
{
    d->mObserversEnabled = enabled;
}

void Calendar::appendAlarms(Alarm::List &alarms, const Incidence::Ptr &incidence, const QDateTime &from, const QDateTime &to) const
{
    QDateTime preTime = from.addSecs(-1);

    Alarm::List alarmlist = incidence->alarms();
    for (int i = 0, iend = alarmlist.count(); i < iend; ++i) {
        if (alarmlist[i]->enabled()) {
            QDateTime dt = alarmlist[i]->nextRepetition(preTime);
            if (dt.isValid() && dt <= to) {
                qCDebug(KCALCORE_LOG) << incidence->summary() << "':" << dt.toString();
                alarms.append(alarmlist[i]);
            }
        }
    }
}

void Calendar::appendRecurringAlarms(Alarm::List &alarms, const Incidence::Ptr &incidence, const QDateTime &from, const QDateTime &to) const
{
    QDateTime dt;
    bool endOffsetValid = false;
    Duration endOffset(0);
    Duration period(from, to);

    Alarm::List alarmlist = incidence->alarms();
    for (int i = 0, iend = alarmlist.count(); i < iend; ++i) {
        Alarm::Ptr a = alarmlist[i];
        if (a->enabled()) {
            if (a->hasTime()) {
                // The alarm time is defined as an absolute date/time
                dt = a->nextRepetition(from.addSecs(-1));
                if (!dt.isValid() || dt > to) {
                    continue;
                }
            } else {
                // Alarm time is defined by an offset from the event start or end time.
                // Find the offset from the event start time, which is also used as the
                // offset from the recurrence time.
                Duration offset(0);
                if (a->hasStartOffset()) {
                    offset = a->startOffset();
                } else if (a->hasEndOffset()) {
                    offset = a->endOffset();
                    if (!endOffsetValid) {
                        endOffset = Duration(incidence->dtStart(), incidence->dateTime(Incidence::RoleAlarmEndOffset));
                        endOffsetValid = true;
                    }
                }

                // Find the incidence's earliest alarm
                QDateTime alarmStart = offset.end(a->hasEndOffset() ? incidence->dateTime(Incidence::RoleAlarmEndOffset) : incidence->dtStart());
                if (alarmStart > to) {
                    continue;
                }
                QDateTime baseStart = incidence->dtStart();
                if (from > alarmStart) {
                    alarmStart = from; // don't look earlier than the earliest alarm
                    baseStart = (-offset).end((-endOffset).end(alarmStart));
                }

                // Adjust the 'alarmStart' date/time and find the next recurrence at or after it.
                // Treat the two offsets separately in case one is daily and the other not.
                dt = incidence->recurrence()->getNextDateTime(baseStart.addSecs(-1));
                if (!dt.isValid() || (dt = endOffset.end(offset.end(dt))) > to) { // adjust 'dt' to get the alarm time
                    // The next recurrence is too late.
                    if (!a->repeatCount()) {
                        continue;
                    }

                    // The alarm has repetitions, so check whether repetitions of previous
                    // recurrences fall within the time period.
                    bool found = false;
                    Duration alarmDuration = a->duration();
                    for (QDateTime base = baseStart; (dt = incidence->recurrence()->getPreviousDateTime(base)).isValid(); base = dt) {
                        if (a->duration().end(dt) < base) {
                            break; // this recurrence's last repetition is too early, so give up
                        }

                        // The last repetition of this recurrence is at or after 'alarmStart' time.
                        // Check if a repetition occurs between 'alarmStart' and 'to'.
                        int snooze = a->snoozeTime().value(); // in seconds or days
                        if (a->snoozeTime().isDaily()) {
                            Duration toFromDuration(dt, base);
                            int toFrom = toFromDuration.asDays();
                            if (a->snoozeTime().end(from) <= to || (toFromDuration.isDaily() && toFrom % snooze == 0)
                                || (toFrom / snooze + 1) * snooze <= toFrom + period.asDays()) {
                                found = true;
#ifndef NDEBUG
                                // for debug output
                                dt = offset.end(dt).addDays(((toFrom - 1) / snooze + 1) * snooze);
#endif
                                break;
                            }
                        } else {
                            int toFrom = dt.secsTo(base);
                            if (period.asSeconds() >= snooze || toFrom % snooze == 0 || (toFrom / snooze + 1) * snooze <= toFrom + period.asSeconds()) {
                                found = true;
#ifndef NDEBUG
                                // for debug output
                                dt = offset.end(dt).addSecs(((toFrom - 1) / snooze + 1) * snooze);
#endif
                                break;
                            }
                        }
                    }
                    if (!found) {
                        continue;
                    }
                }
            }
            qCDebug(KCALCORE_LOG) << incidence->summary() << "':" << dt.toString();
            alarms.append(a);
        }
    }
}

void Calendar::startBatchAdding()
{
    d->batchAddingInProgress = true;
}

void Calendar::endBatchAdding()
{
    d->batchAddingInProgress = false;
}

bool Calendar::batchAdding() const
{
    return d->batchAddingInProgress;
}

Alarm::List Calendar::alarmsTo(const QDateTime &to) const
{
    return alarms(QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0)), to);
}

void Calendar::virtual_hook(int id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    Q_ASSERT(false);
}

QString Calendar::id() const
{
    return d->mId;
}

void Calendar::setId(const QString &id)
{
    if (d->mId != id) {
        d->mId = id;
        Q_EMIT idChanged();
    }
}

QString Calendar::name() const
{
    return d->mName;
}

void Calendar::setName(const QString &name)
{
    if (d->mName != name) {
        d->mName = name;
        Q_EMIT nameChanged();
    }
}

QIcon Calendar::icon() const
{
    return d->mIcon;
}

void Calendar::setIcon(const QIcon &icon)
{
    d->mIcon = icon;
    Q_EMIT iconChanged();
}

AccessMode Calendar::accessMode() const
{
    return d->mAccessMode;
}

void Calendar::setAccessMode(const AccessMode mode)
{
    if (d->mAccessMode != mode) {
        d->mAccessMode = mode;
        Q_EMIT accessModeChanged();
    }
}

bool Calendar::isLoading() const
{
    return d->mIsLoading;
}

void Calendar::setIsLoading(bool isLoading)
{
    if (d->mIsLoading == isLoading) {
        return;
    }

    d->mIsLoading = isLoading;
    Q_EMIT isLoadingChanged();
}

#include "moc_calendar.cpp"
