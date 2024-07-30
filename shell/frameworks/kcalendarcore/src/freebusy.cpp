/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
/**
  @file
  This file is part of the API for handling calendar data and
  defines the FreeBusy class.

  @brief
  Provides information about the free/busy time of a calendar user.

  @author Cornelius Schumacher \<schumacher@kde.org\>
  @author Reinhold Kainhofer \<reinhold@kainhofer.com\>
*/
#include "freebusy.h"
#include "utils_p.h"
#include "visitor.h"

#include "icalformat.h"

#include "incidencebase_p.h"
#include "kcalendarcore_debug.h"
#include <QTime>

using namespace KCalendarCore;

//@cond PRIVATE
class KCalendarCore::FreeBusyPrivate : public IncidenceBasePrivate
{
public:
    FreeBusyPrivate() = default;
    FreeBusyPrivate(const FreeBusyPrivate &other) = default;

    FreeBusyPrivate(const FreeBusyPeriod::List &busyPeriods)
        : IncidenceBasePrivate()
        , mBusyPeriods(busyPeriods)
    {
    }

    void init(const FreeBusyPrivate &other);
    void init(const Event::List &events, const QDateTime &start, const QDateTime &end);

    QDateTime mDtEnd; // end datetime
    FreeBusyPeriod::List mBusyPeriods; // list of periods

    // This is used for creating a freebusy object for the current user
    bool addLocalPeriod(const QDateTime &eventStart, const QDateTime &eventEnd);

    void sortBusyPeriods()
    {
        std::sort(mBusyPeriods.begin(), mBusyPeriods.end());
    }
};

void FreeBusyPrivate::init(const FreeBusyPrivate &other)
{
    mDtEnd = other.mDtEnd;
    mBusyPeriods = other.mBusyPeriods;
}
//@endcond

FreeBusy::FreeBusy()
    : IncidenceBase(new FreeBusyPrivate())
{
}

FreeBusy::FreeBusy(const FreeBusy &other)
    : IncidenceBase(other, new FreeBusyPrivate(*other.d_func()))
{
}

FreeBusy::FreeBusy(const QDateTime &start, const QDateTime &end)
    : FreeBusy()
{
    setDtStart(start); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall
    setDtEnd(end); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall
}

FreeBusy::FreeBusy(const Event::List &events, const QDateTime &start, const QDateTime &end)
    : FreeBusy()
{
    setDtStart(start); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall
    setDtEnd(end); // NOLINT false clang-analyzer-optin.cplusplus.VirtualCall

    Q_D(FreeBusy);
    d->init(events, start, end);
}

//@cond PRIVATE
void FreeBusyPrivate::init(const Event::List &eventList, const QDateTime &start, const QDateTime &end)
{
    const qint64 duration = start.daysTo(end);
    QDate day;
    QDateTime tmpStart;
    QDateTime tmpEnd;

    // Loops through every event in the calendar
    for (auto event : eventList) {
        // If this event is transparent it shouldn't be in the freebusy list.
        if (event->transparency() == Event::Transparent) {
            continue;
        }

        // The code below can not handle all-day events. Fixing this resulted
        // in a lot of duplicated code. Instead, make a copy of the event and
        // set the period to the full day(s). This trick works for recurring,
        // multiday, and single day all-day events.
        Event::Ptr allDayEvent;
        if (event->allDay()) {
            // addDay event. Do the hack
            qCDebug(KCALCORE_LOG) << "All-day event";
            allDayEvent = Event::Ptr(new Event(*event));

            // Set the start and end times to be on midnight
            QDateTime st = allDayEvent->dtStart();
            st.setTime(QTime(0, 0));
            QDateTime nd = allDayEvent->dtEnd();
            nd.setTime(QTime(23, 59, 59, 999));
            allDayEvent->setAllDay(false);
            allDayEvent->setDtStart(st);
            allDayEvent->setDtEnd(nd);

            qCDebug(KCALCORE_LOG) << "Use:" << st.toString() << "to" << nd.toString();
            // Finally, use this event for the setting below
            event = allDayEvent;
        }

        // This whole for loop is for recurring events, it loops through
        // each of the days of the freebusy request

        for (qint64 i = 0; i <= duration; ++i) {
            day = start.addDays(i).date();
            tmpStart.setDate(day);
            tmpEnd.setDate(day);

            if (event->recurs()) {
                if (event->isMultiDay()) {
                    // FIXME: This doesn't work for sub-daily recurrences or recurrences with
                    //        a different time than the original event.
                    const qint64 extraDays = event->dtStart().daysTo(event->dtEnd());
                    for (qint64 x = 0; x <= extraDays; ++x) {
                        if (event->recursOn(day.addDays(-x), start.timeZone())) {
                            tmpStart.setDate(day.addDays(-x));
                            tmpStart.setTime(event->dtStart().time());
                            tmpEnd = event->duration().end(tmpStart);

                            addLocalPeriod(tmpStart, tmpEnd);
                            break;
                        }
                    }
                } else {
                    if (event->recursOn(day, start.timeZone())) {
                        tmpStart.setTime(event->dtStart().time());
                        tmpEnd.setTime(event->dtEnd().time());

                        addLocalPeriod(tmpStart, tmpEnd);
                    }
                }
            }
        }

        // Non-recurring events
        addLocalPeriod(event->dtStart(), event->dtEnd());
    }

    sortBusyPeriods();
}
//@endcond

FreeBusy::FreeBusy(const Period::List &busyPeriods)
    : IncidenceBase(new FreeBusyPrivate())
{
    addPeriods(busyPeriods);
}

FreeBusy::FreeBusy(const FreeBusyPeriod::List &busyPeriods)
    : IncidenceBase(new FreeBusyPrivate(busyPeriods))
{
}

FreeBusy::~FreeBusy() = default;

IncidenceBase::IncidenceType FreeBusy::type() const
{
    return TypeFreeBusy;
}

QByteArray FreeBusy::typeStr() const
{
    return QByteArrayLiteral("FreeBusy");
}

void FreeBusy::setDtStart(const QDateTime &start)
{
    IncidenceBase::setDtStart(start.toUTC());
}

void FreeBusy::setDtEnd(const QDateTime &end)
{
    Q_D(FreeBusy);
    update();
    d->mDtEnd = end;
    setFieldDirty(FieldDtEnd);
    updated();
}

QDateTime FreeBusy::dtEnd() const
{
    Q_D(const FreeBusy);
    return d->mDtEnd;
}

Period::List FreeBusy::busyPeriods() const
{
    Period::List res;

    Q_D(const FreeBusy);
    res.reserve(d->mBusyPeriods.count());
    for (const FreeBusyPeriod &p : std::as_const(d->mBusyPeriods)) {
        res << p;
    }

    return res;
}

FreeBusyPeriod::List FreeBusy::fullBusyPeriods() const
{
    Q_D(const FreeBusy);
    return d->mBusyPeriods;
}

void FreeBusy::sortList()
{
    Q_D(FreeBusy);
    d->sortBusyPeriods();
}

void FreeBusy::addPeriods(const Period::List &list)
{
    Q_D(FreeBusy);
    d->mBusyPeriods.reserve(d->mBusyPeriods.count() + list.count());
    for (const Period &p : std::as_const(list)) {
        d->mBusyPeriods << FreeBusyPeriod(p);
    }
    sortList();
}

void FreeBusy::addPeriods(const FreeBusyPeriod::List &list)
{
    Q_D(FreeBusy);
    d->mBusyPeriods += list;
    sortList();
}

void FreeBusy::addPeriod(const QDateTime &start, const QDateTime &end)
{
    Q_D(FreeBusy);
    d->mBusyPeriods.append(FreeBusyPeriod(start, end));
    sortList();
}

void FreeBusy::addPeriod(const QDateTime &start, const Duration &duration)
{
    Q_D(FreeBusy);
    d->mBusyPeriods.append(FreeBusyPeriod(start, duration));
    sortList();
}

void FreeBusy::merge(const FreeBusy::Ptr &freeBusy)
{
    if (freeBusy->dtStart() < dtStart()) {
        setDtStart(freeBusy->dtStart());
    }

    if (freeBusy->dtEnd() > dtEnd()) {
        setDtEnd(freeBusy->dtEnd());
    }

    Q_D(FreeBusy);
    const Period::List periods = freeBusy->busyPeriods();
    d->mBusyPeriods.reserve(d->mBusyPeriods.count() + periods.count());
    for (const auto &p : periods) {
        d->mBusyPeriods.append(FreeBusyPeriod(p.start(), p.end()));
    }
    sortList();
}

void FreeBusy::shiftTimes(const QTimeZone &oldZone, const QTimeZone &newZone)
{
    Q_D(FreeBusy);
    if (oldZone.isValid() && newZone.isValid() && oldZone != newZone) {
        IncidenceBase::shiftTimes(oldZone, newZone);
        update();
        d->mDtEnd = d->mDtEnd.toTimeZone(oldZone);
        d->mDtEnd.setTimeZone(newZone);
        for (FreeBusyPeriod p : std::as_const(d->mBusyPeriods)) {
            p.shiftTimes(oldZone, newZone);
        }
        setFieldDirty(FieldDtEnd);
        updated();
    }
}

IncidenceBase &FreeBusy::assign(const IncidenceBase &other)
{
    Q_D(FreeBusy);
    if (&other != this) {
        IncidenceBase::assign(other);
        const FreeBusy *f = static_cast<const FreeBusy *>(&other);
        d->init(*(f->d_func()));
    }
    return *this;
}

bool FreeBusy::equals(const IncidenceBase &freeBusy) const
{
    if (!IncidenceBase::equals(freeBusy)) {
        return false;
    } else {
        Q_D(const FreeBusy);
        // If they weren't the same type IncidenceBase::equals would had returned false already
        const FreeBusy *fb = static_cast<const FreeBusy *>(&freeBusy);
        return identical(dtEnd(), fb->dtEnd()) && d->mBusyPeriods == fb->d_func()->mBusyPeriods;
    }
}

bool FreeBusy::accept(Visitor &v, const IncidenceBase::Ptr &incidence)
{
    return v.visit(incidence.staticCast<FreeBusy>());
}

QDateTime FreeBusy::dateTime(DateTimeRole role) const
{
    Q_UNUSED(role);
    // No roles affecting freeBusy yet
    return QDateTime();
}

void FreeBusy::setDateTime(const QDateTime &dateTime, DateTimeRole role)
{
    Q_UNUSED(dateTime);
    Q_UNUSED(role);
}

void FreeBusy::virtual_hook(VirtualHook id, void *data)
{
    Q_UNUSED(id);
    Q_UNUSED(data);
    Q_ASSERT(false);
}

//@cond PRIVATE
bool FreeBusyPrivate::addLocalPeriod(const QDateTime &eventStart, const QDateTime &eventEnd)
{
    QDateTime tmpStart;
    QDateTime tmpEnd;

    // Check to see if the start *or* end of the event is
    // between the start and end of the freebusy dates.
    QDateTime start = mDtStart;
    if (!(((start.secsTo(eventStart) >= 0) && (eventStart.secsTo(mDtEnd) >= 0)) || ((start.secsTo(eventEnd) >= 0) && (eventEnd.secsTo(mDtEnd) >= 0)))) {
        return false;
    }

    if (eventStart.secsTo(start) >= 0) {
        tmpStart = start;
    } else {
        tmpStart = eventStart;
    }

    if (eventEnd.secsTo(mDtEnd) <= 0) {
        tmpEnd = mDtEnd;
    } else {
        tmpEnd = eventEnd;
    }

    FreeBusyPeriod p(tmpStart, tmpEnd);
    mBusyPeriods.append(p);

    return true;
}
//@endcond

QLatin1String FreeBusy::mimeType() const
{
    return FreeBusy::freeBusyMimeType();
}

QLatin1String KCalendarCore::FreeBusy::freeBusyMimeType()
{
    return QLatin1String("application/x-vnd.akonadi.calendar.freebusy");
}

QDataStream &KCalendarCore::operator<<(QDataStream &stream, const KCalendarCore::FreeBusy::Ptr &freebusy)
{
    KCalendarCore::ICalFormat format;
    QString data = format.createScheduleMessage(freebusy, iTIPPublish);
    return stream << data;
}

QDataStream &KCalendarCore::operator>>(QDataStream &stream, KCalendarCore::FreeBusy::Ptr &freebusy)
{
    QString freeBusyVCal;
    stream >> freeBusyVCal;

    KCalendarCore::ICalFormat format;
    freebusy = format.parseFreeBusy(freeBusyVCal);

    if (!freebusy) {
        qCDebug(KCALCORE_LOG) << "Error parsing free/busy";
        qCDebug(KCALCORE_LOG) << freeBusyVCal;
    }

    return stream;
}
