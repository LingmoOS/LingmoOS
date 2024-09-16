/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001-2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2009 Allen Winter <winter@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Todo class.

  @brief
  Provides a To-do in the sense of RFC2445.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Allen Winter \<winter@kde.org\>
*/

#include "todo.h"
#include "recurrence.h"
#include "utils_p.h"
#include "visitor.h"

#include <QTime>
#include <QDebug>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Todo::Private
{
public:
    Private()
    {
    }
    Private(const KCalendarCore::Todo::Private &other)
    {
        init(other);
    }

    void init(const KCalendarCore::Todo::Private &other);

    QDateTime mDtDue; // to-do due date (if there is one)
    // ALSO the first occurrence of a recurring to-do
    QDateTime mDtRecurrence; // next occurrence (for recurring to-dos)
    QDateTime mCompleted; // to-do completion date (if it has been completed)
    int mPercentComplete = 0; // to-do percent complete [0,100]

    /**
      Returns true if the todo got a new date, else false will be returned.
    */
    bool recurTodo(Todo *todo);
};

void KCalendarCore::Todo::Private::init(const KCalendarCore::Todo::Private &other)
{
    mDtDue = other.mDtDue;
    mDtRecurrence = other.mDtRecurrence;
    mCompleted = other.mCompleted;
    mPercentComplete = other.mPercentComplete;
}

//@endcond

Todo::Todo()
    : d(new KCalendarCore::Todo::Private)
{
}

Todo::Todo(const Todo &other)
    : Incidence(other)
    , d(new KCalendarCore::Todo::Private(*other.d))
{
}

Todo::Todo(const Incidence &other)
    : Incidence(other)
    , d(new KCalendarCore::Todo::Private)
{
}

Todo::~Todo()
{
    delete d;
}

Todo *Todo::clone() const
{
    return new Todo(*this);
}

IncidenceBase &Todo::assign(const IncidenceBase &other)
{
    if (&other != this) {
        Incidence::assign(other);
        const Todo *t = static_cast<const Todo *>(&other);
        d->init(*(t->d));
    }
    return *this;
}

bool Todo::equals(const IncidenceBase &todo) const
{
    if (!Incidence::equals(todo)) {
        return false;
    } else {
        // If they weren't the same type IncidenceBase::equals would had returned false already
        const Todo *t = static_cast<const Todo *>(&todo);
        return ((dtDue() == t->dtDue()) || (!dtDue().isValid() && !t->dtDue().isValid())) && hasDueDate() == t->hasDueDate()
               && hasStartDate() == t->hasStartDate() && ((completed() == t->completed()) || (!completed().isValid() && !t->completed().isValid()))
               && hasCompletedDate() == t->hasCompletedDate() && percentComplete() == t->percentComplete();
    }
}

Incidence::IncidenceType Todo::type() const
{
    return TypeTodo;
}

QByteArray Todo::typeStr() const
{
    return QByteArrayLiteral("Todo");
}
void Todo::setDtDue(const QDateTime &dtDue, bool first)
{
    startUpdates();

    // int diffsecs = d->mDtDue.secsTo(dtDue);

    /*if (mReadOnly) return;
    const Alarm::List& alarms = alarms();
    for (Alarm *alarm = alarms.first(); alarm; alarm = alarms.next()) {
      if (alarm->enabled()) {
        alarm->setTime(alarm->time().addSecs(diffsecs));
      }
    }*/

    if (recurs() && !first) {
        d->mDtRecurrence = dtDue;
    } else {
        d->mDtDue = dtDue;
    }

    if (recurs() && dtDue.isValid() && (!dtStart().isValid() || dtDue < recurrence()->startDateTime())) {
        qDebug() << "To-do recurrences are now calculated against DTSTART. Fixing legacy to-do.";
        setDtStart(dtDue);
    }

    /*const Alarm::List& alarms = alarms();
    for (Alarm *alarm = alarms.first(); alarm; alarm = alarms.next())
      alarm->setAlarmStart(d->mDtDue);*/
    setFieldDirty(FieldDtDue);
    endUpdates();
}

QDateTime Todo::dtDue(bool first) const
{
    if (!hasDueDate()) {
        return QDateTime();
    }

    const QDateTime start = IncidenceBase::dtStart();
    if (recurs() && !first && d->mDtRecurrence.isValid()) {
        if (start.isValid()) {
            // This is the normal case, recurring to-dos have a valid DTSTART.
            const qint64 duration = start.daysTo(d->mDtDue);
            QDateTime dt = d->mDtRecurrence.addDays(duration);
            dt.setTime(d->mDtDue.time());
            return dt;
        } else {
            // This is a legacy case, where recurrence was calculated against DTDUE
            return d->mDtRecurrence;
        }
    }

    return d->mDtDue;
}

bool Todo::hasDueDate() const
{
    return d->mDtDue.isValid();
}

bool Todo::hasStartDate() const
{
    return IncidenceBase::dtStart().isValid();
}

QDateTime Todo::dtStart() const
{
    return dtStart(/*first=*/false);
}

QDateTime Todo::dtStart(bool first) const
{
    if (!hasStartDate()) {
        return QDateTime();
    }

    if (recurs() && !first && d->mDtRecurrence.isValid()) {
        return d->mDtRecurrence;
    } else {
        return IncidenceBase::dtStart();
    }
}

bool Todo::isCompleted() const
{
    return d->mPercentComplete == 100 || status() == StatusCompleted || hasCompletedDate();
}

void Todo::setCompleted(bool completed)
{
    update();
    if (completed) {
        d->mPercentComplete = 100;
    } else {
        d->mPercentComplete = 0;
        if (hasCompletedDate()) {
            d->mCompleted = QDateTime();
            setFieldDirty(FieldCompleted);
        }
    }
    setFieldDirty(FieldPercentComplete);
    updated();

    setStatus(completed ? StatusCompleted : StatusNone); // Calls update()/updated().
}

QDateTime Todo::completed() const
{
    if (hasCompletedDate()) {
        return d->mCompleted;
    } else {
        return QDateTime();
    }
}

void Todo::setCompleted(const QDateTime &completed)
{
    update();
    if (!d->recurTodo(this)) {
        if (d->mPercentComplete != 100) {
            d->mPercentComplete = 100;
            setFieldDirty(FieldPercentComplete);
        }
        if (d->mCompleted.isValid() != completed.isValid()) {
            d->mCompleted = completed.toUTC();
            setFieldDirty(FieldCompleted);
        }
    }
    updated();
    if (status() != StatusNone) {
        setStatus(StatusCompleted); // Calls update()/updated()
    }
}

bool Todo::hasCompletedDate() const
{
    return d->mCompleted.isValid();
}

int Todo::percentComplete() const
{
    return d->mPercentComplete;
}

void Todo::setPercentComplete(int percent)
{
    if (percent > 100) {
        percent = 100;
    } else if (percent < 0) {
        percent = 0;
    }

    update();
    if (percent != d->mPercentComplete) {
        d->mPercentComplete = percent;
        setFieldDirty(FieldPercentComplete);
    }
    if (percent != 100 && d->mCompleted.isValid()) {
        d->mCompleted = QDateTime();
        setFieldDirty(FieldCompleted);
    }
    updated();
    if (percent != 100 && status() == Incidence::StatusCompleted) {
        setStatus(Incidence::StatusNone); // Calls update()/updated().
    }
}

bool Todo::isInProgress(bool first) const
{
    if (isOverdue()) {
        return false;
    }

    if (d->mPercentComplete > 0) {
        return true;
    }

    if (hasStartDate() && hasDueDate()) {
        if (allDay()) {
            QDate currDate = QDate::currentDate();
            if (dtStart(first).date() <= currDate && currDate < dtDue(first).date()) {
                return true;
            }
        } else {
            QDateTime currDate = QDateTime::currentDateTimeUtc();
            if (dtStart(first) <= currDate && currDate < dtDue(first)) {
                return true;
            }
        }
    }

    return false;
}

bool Todo::isOpenEnded() const
{
    if (!hasDueDate() && !isCompleted()) {
        return true;
    }
    return false;
}

bool Todo::isNotStarted(bool first) const
{
    if (d->mPercentComplete > 0) {
        return false;
    }

    if (!hasStartDate()) {
        return false;
    }

    if (allDay()) {
        if (dtStart(first).date() >= QDate::currentDate()) {
            return false;
        }
    } else {
        if (dtStart(first) >= QDateTime::currentDateTimeUtc()) {
            return false;
        }
    }
    return true;
}

void Todo::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    Incidence::shiftTimes(oldZone, newZone);
    d->mDtDue = d->mDtDue.toTimeZone(oldZone);
    d->mDtDue.setTimeZone(newZone);
    if (recurs()) {
        d->mDtRecurrence = d->mDtRecurrence.toTimeZone(oldZone);
        d->mDtRecurrence.setTimeZone(newZone);
    }
    if (hasCompletedDate()) {
        d->mCompleted = d->mCompleted.toTimeZone(oldZone);
        d->mCompleted.setTimeZone(newZone);
    }
}

void Todo::setDtRecurrence(const QDateTime &dt)
{
    d->mDtRecurrence = dt;
    setFieldDirty(FieldRecurrence);
}

QDateTime Todo::dtRecurrence() const
{
    return d->mDtRecurrence.isValid() ? d->mDtRecurrence : d->mDtDue;
}

bool Todo::recursOn(const QDate &date, const QTimeZone &timeZone) const
{
    QDate today = QDate::currentDate();
    return Incidence::recursOn(date, timeZone) && !(date < today && d->mDtRecurrence.date() < today && d->mDtRecurrence > recurrence()->startDateTime());
}

bool Todo::isOverdue() const
{
    if (!dtDue().isValid()) {
        return false; // if it's never due, it can't be overdue
    }

    const bool inPast = allDay() ? dtDue().date() < QDate::currentDate() : dtDue() < QDateTime::currentDateTimeUtc();

    return inPast && !isCompleted();
}

void Todo::setAllDay(bool allday)
{
    if (allday != allDay() && !mReadOnly) {
        if (hasDueDate()) {
            setFieldDirty(FieldDtDue);
        }
        Incidence::setAllDay(allday);
    }
}

//@cond PRIVATE
bool Todo::Private::recurTodo(Todo *todo)
{
    if (todo && todo->recurs()) {
        Recurrence *r = todo->recurrence();
        const QDateTime recurrenceEndDateTime = r->endDateTime();
        QDateTime nextOccurrenceDateTime = r->getNextDateTime(todo->dtStart());

        if ((r->duration() == -1 || (nextOccurrenceDateTime.isValid() && recurrenceEndDateTime.isValid() && nextOccurrenceDateTime <= recurrenceEndDateTime))) {
            // We convert to the same timeSpec so we get the correct .date()
            const auto rightNow = QDateTime::currentDateTimeUtc().toTimeZone(nextOccurrenceDateTime.timeZone());
            const bool isDateOnly = todo->allDay();

            /* Now we search for the occurrence that's _after_ the currentUtcDateTime, or
             * if it's dateOnly, the occurrrence that's _during or after today_.
             * The reason we use "<" for date only, but "<=" for occurrences with time is that
             * if it's date only, the user can still complete that occurrence today, so that's
             * the current occurrence that needs completing.
             */
            while (!todo->recursAt(nextOccurrenceDateTime) || (!isDateOnly && nextOccurrenceDateTime <= rightNow)
                   || (isDateOnly && nextOccurrenceDateTime.date() < rightNow.date())) {
                if (!nextOccurrenceDateTime.isValid() || (nextOccurrenceDateTime > recurrenceEndDateTime && r->duration() != -1)) {
                    return false;
                }
                nextOccurrenceDateTime = r->getNextDateTime(nextOccurrenceDateTime);
            }

            todo->setDtRecurrence(nextOccurrenceDateTime);
            todo->setCompleted(false);
            todo->setRevision(todo->revision() + 1);

            return true;
        }
    }

    return false;
}
//@endcond

bool Todo::accept(Visitor &v, const IncidenceBase::Ptr &incidence)
{
    return v.visit(incidence.staticCast<Todo>());
}

QDateTime Todo::dateTime(DateTimeRole role) const
{
    switch (role) {
    case RoleAlarmStartOffset:
        return dtStart();
    case RoleAlarmEndOffset:
        return dtDue();
    case RoleSort:
        // Sorting to-dos first compares dtDue, then dtStart if
        // dtDue doesn't exist
        return hasDueDate() ? dtDue() : dtStart();
    case RoleCalendarHashing:
        return dtDue();
    case RoleStartTimeZone:
        return dtStart();
    case RoleEndTimeZone:
        return dtDue();
    case RoleEndRecurrenceBase:
        return dtDue();
    case RoleDisplayStart:
    case RoleDisplayEnd:
        return dtDue().isValid() ? dtDue() : dtStart();
    case RoleAlarm:
        if (alarms().isEmpty()) {
            return QDateTime();
        } else {
            Alarm::Ptr alarm = alarms().at(0);
            if (alarm->hasStartOffset() && hasStartDate()) {
                return dtStart();
            } else if (alarm->hasEndOffset() && hasDueDate()) {
                return dtDue();
            } else {
                // The application shouldn't add alarms on to-dos without dates.
                return QDateTime();
            }
        }
    case RoleRecurrenceStart:
        if (dtStart().isValid()) {
            return dtStart();
        }
        return dtDue(); // For the sake of backwards compatibility
    // where we calculated recurrences based on dtDue
    case RoleEnd:
        return dtDue();
    default:
        return QDateTime();
    }
}

void Todo::setDateTime(const QDateTime &dateTime, DateTimeRole role)
{
    switch (role) {
    case RoleDnD:
        setDtDue(dateTime);
        break;
    case RoleEnd:
        setDtDue(dateTime, true);
        break;
    default:
        qDebug() << "Unhandled role" << role;
    }
}

void Todo::virtual_hook(VirtualHook id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
}

QLatin1String Todo::mimeType() const
{
    return Todo::todoMimeType();
}

QLatin1String Todo::todoMimeType()
{
    return QLatin1String("application/x-vnd.akonadi.calendar.todo");
}

QLatin1String Todo::iconName(const QDateTime &recurrenceId) const
{
    const bool usesCompletedTaskPixmap = isCompleted() || (recurs() && recurrenceId.isValid() && (recurrenceId < dtStart(/*first=*/false)));

    if (usesCompletedTaskPixmap) {
        return QLatin1String("task-complete");
    } else {
        return QLatin1String("view-calendar-tasks");
    }
}

void Todo::serialize(QDataStream &out) const
{
    Incidence::serialize(out);
    serializeQDateTimeAsKDateTime(out, d->mDtDue);
    serializeQDateTimeAsKDateTime(out, d->mDtRecurrence);
    serializeQDateTimeAsKDateTime(out, d->mCompleted);
    out << d->mPercentComplete;
}

void Todo::deserialize(QDataStream &in)
{
    Incidence::deserialize(in);
    deserializeKDateTimeAsQDateTime(in, d->mDtDue);
    deserializeKDateTimeAsQDateTime(in, d->mDtRecurrence);
    deserializeKDateTimeAsQDateTime(in, d->mCompleted);
    in >> d->mPercentComplete;
}

bool Todo::supportsGroupwareCommunication() const
{
    return true;
}
