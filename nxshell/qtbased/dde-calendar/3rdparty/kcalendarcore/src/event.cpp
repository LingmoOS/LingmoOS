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
#include "utils_p.h"
#include "visitor.h"

#include <QDate>
#include <QDebug>

using namespace KCalendarCore;

/**
  Private class that helps to provide binary compatibility between releases.
  @internal
*/
//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Event::Private
{
public:
    QDateTime mDtEnd;
    Transparency mTransparency = Opaque;
    bool mMultiDayValid = false;
    bool mMultiDay = false;
    bool mIsLunnar = false; //是否为农历
};
//@endcond

Event::Event()
    : d(new KCalendarCore::Event::Private)
{
}

Event::Event(const Event &other)
    : Incidence(other)
    , d(new KCalendarCore::Event::Private(*other.d))
{
}

Event::Event(const Incidence &other)
    : Incidence(other)
    , d(new KCalendarCore::Event::Private)
{
}

Event::~Event()
{
    delete d;
}

Event *Event::clone() const
{
    return new Event(*this);
}

IncidenceBase &Event::assign(const IncidenceBase &other)
{
    if (&other != this) {
        Incidence::assign(other);
        const Event *e = static_cast<const Event *>(&other);
        *d = *(e->d);
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
        return ((dtEnd() == e->dtEnd()) || (!dtEnd().isValid() && !e->dtEnd().isValid())) && transparency() == e->transparency();
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
    d->mMultiDayValid = false;
    Incidence::setDtStart(dt);
}

void Event::setDtEnd(const QDateTime &dtEnd)
{
    if (mReadOnly) {
        return;
    }

    if (d->mDtEnd != dtEnd || hasDuration() == dtEnd.isValid()) {
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
    return d->mDtEnd.isValid();
}

bool Event::isMultiDay(const QTimeZone &zone) const
{
    // First off, if spec's not valid, we can check for cache
    if (!zone.isValid() && d->mMultiDayValid) {
        return d->mMultiDay;
    }

    // Not in cache -> do it the hard way
    QDateTime start, end;

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
    d->mMultiDayValid = true;
    d->mMultiDay = multi;
    return multi;
}

void Event::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    Incidence::shiftTimes(oldZone, newZone);
    if (d->mDtEnd.isValid()) {
        d->mDtEnd = d->mDtEnd.toTimeZone(oldZone);
        d->mDtEnd.setTimeZone(newZone);
    }
}

void Event::setTransparency(Event::Transparency transparency)
{
    if (mReadOnly) {
        return;
    }
    update();
    d->mTransparency = transparency;
    setFieldDirty(FieldTransparency);
    updated();
}

Event::Transparency Event::transparency() const
{
    return d->mTransparency;
}

void Event::setDuration(const Duration &duration)
{
    setDtEnd(QDateTime());
    Incidence::setDuration(duration);
}

void Event::setAllDay(bool allday)
{
    if (allday != allDay() && !mReadOnly) {
        setFieldDirty(FieldDtEnd);
        Incidence::setAllDay(allday);
    }
}

void Event::setLunnar(bool lunnar)
{
    if (d->mIsLunnar == lunnar)
        return;
    update();
    d->mIsLunnar = lunnar;
    updated();
}

bool Event::lunnar() const
{
    return d->mIsLunnar;
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
        qDebug() << "Unhandled role" << role;
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
    Incidence::serialize(out);
    serializeQDateTimeAsKDateTime(out, d->mDtEnd);
    out << hasEndDate() << static_cast<quint32>(d->mTransparency) << d->mMultiDayValid << d->mMultiDay;
}

void Event::deserialize(QDataStream &in)
{
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
