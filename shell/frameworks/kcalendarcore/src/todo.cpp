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

#include "incidence_p.h"
#include "todo.h"
#include "recurrence.h"
#include "utils_p.h"
#include "visitor.h"

#include "kcalendarcore_debug.h"

#include <QTime>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalendarCore::TodoPrivate : public IncidencePrivate
{
    // Due date of the to-do or its first recurrence if it recurs;  invalid() <=> no defined due date.
    QDateTime mDtDue;
    QDateTime mDtRecurrence; // next occurrence (for recurring to-dos)
    QDateTime mCompleted; // to-do completion date (if it has been completed)
    int mPercentComplete = 0; // to-do percent complete [0,100]

public:

    TodoPrivate() = default;
    TodoPrivate(const TodoPrivate &other) = default;

    // Copy IncidencePrivate and IncidenceBasePrivate members,
    // but default-initialize TodoPrivate members.
    TodoPrivate(const Incidence &other)
        : IncidencePrivate(other)
    {
    }

    void init(const TodoPrivate &other);

    void setDtDue(const QDateTime dd);
    QDateTime dtDue() const
    {
        return mDtDue;
    }

    void setDtRecurrence(const QDateTime dr);
    QDateTime dtRecurrence() const
    {
        return mDtRecurrence;
    }

    void setCompleted(const QDateTime dc);
    QDateTime completed() const
    {
        return mCompleted;
    }

    void setPercentComplete(const int pc);
    int percentComplete() const
    {
        return mPercentComplete;
    }

    /**
      Returns true if the todo got a new date, else false will be returned.
    */
    bool recurTodo(Todo *todo);

    void deserialize(QDataStream &in);

    bool validStatus(Incidence::Status) override;
};

void TodoPrivate::setDtDue(const QDateTime dd)
{
    if (!identical(dd, mDtDue)) {
        mDtDue = dd;
        mDirtyFields.insert(IncidenceBase::FieldDtDue);
    }
}

void TodoPrivate::setDtRecurrence(const QDateTime dr)
{
    if (!identical(dr, mDtRecurrence)) {
        mDtRecurrence = dr;
        mDirtyFields.insert(IncidenceBase::FieldRecurrenceId);
    }
}

void TodoPrivate::setCompleted(const QDateTime dc)
{
    if (dc != mCompleted) {
        mCompleted = dc.toUTC();
        mDirtyFields.insert(IncidenceBase::FieldCompleted);
    }
}

void TodoPrivate::setPercentComplete(const int pc)
{
    if (pc != mPercentComplete) {
        mPercentComplete = pc;
        mDirtyFields.insert(IncidenceBase::FieldPercentComplete);
    }
}

void TodoPrivate::init(const TodoPrivate &other)
{
    mDtDue = other.mDtDue;
    mDtRecurrence = other.mDtRecurrence;
    mCompleted = other.mCompleted;
    mPercentComplete = other.mPercentComplete;
}

bool TodoPrivate::validStatus(Incidence::Status status)
{
    constexpr unsigned validSet
        = 1u << Incidence::StatusNone
        | 1u << Incidence::StatusNeedsAction
        | 1u << Incidence::StatusCompleted
        | 1u << Incidence::StatusInProcess
        | 1u << Incidence::StatusCanceled;
    return validSet & (1u << status);
}

//@endcond

Todo::Todo()
    : Incidence(new TodoPrivate())
{
}

Todo::Todo(const Todo &other)
    : Incidence(other, new TodoPrivate(*(other.d_func())))
{
}

Todo::Todo(const Incidence &other)
    : Incidence(other, new TodoPrivate(other))
{
}

Todo::~Todo() = default;

Todo *Todo::clone() const
{
    return new Todo(*this);
}

IncidenceBase &Todo::assign(const IncidenceBase &other)
{
    Q_D(Todo);
    if (&other != this) {
        Incidence::assign(other);
        const Todo *t = static_cast<const Todo *>(&other);
        d->init(*(t->d_func()));
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
        return identical(dtDue(), t->dtDue())
            && hasDueDate() == t->hasDueDate()
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

    Q_D(Todo);
    if (recurs() && !first) {
        d->setDtRecurrence(dtDue);
    } else {
        d->setDtDue(dtDue);
    }

    if (recurs() && dtDue.isValid() && (!dtStart().isValid() || dtDue < recurrence()->startDateTime())) {
        qCDebug(KCALCORE_LOG) << "To-do recurrences are now calculated against DTSTART. Fixing legacy to-do.";
        setDtStart(dtDue);
    }

    /*const Alarm::List& alarms = alarms();
    for (Alarm *alarm = alarms.first(); alarm; alarm = alarms.next())
      alarm->setAlarmStart(d->mDtDue);*/
    endUpdates();
}

QDateTime Todo::dtDue(bool first) const
{
    if (!hasDueDate()) {
        return QDateTime();
    }

    Q_D(const Todo);
    const QDateTime start = IncidenceBase::dtStart();
    if (recurs() && !first && d->dtRecurrence().isValid()) {
        if (start.isValid()) {
            // This is the normal case, recurring to-dos have a valid DTSTART.
            const qint64 duration = start.daysTo(d->dtDue());
            QDateTime dt = d->dtRecurrence().addDays(duration);
            dt.setTime(d->dtDue().time());
            return dt;
        } else {
            // This is a legacy case, where recurrence was calculated against DTDUE
            return d->dtRecurrence();
        }
    }

    return d->dtDue();
}

bool Todo::hasDueDate() const
{
    Q_D(const Todo);
    return d->dtDue().isValid();
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

    Q_D(const Todo);
    if (recurs() && !first && d->dtRecurrence().isValid()) {
        return d->dtRecurrence();
    } else {
        return IncidenceBase::dtStart();
    }
}

bool Todo::isCompleted() const
{
    Q_D(const Todo);
    return d->percentComplete() == 100 || status() == StatusCompleted || hasCompletedDate();
}

void Todo::setCompleted(bool completed)
{
    update();
    Q_D(Todo);
    if (completed) {
        d->setPercentComplete(100);
    } else {
        d->setPercentComplete(0);
        if (hasCompletedDate()) {
            d->setCompleted(QDateTime());
        }
    }
    updated();

    setStatus(completed ? StatusCompleted : StatusNone);    // Calls update()/updated().
}

QDateTime Todo::completed() const
{
    Q_D(const Todo);
    if (hasCompletedDate()) {
        return d->completed();
    } else {
        return QDateTime();
    }
}

void Todo::setCompleted(const QDateTime &completed)
{
    Q_D(Todo);
    if (!d->recurTodo(this)) { // May indirectly call update()/updated().
        update();
        d->setPercentComplete(100);
        d->setCompleted(completed);
        updated();
    }
    if (status() != StatusNone) {
        setStatus(StatusCompleted); // Calls update()/updated()
    }
}

bool Todo::hasCompletedDate() const
{
    Q_D(const Todo);
    return d->completed().isValid();
}

int Todo::percentComplete() const
{
    Q_D(const Todo);
    return d->percentComplete();
}

void Todo::setPercentComplete(int percent)
{
    if (percent > 100) {
        percent = 100;
    } else if (percent < 0) {
        percent = 0;
    }

    update();
    Q_D(Todo);
    d->setPercentComplete(percent);
    if (percent != 100) {
        d->setCompleted(QDateTime());
    }
    updated();
    if (percent != 100 && status() == Incidence::StatusCompleted) {
        setStatus(Incidence::StatusNone);   // Calls update()/updated().
    }
}

bool Todo::isInProgress(bool first) const
{
    if (isOverdue()) {
        return false;
    }

    Q_D(const Todo);
    if (d->percentComplete() > 0) {
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
    Q_D(const Todo);
    if (d->percentComplete() > 0) {
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
    Q_D(Todo);
    Incidence::shiftTimes(oldZone, newZone);
    auto dt = d->dtDue().toTimeZone(oldZone);
    dt.setTimeZone(newZone);
    d->setDtDue(dt);
    if (recurs()) {
        auto dr = d->dtRecurrence().toTimeZone(oldZone);
        dr.setTimeZone(newZone);
        d->setDtRecurrence(dr);
    }
    if (hasCompletedDate()) {
        auto dc = d->completed().toTimeZone(oldZone);
        dc.setTimeZone(newZone);
        d->setCompleted(dc);
    }
}

void Todo::setDtRecurrence(const QDateTime &dt)
{
    Q_D(Todo);
    d->setDtRecurrence(dt);
}

QDateTime Todo::dtRecurrence() const
{
    Q_D(const Todo);
    auto dt = d->dtRecurrence();
    if (!dt.isValid()) {
        dt = IncidenceBase::dtStart();
    }
    if (!dt.isValid()) {
        dt = d->dtDue();
    }
    return dt;
}

bool Todo::recursOn(const QDate &date, const QTimeZone &timeZone) const
{
    Q_D(const Todo);
    QDate today = QDate::currentDate();
    return Incidence::recursOn(date, timeZone) && !(date < today && d->dtRecurrence().date() < today && d->dtRecurrence() > recurrence()->startDateTime());
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
bool TodoPrivate::recurTodo(Todo *todo)
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
        qCDebug(KCALCORE_LOG) << "Unhandled role" << role;
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
    Q_D(const Todo);
    Incidence::serialize(out);
    serializeQDateTimeAsKDateTime(out, d->dtDue());
    serializeQDateTimeAsKDateTime(out, d->dtRecurrence());
    serializeQDateTimeAsKDateTime(out, d->completed());
    out << d->percentComplete();
}

void TodoPrivate::deserialize(QDataStream &in)
{
    deserializeKDateTimeAsQDateTime(in, mDtDue);
    deserializeKDateTimeAsQDateTime(in, mDtRecurrence);
    deserializeKDateTimeAsQDateTime(in, mCompleted);
    in >> mPercentComplete;
}

void Todo::deserialize(QDataStream &in)
{
    Q_D(Todo);
    Incidence::deserialize(in);
    d->deserialize(in);
}

bool Todo::supportsGroupwareCommunication() const
{
    return true;
}
