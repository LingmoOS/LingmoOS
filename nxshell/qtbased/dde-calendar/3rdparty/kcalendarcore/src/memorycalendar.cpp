/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001, 2003, 2004 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2003-2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the MemoryCalendar class.

  @brief
  This class provides a calendar stored as a local file.

  @author Preston Brown \<pbrown@kde.org\>
  @author Cornelius Schumacher \<schumacher@kde.org\>
 */

#include "memorycalendar.h"
#include "calformat.h"

#include <QDate>
#include <QDebug>

#include <functional>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::MemoryCalendar::Private
{
private:
    static constexpr int incidenceTypeCount = 4;

public:
    Private(MemoryCalendar *qq)
        : q(qq)
        , mFormat(nullptr)
        , mUpdateLastModified(true)
    {
    }
    ~Private()
    {
    }

    MemoryCalendar *q;
    CalFormat *mFormat; // calendar format
    QString mIncidenceBeingUpdated; //  Instance identifier of Incidence currently being updated
    bool mUpdateLastModified; // Call setLastModified() on incidence modific ations

    /**
     * List of all incidences.
     * First indexed by incidence->type(), then by incidence->uid();
     */
    QMultiHash<QString, Incidence::Ptr> mIncidences[incidenceTypeCount];

    /**
     * Has all incidences, indexed by identifier.
     */
    QHash<QString, KCalendarCore::Incidence::Ptr> mIncidencesByIdentifier;

    /**
     * List of all deleted incidences.
     * First indexed by incidence->type(), then by incidence->uid();
     */
    QMultiHash<QString, Incidence::Ptr> mDeletedIncidences[incidenceTypeCount];

    /**
     * Contains incidences ( to-dos; non-recurring, non-multiday events; journals; )
     * indexed by start/due date.
     *
     * The QMap key is the incidence->type().
     * The QMultiHash key is the dtStart/dtDue() converted to calendar's timezone
     *
     * Note: We had 3 variables, mJournalsForDate, mTodosForDate and mEventsForDate
     * but i merged them into one (indexed by type) because it simplifies code using
     * it. No need to if else based on type.
     */
    QMultiHash<QDate, Incidence::Ptr> mIncidencesForDate[incidenceTypeCount];

    void insertIncidence(const Incidence::Ptr &incidence);

    Incidence::Ptr incidence(const QString &uid, IncidenceBase::IncidenceType type, const QDateTime &recurrenceId = {}) const;

    bool deleteIncidence(const QString &uid, IncidenceBase::IncidenceType type, const QDateTime &recurrenceId = {});

    Incidence::Ptr deletedIncidence(const QString &uid, const QDateTime &recurrenceId, IncidenceBase::IncidenceType type) const;

    void deleteAllIncidences(IncidenceBase::IncidenceType type);

    template<typename IncidenceType, typename Key>
    void forIncidences(const QMultiHash<Key, Incidence::Ptr> &incidences, const Key &key, std::function<void(const typename IncidenceType::Ptr &)> &&op) const
    {
        for (auto it = incidences.constFind(key), end = incidences.cend(); it != end && it.key() == key; ++it) {
            op(it.value().template staticCast<IncidenceType>());
        }
    }

    template<typename IncidenceType, typename Key>
    void forIncidences(const QMultiHash<Key, Incidence::Ptr> &incidences, std::function<void(const typename IncidenceType::Ptr &)> &&op) const
    {
        for (const auto &incidence : incidences) {
            op(incidence.template staticCast<IncidenceType>());
        }
    }

    template<typename IncidenceType>
    typename IncidenceType::List castIncidenceList(const QMultiHash<QString, Incidence::Ptr> &incidences) const
    {
        typename IncidenceType::List list;
        list.reserve(incidences.size());
        std::transform(incidences.cbegin(), incidences.cend(), std::back_inserter(list), [](const Incidence::Ptr &inc) {
            return inc.staticCast<IncidenceType>();
        });
        return list;
    }

    template<typename IncidenceType>
    typename IncidenceType::List incidenceInstances(IncidenceBase::IncidenceType type, const Incidence::Ptr &incidence) const
    {
        typename IncidenceType::List list;
        forIncidences<IncidenceType, QString>(mIncidences[type], incidence->uid(), [&list](const typename IncidenceType::Ptr &incidence) {
            if (incidence->hasRecurrenceId()) {
                list.push_back(incidence);
            }
        });
        return list;
    }

    Incidence::Ptr findIncidence(const QMultiHash<QString, Incidence::Ptr> &incidences, const QString &uid, const QDateTime &recurrenceId) const
    {
        for (auto it = incidences.constFind(uid), end = incidences.cend(); it != end && it.key() == uid; ++it) {
            const auto &incidence = it.value();
            if (recurrenceId.isNull() && !incidence->hasRecurrenceId()) {
                return incidence;
            } else if (!recurrenceId.isNull() && incidence->hasRecurrenceId() && recurrenceId == incidence->recurrenceId()) {
                return incidence;
            }
        }
        return {};
    }
};
//@endcond

MemoryCalendar::MemoryCalendar(const QTimeZone &timeZone)
    : Calendar(timeZone)
    , d(new KCalendarCore::MemoryCalendar::Private(this))
{
}

MemoryCalendar::MemoryCalendar(const QByteArray &timeZoneId)
    : Calendar(timeZoneId)
    , d(new KCalendarCore::MemoryCalendar::Private(this))
{
}

MemoryCalendar::~MemoryCalendar()
{
    close(); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall
    delete d;
}

void MemoryCalendar::doSetTimeZone(const QTimeZone &timeZone)
{
    // Reset date based hashes before storing for the new zone.
    for (auto &table : d->mIncidencesForDate) {
        table.clear();
    }

    for (auto &table : d->mIncidences) {
        for (const auto &incidence : table) {
            const QDateTime dt = incidence->dateTime(Incidence::RoleCalendarHashing);
            if (dt.isValid()) {
                d->mIncidencesForDate[incidence->type()].insert(dt.toTimeZone(timeZone).date(), incidence);
            }
        }
    }
}

void MemoryCalendar::close()
{
    setObserversEnabled(false);

    // Don't call the virtual function deleteEvents() etc, the base class might have
    // other ways of deleting the data.
    d->deleteAllIncidences(Incidence::TypeEvent);
    d->deleteAllIncidences(Incidence::TypeTodo);
    d->deleteAllIncidences(Incidence::TypeJournal);

    d->mIncidencesByIdentifier.clear();
    for (auto &table : d->mDeletedIncidences) {
        table.clear();
    }

    clearNotebookAssociations();

    setModified(false);

    setObserversEnabled(true);
}

bool MemoryCalendar::deleteIncidence(const Incidence::Ptr &incidence)
{
    // Handle orphaned children
    // relations is an Incidence's property, not a Todo's, so
    // we remove relations in deleteIncidence, not in deleteTodo.
    removeRelations(incidence);
    // Notify while the incidence is still available,
    // this is necessary so korganizer still has time to query for exceptions
    notifyIncidenceAboutToBeDeleted(incidence);
    incidence->unRegisterObserver(this);
    const Incidence::IncidenceType type = incidence->type();
    const QString &uid = incidence->uid();
    bool deleted = d->deleteIncidence(uid, type, incidence->recurrenceId());
    if (deleted) {
        setModified(true);
        if (deletionTracking()) {
            d->mDeletedIncidences[type].insert(uid, incidence);
        }

        // Delete child-incidences.
        if (!incidence->hasRecurrenceId() && incidence->recurs()) {
            deleteIncidenceInstances(incidence);
        }
    } else {
        qWarning() << incidence->typeStr() << " not found. uid=" << uid;
    }
    notifyIncidenceDeleted(incidence);
    return deleted;
}

bool MemoryCalendar::deleteIncidenceInstances(const Incidence::Ptr &incidence)
{
    Incidence::List instances;
    for (auto it = d->mIncidences[incidence->type()].constFind(incidence->uid()), end = d->mIncidences[incidence->type()].constEnd();
         it != end && it.key() == incidence->uid();
         ++it) {
        if (it.value()->hasRecurrenceId()) {
            qDebug() << "deleting child"
                     << ", type=" << int(incidence->type()) << ", uid="
                     << incidence->uid()
                     //                   << ", start=" << i->dtStart()
                     << " from calendar";
            // Don't call deleteIncidence() now since it's modifying the
            // mIncidences map we're iterating over.
            instances.append(it.value());
        }
    }
    for (Incidence::Ptr instance : instances) {
        deleteIncidence(instance);
    }

    return true;
}

//@cond PRIVATE
bool MemoryCalendar::Private::deleteIncidence(const QString &uid, IncidenceBase::IncidenceType type, const QDateTime &recurrenceId)
{
    for (auto it = mIncidences[type].find(uid), end = mIncidences[type].end(); it != end && it.key() == uid; ++it) {
        Incidence::Ptr incidence = it.value();
        if (recurrenceId.isNull() && incidence->hasRecurrenceId()) {
            continue;
        } else if (!recurrenceId.isNull() && (!incidence->hasRecurrenceId() || recurrenceId != incidence->recurrenceId())) {
            continue;
        }
        mIncidences[type].erase(it);
        mIncidencesByIdentifier.remove(incidence->instanceIdentifier());
        const QDateTime dt = incidence->dateTime(Incidence::RoleCalendarHashing);
        if (dt.isValid()) {
            mIncidencesForDate[type].remove(dt.toTimeZone(q->timeZone()).date(), incidence);
        }
        return true;
    }
    return false;
}

void MemoryCalendar::Private::deleteAllIncidences(Incidence::IncidenceType incidenceType)
{
    for (auto &incidence : mIncidences[incidenceType]) {
        q->notifyIncidenceAboutToBeDeleted(incidence);
        incidence->unRegisterObserver(q);
    }
    mIncidences[incidenceType].clear();
    mIncidencesForDate[incidenceType].clear();
}

Incidence::Ptr MemoryCalendar::Private::incidence(const QString &uid, Incidence::IncidenceType type, const QDateTime &recurrenceId) const
{
    return findIncidence(mIncidences[type], uid, recurrenceId);
}

Incidence::Ptr MemoryCalendar::Private::deletedIncidence(const QString &uid, const QDateTime &recurrenceId, IncidenceBase::IncidenceType type) const
{
    if (!q->deletionTracking()) {
        return Incidence::Ptr();
    }

    return findIncidence(mDeletedIncidences[type], uid, recurrenceId);
}

void MemoryCalendar::Private::insertIncidence(const Incidence::Ptr &incidence)
{
    const QString uid = incidence->uid();
    const Incidence::IncidenceType type = incidence->type();
    if (!mIncidences[type].contains(uid, incidence)) {
        mIncidences[type].insert(uid, incidence);
        mIncidencesByIdentifier.insert(incidence->instanceIdentifier(), incidence);
        const QDateTime dt = incidence->dateTime(Incidence::RoleCalendarHashing);
        if (dt.isValid()) {
            mIncidencesForDate[type].insert(dt.toTimeZone(q->timeZone()).date(), incidence);
        }

    } else {
#ifndef NDEBUG
        // if we already have an to-do with this UID, it must be the same incidence,
        // otherwise something's really broken
        Q_ASSERT(mIncidences[type].value(uid) == incidence);
#endif
    }
}
//@endcond

bool MemoryCalendar::addIncidence(const Incidence::Ptr &incidence)
{
    d->insertIncidence(incidence);

    notifyIncidenceAdded(incidence);

    incidence->registerObserver(this);

    setupRelations(incidence);

    setModified(true);

    return true;
}

bool MemoryCalendar::addEvent(const Event::Ptr &event)
{
    return addIncidence(event);
}

bool MemoryCalendar::deleteEvent(const Event::Ptr &event)
{
    return deleteIncidence(event);
}

bool MemoryCalendar::deleteEventInstances(const Event::Ptr &event)
{
    return deleteIncidenceInstances(event);
}

Event::Ptr MemoryCalendar::event(const QString &uid, const QDateTime &recurrenceId) const
{
    return d->incidence(uid, Incidence::TypeEvent, recurrenceId).staticCast<Event>();
}

Event::Ptr MemoryCalendar::deletedEvent(const QString &uid, const QDateTime &recurrenceId) const
{
    return d->deletedIncidence(uid, recurrenceId, Incidence::TypeEvent).staticCast<Event>();
}

bool MemoryCalendar::addTodo(const Todo::Ptr &todo)
{
    return addIncidence(todo);
}

bool MemoryCalendar::deleteTodo(const Todo::Ptr &todo)
{
    return deleteIncidence(todo);
}

bool MemoryCalendar::deleteTodoInstances(const Todo::Ptr &todo)
{
    return deleteIncidenceInstances(todo);
}

Todo::Ptr MemoryCalendar::todo(const QString &uid, const QDateTime &recurrenceId) const
{
    return d->incidence(uid, Incidence::TypeTodo, recurrenceId).staticCast<Todo>();
}

Todo::Ptr MemoryCalendar::deletedTodo(const QString &uid, const QDateTime &recurrenceId) const
{
    return d->deletedIncidence(uid, recurrenceId, Incidence::TypeTodo).staticCast<Todo>();
}

Todo::List MemoryCalendar::rawTodos(TodoSortField sortField, SortDirection sortDirection) const
{
    return Calendar::sortTodos(d->castIncidenceList<Todo>(d->mIncidences[Incidence::TypeTodo]), sortField, sortDirection);
}

Todo::List MemoryCalendar::deletedTodos(TodoSortField sortField, SortDirection sortDirection) const
{
    if (!deletionTracking()) {
        return Todo::List();
    }

    return Calendar::sortTodos(d->castIncidenceList<Todo>(d->mDeletedIncidences[Incidence::TypeTodo]), sortField, sortDirection);
}

Todo::List MemoryCalendar::todoInstances(const Incidence::Ptr &todo, TodoSortField sortField, SortDirection sortDirection) const
{
    return Calendar::sortTodos(d->incidenceInstances<Todo>(Incidence::TypeTodo, todo), sortField, sortDirection);
}

Todo::List MemoryCalendar::rawTodosForDate(const QDate &date) const
{
    Todo::List todoList;

    d->forIncidences<Todo>(d->mIncidencesForDate[Incidence::TypeTodo], date, [&todoList](const Todo::Ptr &todo) {
        todoList.append(todo);
    });

    // Iterate over all todos. Look for recurring todoss that occur on this date
    d->forIncidences<Todo>(d->mIncidences[Incidence::TypeTodo], [this, &todoList, &date](const Todo::Ptr &todo) {
        if (todo->recurs() && todo->recursOn(date, timeZone())) {
            todoList.append(todo);
        }
    });

    return todoList;
}

Todo::List MemoryCalendar::rawTodos(const QDate &start, const QDate &end, const QTimeZone &timeZone, bool inclusive) const
{
    Q_UNUSED(inclusive); // use only exact dtDue/dtStart, not dtStart and dtEnd

    Todo::List todoList;
    const auto ts = timeZone.isValid() ? timeZone : this->timeZone();
    QDateTime st(start, QTime(0, 0, 0), ts);
    QDateTime nd(end, QTime(23, 59, 59, 999), ts);

    // Get todos
    for (const auto &incidence : d->mIncidences[Incidence::TypeTodo]) {
        const auto todo = incidence.staticCast<Todo>();
        if (!isVisible(todo)) {
            continue;
        }

        QDateTime rStart = todo->hasDueDate() ? todo->dtDue() : todo->hasStartDate() ? todo->dtStart() : QDateTime();
        if (!rStart.isValid()) {
            continue;
        }

        if (!todo->recurs()) { // non-recurring todos
            if (nd.isValid() && nd < rStart) {
                continue;
            }
            if (st.isValid() && rStart < st) {
                continue;
            }
        } else { // recurring events
            switch (todo->recurrence()->duration()) {
            case -1: // infinite
                break;
            case 0: // end date given
            default: // count given
                QDateTime rEnd(todo->recurrence()->endDate(), QTime(23, 59, 59, 999), ts);
                if (!rEnd.isValid()) {
                    continue;
                }
                if (st.isValid() && rEnd < st) {
                    continue;
                }
                break;
            } // switch(duration)
        } // if(recurs)

        todoList.append(todo);
    }

    return todoList;
}

Alarm::List MemoryCalendar::alarmsTo(const QDateTime &to) const
{
    return alarms(QDateTime(QDate(1900, 1, 1), QTime(0, 0, 0)), to);
}

Alarm::List MemoryCalendar::alarms(const QDateTime &from, const QDateTime &to, bool excludeBlockedAlarms) const
{
    Q_UNUSED(excludeBlockedAlarms);
    Alarm::List alarmList;

    d->forIncidences<Event>(d->mIncidences[Incidence::TypeEvent], [this, &alarmList, &from, &to](const Event::Ptr &e) {
        if (e->recurs()) {
            appendRecurringAlarms(alarmList, e, from, to);
        } else {
            appendAlarms(alarmList, e, from, to);
        }
    });

    d->forIncidences<Todo>(d->mIncidences[IncidenceBase::TypeTodo], [this, &alarmList, &from, &to](const Todo::Ptr &t) {
        if (!t->isCompleted()) {
            appendAlarms(alarmList, t, from, to);
            if (t->recurs()) {
                appendRecurringAlarms(alarmList, t, from, to);
            } else {
                appendAlarms(alarmList, t, from, to);
            }
        }
    });

    return alarmList;
}

bool MemoryCalendar::updateLastModifiedOnChange() const
{
    return d->mUpdateLastModified;
}

void MemoryCalendar::setUpdateLastModifiedOnChange(bool update)
{
    d->mUpdateLastModified = update;
}

void MemoryCalendar::incidenceUpdate(const QString &uid, const QDateTime &recurrenceId)
{
    Incidence::Ptr inc = incidence(uid, recurrenceId);

    if (inc) {
        if (!d->mIncidenceBeingUpdated.isEmpty()) {
            qWarning() << "Incidence::update() called twice without an updated() call in between.";
        }

        // Save it so we can detect changes to uid or recurringId.
        d->mIncidenceBeingUpdated = inc->instanceIdentifier();

        const QDateTime dt = inc->dateTime(Incidence::RoleCalendarHashing);
        if (dt.isValid()) {
            d->mIncidencesForDate[inc->type()].remove(dt.toTimeZone(timeZone()).date(), inc);
        }
    }
}

void MemoryCalendar::incidenceUpdated(const QString &uid, const QDateTime &recurrenceId)
{
    Incidence::Ptr inc = incidence(uid, recurrenceId);

    if (inc) {
        if (d->mIncidenceBeingUpdated.isEmpty()) {
            qWarning() << "Incidence::updated() called twice without an update() call in between.";
        } else if (inc->instanceIdentifier() != d->mIncidenceBeingUpdated) {
            // Instance identifier changed, update our hash table
            d->mIncidencesByIdentifier.remove(d->mIncidenceBeingUpdated);
            d->mIncidencesByIdentifier.insert(inc->instanceIdentifier(), inc);
        }

        d->mIncidenceBeingUpdated = QString();

        if (d->mUpdateLastModified) {
            inc->setLastModified(QDateTime::currentDateTimeUtc());
        }
        // we should probably update the revision number here,
        // or internally in the Event itself when certain things change.
        // need to verify with ical documentation.

        const QDateTime dt = inc->dateTime(Incidence::RoleCalendarHashing);
        if (dt.isValid()) {
            d->mIncidencesForDate[inc->type()].insert(dt.toTimeZone(timeZone()).date(), inc);
        }

        notifyIncidenceChanged(inc);

        setModified(true);
    }
}

Event::List MemoryCalendar::rawEventsForDate(const QDate &date, const QTimeZone &timeZone, EventSortField sortField, SortDirection sortDirection) const
{
    Event::List eventList;

    if (!date.isValid()) {
        // There can't be events on invalid dates
        return eventList;
    }

    if (timeZone.isValid() && timeZone != this->timeZone()) {
        // We cannot use the hash table on date, since time zone is different.
        eventList = rawEvents(date, date, timeZone, false);
        return Calendar::sortEvents(eventList, sortField, sortDirection);
    }

    // Iterate over all non-recurring, single-day events that start on this date
    d->forIncidences<Event>(d->mIncidencesForDate[Incidence::TypeEvent], date, [&eventList](const Event::Ptr &event) {
        eventList.append(event);
    });

    // Iterate over all events. Look for recurring events that occur on this date
    const auto ts = timeZone.isValid() ? timeZone : this->timeZone();
    for (const auto &event : d->mIncidences[Incidence::TypeEvent]) {
        const auto ev = event.staticCast<Event>();
        if (ev->recurs()) {
            if (ev->isMultiDay()) {
                int extraDays = ev->dtStart().date().daysTo(ev->dtEnd().date());
                for (int i = 0; i <= extraDays; ++i) {
                    if (ev->recursOn(date.addDays(-i), ts)) {
                        eventList.append(ev);
                        break;
                    }
                }
            } else {
                if (ev->recursOn(date, ts)) {
                    eventList.append(ev);
                }
            }
        } else {
            if (ev->isMultiDay()) {
                if (ev->dtStart().toTimeZone(ts).date() <= date && ev->dtEnd().toTimeZone(ts).date() >= date) {
                    eventList.append(ev);
                }
            }
        }
    }

    return Calendar::sortEvents(eventList, sortField, sortDirection);
}

Event::List MemoryCalendar::rawEvents(const QDate &start, const QDate &end, const QTimeZone &timeZone, bool inclusive) const
{
    Event::List eventList;
    const auto ts = timeZone.isValid() ? timeZone : this->timeZone();
    QDateTime st(start, QTime(0, 0, 0), ts);
    QDateTime nd(end, QTime(23, 59, 59, 999), ts);

    // Get non-recurring events
    for (const auto &e : d->mIncidences[Incidence::TypeEvent]) {
        const auto event = e.staticCast<Event>();
        QDateTime rStart = event->dtStart();
        if (nd.isValid() && nd < rStart) {
            continue;
        }
        if (inclusive && st.isValid() && rStart < st) {
            continue;
        }

        if (!event->recurs()) { // non-recurring events
            QDateTime rEnd = event->dtEnd();
            if (st.isValid() && rEnd < st) {
                continue;
            }
            if (inclusive && nd.isValid() && nd < rEnd) {
                continue;
            }
        } else { // recurring events
            switch (event->recurrence()->duration()) {
            case -1: // infinite
                if (inclusive) {
                    continue;
                }
                break;
            case 0: // end date given
            default: // count given
                QDateTime rEnd(event->recurrence()->endDate(), QTime(23, 59, 59, 999), ts);
                if (!rEnd.isValid()) {
                    continue;
                }
                if (st.isValid() && rEnd < st) {
                    continue;
                }
                if (inclusive && nd.isValid() && nd < rEnd) {
                    continue;
                }
                break;
            } // switch(duration)
        } // if(recurs)

        eventList.append(event);
    }

    return eventList;
}

Event::List MemoryCalendar::rawEventsForDate(const QDateTime &kdt) const
{
    return rawEventsForDate(kdt.date(), kdt.timeZone());
}

Event::List MemoryCalendar::rawEvents(EventSortField sortField, SortDirection sortDirection) const
{
    return Calendar::sortEvents(d->castIncidenceList<Event>(d->mIncidences[Incidence::TypeEvent]), sortField, sortDirection);
}

Event::List MemoryCalendar::deletedEvents(EventSortField sortField, SortDirection sortDirection) const
{
    if (!deletionTracking()) {
        return Event::List();
    }

    return Calendar::sortEvents(d->castIncidenceList<Event>(d->mDeletedIncidences[Incidence::TypeEvent]), sortField, sortDirection);
}

Event::List MemoryCalendar::eventInstances(const Incidence::Ptr &event, EventSortField sortField, SortDirection sortDirection) const
{
    return Calendar::sortEvents(d->incidenceInstances<Event>(Incidence::TypeEvent, event), sortField, sortDirection);
}

bool MemoryCalendar::addJournal(const Journal::Ptr &journal)
{
    return addIncidence(journal);
}

bool MemoryCalendar::deleteJournal(const Journal::Ptr &journal)
{
    return deleteIncidence(journal);
}

bool MemoryCalendar::deleteJournalInstances(const Journal::Ptr &journal)
{
    return deleteIncidenceInstances(journal);
}

Journal::Ptr MemoryCalendar::journal(const QString &uid, const QDateTime &recurrenceId) const
{
    return d->incidence(uid, Incidence::TypeJournal, recurrenceId).staticCast<Journal>();
}

Journal::Ptr MemoryCalendar::deletedJournal(const QString &uid, const QDateTime &recurrenceId) const
{
    return d->deletedIncidence(uid, recurrenceId, Incidence::TypeJournal).staticCast<Journal>();
}

Journal::List MemoryCalendar::rawJournals(JournalSortField sortField, SortDirection sortDirection) const
{
    return Calendar::sortJournals(d->castIncidenceList<Journal>(d->mIncidences[Incidence::TypeJournal]), sortField, sortDirection);
}

Journal::List MemoryCalendar::deletedJournals(JournalSortField sortField, SortDirection sortDirection) const
{
    if (!deletionTracking()) {
        return Journal::List();
    }

    return Calendar::sortJournals(d->castIncidenceList<Journal>(d->mDeletedIncidences[Incidence::TypeJournal]), sortField, sortDirection);
}

Journal::List MemoryCalendar::journalInstances(const Incidence::Ptr &journal, JournalSortField sortField, SortDirection sortDirection) const
{
    return Calendar::sortJournals(d->incidenceInstances<Journal>(Incidence::TypeJournal, journal), sortField, sortDirection);
}

Journal::List MemoryCalendar::rawJournalsForDate(const QDate &date) const
{
    Journal::List journalList;

    d->forIncidences<Journal>(d->mIncidencesForDate[Incidence::TypeJournal], date, [&journalList](const Journal::Ptr &journal) {
        journalList.append(journal);
    });

    return journalList;
}

Incidence::Ptr MemoryCalendar::instance(const QString &identifier) const
{
    return d->mIncidencesByIdentifier.value(identifier);
}

void MemoryCalendar::virtual_hook(int id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    Q_ASSERT(false);
}
