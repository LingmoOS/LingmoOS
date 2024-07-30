/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the Event class.

  @brief
  This class provides an Event in the sense of RFC2445.

  @author Cornelius Schumacher \<schumacher@kde.org\>
*/

#include "event.h"
#include "incidence_p.h"
#include "kcalendarcore_debug.h"
#include "utils_p.h"
#include "visitor.h"

#include <QDate>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class KCalendarCore::EventPrivate : public IncidencePrivate
{
public:
    EventPrivate() = default;
    explicit EventPrivate(const Incidence &);
    bool validStatus(Incidence::Status) override;

    QDateTime mDtEnd;
    Event::Transparency mTransparency = Event::Opaque;
    bool mMultiDayValid = false;
    bool mMultiDay = false;
};

// Copy IncidencePrivate and IncidenceBasePrivate members,
// but default-initialize EventPrivate members.
EventPrivate::EventPrivate(const Incidence &other)
    : IncidencePrivate(other)
{
}

bool EventPrivate::validStatus(Incidence::Status status)
{
    constexpr unsigned validSet
        = 1u << Incidence::StatusNone
        | 1u << Incidence::StatusTentative
        | 1u << Incidence::StatusConfirmed
        | 1u << Incidence::StatusCanceled;
    return validSet & (1u << status);
}
//@endcond

Event::Event()
    : Incidence(new EventPrivate)
{
}

Event::Event(const Event &other)
    : Incidence(other, new EventPrivate(*(other.d_func())))
{
}

Event::Event(const Incidence &other)
    : Incidence(other, new EventPrivate(other))
{
}

Event::~Event() = default;

Event *Event::clone() const
{
    return new Event(*this);
}

IncidenceBase &Event::assign(const IncidenceBase &other)
{
    Q_D(Event);
    if (&other != this) {
        Incidence::assign(other);
        const auto o = static_cast<const Event *>(&other)->d_func();
        d->mDtEnd = o->mDtEnd;
        d->mTransparency = o->mTransparency;
        d->mMultiDayValid = o->mMultiDayValid;
        d->mMultiDay = o->mMultiDay;
    }
    return *this;
}

bool Event::equals(const IncidenceBase &event) const
{
    if (!Incidence::equals(event)) {
        return false;
    } else {
        // If they weren't the same type IncidenceBase::equals would had returned false already
        const Event *e = static_cast<const Event *>(&event);
        return identical(dtEnd(), e->dtEnd()) && transparency() == e->transparency();
    }
}

Incidence::IncidenceType Event::type() const
{
    return TypeEvent;
}

QByteArray Event::typeStr() const
{
    return QByteArrayLiteral("Event");
}

void Event::setDtStart(const QDateTime &dt)
{
    Q_D(Event);
    d->mMultiDayValid = false;
    Incidence::setDtStart(dt);
}

void Event::setDtEnd(const QDateTime &dtEnd)
{
    if (mReadOnly) {
        return;
    }

    Q_D(Event);
    if (!identical(d->mDtEnd, dtEnd) || hasDuration() == dtEnd.isValid()) {
        update();
        d->mDtEnd = dtEnd;
        d->mMultiDayValid = false;
        setHasDuration(!dtEnd.isValid());
        setFieldDirty(FieldDtEnd);
        updated();
    }
}

QDateTime Event::dtEnd() const
{
    Q_D(const Event);
    if (d->mDtEnd.isValid()) {
        return d->mDtEnd;
    }

    if (hasDuration()) {
        if (allDay()) {
            // For all day events, dtEnd is always inclusive
            QDateTime end = duration().end(dtStart().addDays(-1));
            return end >= dtStart() ? end : dtStart();
        } else {
            return duration().end(dtStart());
        }
    }

    // It is valid for a VEVENT to be without a DTEND. See RFC2445, Sect4.6.1.
    // Be careful to use Event::dateEnd() as appropriate due to this possibility.
    return dtStart();
}

QDate Event::dateEnd() const
{
    QDateTime end = dtEnd().toTimeZone(dtStart().timeZone());
    if (allDay()) {
        return end.date();
    } else {
        return end.addSecs(-1).date();
    }
}

bool Event::hasEndDate() const
{
    Q_D(const Event);
    return d->mDtEnd.isValid();
}

bool Event::isMultiDay(const QTimeZone &zone) const
{
    Q_D(const Event);
    // First off, if spec's not valid, we can check for cache
    if (!zone.isValid() && d->mMultiDayValid) {
        return d->mMultiDay;
    }

    // Not in cache -> do it the hard way
    QDateTime start;
    QDateTime end;

    if (!zone.isValid()) {
        start = dtStart();
        end = dtEnd();
    } else {
        start = dtStart().toTimeZone(zone);
        end = dtEnd().toTimeZone(zone);
    }

    bool multi = (start < end && start.date() != end.date());

    // End date is non inclusive
    // If we have an incidence that duration is one day and ends with a start of a new day
    // than it is not a multiday event
    if (multi && end.time() == QTime(0, 0, 0)) {
        multi = start.daysTo(end) > 1;
    }

    // Update the cache
    // Also update Cache if spec is invalid
    {
        auto d = static_cast<EventPrivate *>(d_ptr);
        d->mMultiDayValid = true;
        d->mMultiDay = multi;
    }
    return multi;
}

void Event::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    Q_D(Event);
    Incidence::shiftTimes(oldZone, newZone);
    if (d->mDtEnd.isValid()) {
        update();
        d->mDtEnd = d->mDtEnd.toTimeZone(oldZone);
        d->mDtEnd.setTimeZone(newZone);
        setFieldDirty(FieldDtEnd);
        updated();
    }
}

void Event::setTransparency(Event::Transparency transparency)
{
    if (mReadOnly) {
        return;
    }
    update();
    Q_D(Event);
    d->mTransparency = transparency;
    setFieldDirty(FieldTransparency);
    updated();
}

Event::Transparency Event::transparency() const
{
    Q_D(const Event);
    return d->mTransparency;
}

void Event::setDuration(const Duration &duration)
{
    // These both call update()/updated() and setFieldDirty().
    setDtEnd(QDateTime());
    Incidence::setDuration(duration);
}

void Event::setAllDay(bool allday)
{
    if (allday != allDay() && !mReadOnly) {
        update();
        setFieldDirty(FieldDtEnd);
        Incidence::setAllDay(allday);
        updated();
    }
}

bool Event::accept(Visitor &v, const IncidenceBase::Ptr &incidence)
{
    return v.visit(incidence.staticCast<Event>());
}

QDateTime Event::dateTime(DateTimeRole role) const
{
    switch (role) {
    case RoleRecurrenceStart:
    case RoleAlarmStartOffset:
    case RoleStartTimeZone:
    case RoleSort:
        return dtStart();
    case RoleCalendarHashing:
        return !recurs() && !isMultiDay() ? dtStart() : QDateTime();
    case RoleAlarmEndOffset:
    case RoleEndTimeZone:
    case RoleEndRecurrenceBase:
    case RoleEnd:
    case RoleDisplayEnd:
        return dtEnd();
    case RoleDisplayStart:
        return dtStart();
    case RoleAlarm:
        if (alarms().isEmpty()) {
            return QDateTime();
        } else {
            Alarm::Ptr alarm = alarms().at(0);
            return alarm->hasStartOffset() ? dtStart() : dtEnd();
        }
    default:
        return QDateTime();
    }
}

void Event::setDateTime(const QDateTime &dateTime, DateTimeRole role)
{
    switch (role) {
    case RoleDnD: {
        const qint64 duration = dtStart().secsTo(dtEnd());

        setDtStart(dateTime);
        setDtEnd(dateTime.addSecs(duration <= 0 ? 3600 : duration));
        break;
    }
    case RoleEnd:
        setDtEnd(dateTime);
        break;
    default:
        qCDebug(KCALCORE_LOG) << "Unhandled role" << role;
    }
}

void Event::virtual_hook(VirtualHook id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
}

QLatin1String KCalendarCore::Event::mimeType() const
{
    return Event::eventMimeType();
}

QLatin1String Event::eventMimeType()
{
    return QLatin1String("application/x-vnd.akonadi.calendar.event");
}

QLatin1String Event::iconName(const QDateTime &) const
{
    return QLatin1String("view-calendar-day");
}

void Event::serialize(QDataStream &out) const
{
    Q_D(const Event);
    Incidence::serialize(out);
    serializeQDateTimeAsKDateTime(out, d->mDtEnd);
    out << hasEndDate() << static_cast<quint32>(d->mTransparency) << d->mMultiDayValid << d->mMultiDay;
}

void Event::deserialize(QDataStream &in)
{
    Q_D(Event);
    Incidence::deserialize(in);
    bool hasEndDateDummy = true;
    deserializeKDateTimeAsQDateTime(in, d->mDtEnd);
    in >> hasEndDateDummy;
    quint32 transp;
    in >> transp;
    d->mTransparency = static_cast<Transparency>(transp);
    in >> d->mMultiDayValid >> d->mMultiDay;
}

bool Event::supportsGroupwareCommunication() const
{
    return true;
}

#include "moc_event.cpp"
