/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2013 Christian Mollekopf <mollekopf@kolabsys.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the OccurrenceIterator class.

  @brief
  This class provides an iterator to iterate over all occurrences of incidences.

  @author Christian Mollekopf \<mollekopf@kolabsys.com\>
 */

#include "occurrenceiterator.h"
#include "calendar.h"
#include "calfilter.h"

#include <QDate>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::OccurrenceIterator::Private
{
public:
    Private(OccurrenceIterator *qq)
        : q(qq)
        , occurrenceIt(occurrenceList)
    {
    }

    OccurrenceIterator *q;
    QDateTime start;
    QDateTime end;

    struct Occurrence {
        Occurrence()
        {
        }

        Occurrence(const Incidence::Ptr &i, const QDateTime &recurrenceId, const QDateTime &startDate)
            : incidence(i)
            , recurrenceId(recurrenceId)
            , startDate(startDate)
        {
        }

        Incidence::Ptr incidence;
        QDateTime recurrenceId;
        QDateTime startDate;
    };
    QList<Occurrence> occurrenceList;
    QListIterator<Occurrence> occurrenceIt;
    Occurrence current;

    /*
     * KCalendarCore::CalFilter can't handle individual occurrences.
     * When filtering completed to-dos, the CalFilter doesn't hide
     * them if it's a recurring to-do.
     */
    bool occurrenceIsHidden(const Calendar &calendar, const Incidence::Ptr &inc, const QDateTime &occurrenceDate)
    {
        if ((inc->type() == Incidence::TypeTodo) && calendar.filter() && (calendar.filter()->criteria() & KCalendarCore::CalFilter::HideCompletedTodos)) {
            if (inc->recurs()) {
                const Todo::Ptr todo = inc.staticCast<Todo>();
                if (todo && (occurrenceDate < todo->dtDue())) {
                    return true;
                }
            } else if (inc->hasRecurrenceId()) {
                const Todo::Ptr mainTodo = calendar.todo(inc->uid());
                if (mainTodo && mainTodo->isCompleted()) {
                    return true;
                }
            }
        }
        return false;
    }

    void setupIterator(const Calendar &calendar, const Incidence::List &incidences)
    {
        for (const Incidence::Ptr &inc : qAsConst(incidences)) {
            if (inc->hasRecurrenceId()) {
                continue;
            }
            if (inc->recurs()) {
                QHash<QDateTime, Incidence::Ptr> recurrenceIds;
                QDateTime incidenceRecStart = inc->dateTime(Incidence::RoleRecurrenceStart);
                // const bool isAllDay = inc->allDay();
                const auto lstInstances = calendar.instances(inc);
                for (const Incidence::Ptr &exception : lstInstances) {
                    if (incidenceRecStart.isValid()) {
                        recurrenceIds.insert(exception->recurrenceId().toTimeZone(incidenceRecStart.timeZone()), exception);
                    }
                }
                const auto occurrences = inc->recurrence()->timesInInterval(start, end);
                Incidence::Ptr incidence(inc), lastInc(inc);
                qint64 offset(0), lastOffset(0);
                QDateTime occurrenceStartDate;
                for (const auto &recurrenceId : qAsConst(occurrences)) {
                    occurrenceStartDate = recurrenceId;

                    bool resetIncidence = false;
                    if (recurrenceIds.contains(recurrenceId)) {
                        // TODO: exclude exceptions where the start/end is not within
                        // (so the occurrence of the recurrence is omitted, but no exception is added)
                        if (recurrenceIds.value(recurrenceId)->status() == Incidence::StatusCanceled) {
                            continue;
                        }

                        incidence = recurrenceIds.value(recurrenceId);
                        occurrenceStartDate = incidence->dtStart();
                        resetIncidence = !incidence->thisAndFuture();
                        offset = incidence->recurrenceId().secsTo(incidence->dtStart());
                        if (incidence->thisAndFuture()) {
                            lastInc = incidence;
                            lastOffset = offset;
                        }
                    } else if (inc != incidence) { // thisAndFuture exception is active
                        occurrenceStartDate = occurrenceStartDate.addSecs(offset);
                    }

                    if (!occurrenceIsHidden(calendar, incidence, occurrenceStartDate)) {
                        occurrenceList << Private::Occurrence(incidence, recurrenceId, occurrenceStartDate);
                    }

                    if (resetIncidence) {
                        incidence = lastInc;
                        offset = lastOffset;
                    }
                }
            } else {
                occurrenceList << Private::Occurrence(inc, {}, inc->dtStart());
            }
        }
        occurrenceIt = QListIterator<Private::Occurrence>(occurrenceList);
    }
};
//@endcond

/**
 * Right now there is little point in the iterator, but:
 * With an iterator it should be possible to solve this more memory efficiently
 * and with immediate results at the beginning of the selected timeframe.
 * Either all events are iterated simoulatneously, resulting in occurrences
 * of all events in parallel in the correct time-order, or incidence after
 * incidence, which would be even more efficient.
 *
 * By making this class a friend of calendar, we could also use the internally
 * available data structures.
 */
OccurrenceIterator::OccurrenceIterator(const Calendar &calendar, const QDateTime &start, const QDateTime &end)
    : d(new KCalendarCore::OccurrenceIterator::Private(this))
{
    d->start = start;
    d->end = end;

    Event::List events = calendar.rawEvents(start.date(), end.date(), start.timeZone());
    if (calendar.filter()) {
        calendar.filter()->apply(&events);
    }

    Todo::List todos = calendar.rawTodos(start.date(), end.date(), start.timeZone());
    if (calendar.filter()) {
        calendar.filter()->apply(&todos);
    }

    Journal::List journals;
    const Journal::List allJournals = calendar.rawJournals();
    for (const KCalendarCore::Journal::Ptr &journal : allJournals) {
        const QDate journalStart = journal->dtStart().toTimeZone(start.timeZone()).date();
        if (journal->dtStart().isValid() && journalStart >= start.date() && journalStart <= end.date()) {
            journals << journal;
        }
    }

    if (calendar.filter()) {
        calendar.filter()->apply(&journals);
    }

    const Incidence::List incidences = KCalendarCore::Calendar::mergeIncidenceList(events, todos, journals);
    d->setupIterator(calendar, incidences);
}

OccurrenceIterator::OccurrenceIterator(const Calendar &calendar, const Incidence::Ptr &incidence, const QDateTime &start, const QDateTime &end)
    : d(new KCalendarCore::OccurrenceIterator::Private(this))
{
    Q_ASSERT(incidence);
    d->start = start;
    d->end = end;
    d->setupIterator(calendar, Incidence::List() << incidence);
}

OccurrenceIterator::~OccurrenceIterator()
{
}

bool OccurrenceIterator::hasNext() const
{
    return d->occurrenceIt.hasNext();
}

void OccurrenceIterator::next()
{
    d->current = d->occurrenceIt.next();
}

Incidence::Ptr OccurrenceIterator::incidence() const
{
    return d->current.incidence;
}

QDateTime OccurrenceIterator::occurrenceStartDate() const
{
    return d->current.startDate;
}

QDateTime OccurrenceIterator::recurrenceId() const
{
    return d->current.recurrenceId;
}
