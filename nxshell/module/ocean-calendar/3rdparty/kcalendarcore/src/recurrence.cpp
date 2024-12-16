/*
  This file is part of kcalcore library.

  SPDX-FileCopyrightText: 1998 Preston Brown <pbrown@kde.org>
  SPDX-FileCopyrightText: 2001 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2002, 2006 David Jarvie <djarvie@kde.org>
  SPDX-FileCopyrightText: 2005 Reinhold Kainhofer <kainhofer@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "recurrence.h"
#include "recurrencehelper_p.h"
#include "utils_p.h"

#include <QBitArray>
#include <QDataStream>
#include <QTime>
#include <QTimeZone>
#include <QDebug>

using namespace KCalendarCore;

//@cond PRIVATE
class Q_DECL_HIDDEN KCalendarCore::Recurrence::Private
{
public:
    Private()
        : mCachedType(rMax)
        , mAllDay(false)
        , mRecurReadOnly(false)
    {
    }

    Private(const Private &p)
        : mRDateTimes(p.mRDateTimes)
        , mRDates(p.mRDates)
        , mExDateTimes(p.mExDateTimes)
        , mExDates(p.mExDates)
        , mStartDateTime(p.mStartDateTime)
        , mCachedType(p.mCachedType)
        , mAllDay(p.mAllDay)
        , mRecurReadOnly(p.mRecurReadOnly)
    {
    }

    bool operator==(const Private &p) const;

    RecurrenceRule::List mExRules;
    RecurrenceRule::List mRRules;
    QList<QDateTime> mRDateTimes;
    DateList mRDates;
    QList<QDateTime> mExDateTimes;
    DateList mExDates;
    QDateTime mStartDateTime; // date/time of first recurrence
    QList<RecurrenceObserver *> mObservers;

    // Cache the type of the recurrence with the old system (e.g. MonthlyPos)
    //使用旧系统缓存重复发生的类型
    mutable ushort mCachedType;

    bool mAllDay = false; // the recurrence has no time, just a date
    bool mRecurReadOnly = false;
};

bool Recurrence::Private::operator==(const Recurrence::Private &p) const
{
    //   qDebug() << mStartDateTime << p.mStartDateTime;
    if ((mStartDateTime != p.mStartDateTime && (mStartDateTime.isValid() || p.mStartDateTime.isValid())) || mAllDay != p.mAllDay
        || mRecurReadOnly != p.mRecurReadOnly || mExDates != p.mExDates || mExDateTimes != p.mExDateTimes || mRDates != p.mRDates
        || mRDateTimes != p.mRDateTimes) {
        return false;
    }

    // Compare the rrules, exrules! Assume they have the same order... This only
    // matters if we have more than one rule (which shouldn't be the default anyway)
    int i;
    int end = mRRules.count();
    if (end != p.mRRules.count()) {
        return false;
    }
    for (i = 0; i < end; ++i) {
        if (*mRRules[i] != *p.mRRules[i]) {
            return false;
        }
    }
    end = mExRules.count();
    if (end != p.mExRules.count()) {
        return false;
    }
    for (i = 0; i < end; ++i) {
        if (*mExRules[i] != *p.mExRules[i]) {
            return false;
        }
    }
    return true;
}
//@endcond

Recurrence::Recurrence()
    : d(new KCalendarCore::Recurrence::Private())
{
}

Recurrence::Recurrence(const Recurrence &r)
    : RecurrenceRule::RuleObserver()
    , d(new KCalendarCore::Recurrence::Private(*r.d))
{
    int i, end;
    d->mRRules.reserve(r.d->mRRules.count());
    for (i = 0, end = r.d->mRRules.count(); i < end; ++i) {
        RecurrenceRule *rule = new RecurrenceRule(*r.d->mRRules[i]);
        d->mRRules.append(rule);
        rule->addObserver(this);
    }
    d->mExRules.reserve(r.d->mExRules.count());
    for (i = 0, end = r.d->mExRules.count(); i < end; ++i) {
        RecurrenceRule *rule = new RecurrenceRule(*r.d->mExRules[i]);
        d->mExRules.append(rule);
        rule->addObserver(this);
    }
}

Recurrence::~Recurrence()
{
    qDeleteAll(d->mExRules);
    qDeleteAll(d->mRRules);
    delete d;
}

bool Recurrence::operator==(const Recurrence &recurrence) const
{
    return *d == *recurrence.d;
}

void Recurrence::addObserver(RecurrenceObserver *observer)
{
    if (!d->mObservers.contains(observer)) {
        d->mObservers.append(observer);
    }
}

void Recurrence::removeObserver(RecurrenceObserver *observer)
{
    d->mObservers.removeAll(observer);
}

QDateTime Recurrence::startDateTime() const
{
    return d->mStartDateTime;
}

bool Recurrence::allDay() const
{
    return d->mAllDay;
}

void Recurrence::setAllDay(bool allDay)
{
    if (d->mRecurReadOnly || allDay == d->mAllDay) {
        return;
    }

    d->mAllDay = allDay;
    for (int i = 0, end = d->mRRules.count(); i < end; ++i) {
        d->mRRules[i]->setAllDay(allDay);
    }
    for (int i = 0, end = d->mExRules.count(); i < end; ++i) {
        d->mExRules[i]->setAllDay(allDay);
    }
    updated();
}

RecurrenceRule *Recurrence::defaultRRule(bool create) const
{
    if (d->mRRules.isEmpty()) {
        if (!create || d->mRecurReadOnly) {
            return nullptr;
        }
        RecurrenceRule *rrule = new RecurrenceRule();
        rrule->setStartDt(startDateTime());
        const_cast<KCalendarCore::Recurrence *>(this)->addRRule(rrule);
        return rrule;
    } else {
        return d->mRRules[0];
    }
}

RecurrenceRule *Recurrence::defaultRRuleConst() const
{
    return d->mRRules.isEmpty() ? nullptr : d->mRRules[0];
}

void Recurrence::updated()
{
    // recurrenceType() re-calculates the type if it's rMax
    d->mCachedType = rMax;
    for (int i = 0, end = d->mObservers.count(); i < end; ++i) {
        if (d->mObservers[i]) {
            d->mObservers[i]->recurrenceUpdated(this);
        }
    }
}

bool Recurrence::recurs() const
{
    return !d->mRRules.isEmpty() || !d->mRDates.isEmpty() || !d->mRDateTimes.isEmpty();
}

ushort Recurrence::recurrenceType() const
{
    if (d->mCachedType == rMax) {
        d->mCachedType = recurrenceType(defaultRRuleConst());
    }
    return d->mCachedType;
}

ushort Recurrence::recurrenceType(const RecurrenceRule *rrule)
{
    if (!rrule) {
        return rNone;
    }
    RecurrenceRule::PeriodType type = rrule->recurrenceType();

    // BYSETPOS, BYWEEKNUMBER and BYSECOND were not supported in old versions
    if (!rrule->bySetPos().isEmpty() || !rrule->bySeconds().isEmpty() || !rrule->byWeekNumbers().isEmpty()) {
        return rOther;
    }

    // It wasn't possible to set BYMINUTES, BYHOUR etc. by the old code. So if
    // it's set, it's none of the old types
    if (!rrule->byMinutes().isEmpty() || !rrule->byHours().isEmpty()) {
        return rOther;
    }

    // Possible combinations were:
    // BYDAY: with WEEKLY, MONTHLY, YEARLY
    // BYMONTHDAY: with MONTHLY, YEARLY
    // BYMONTH: with YEARLY
    // BYYEARDAY: with YEARLY
    if ((!rrule->byYearDays().isEmpty() && type != RecurrenceRule::rYearly) || (!rrule->byMonths().isEmpty() && type != RecurrenceRule::rYearly)) {
        return rOther;
    }
    if (!rrule->byDays().isEmpty()) {
        if (type != RecurrenceRule::rYearly && type != RecurrenceRule::rMonthly && type != RecurrenceRule::rWeekly) {
            return rOther;
        }
    }

    switch (type) {
    case RecurrenceRule::rNone:
        return rNone;
    case RecurrenceRule::rMinutely:
        return rMinutely;
    case RecurrenceRule::rHourly:
        return rHourly;
    case RecurrenceRule::rDaily:
        return rDaily;
    case RecurrenceRule::rWeekly:
        return rWeekly;
    case RecurrenceRule::rMonthly: {
        if (rrule->byDays().isEmpty()) {
            return rMonthlyDay;
        } else if (rrule->byMonthDays().isEmpty()) {
            return rMonthlyPos;
        } else {
            return rOther; // both position and date specified
        }
    }
    case RecurrenceRule::rYearly: {
        // Possible combinations:
        //   rYearlyMonth: [BYMONTH &] BYMONTHDAY
        //   rYearlyDay: BYYEARDAY
        //   rYearlyPos: [BYMONTH &] BYDAY
        if (!rrule->byDays().isEmpty()) {
            // can only by rYearlyPos
            if (rrule->byMonthDays().isEmpty() && rrule->byYearDays().isEmpty()) {
                return rYearlyPos;
            } else {
                return rOther;
            }
        } else if (!rrule->byYearDays().isEmpty()) {
            // Can only be rYearlyDay
            if (rrule->byMonths().isEmpty() && rrule->byMonthDays().isEmpty()) {
                return rYearlyDay;
            } else {
                return rOther;
            }
        } else {
            return rYearlyMonth;
        }
    }
    default:
        return rOther;
    }
}

bool Recurrence::recursOn(const QDate &qd, const QTimeZone &timeZone) const
{
    // Don't waste time if date is before the start of the recurrence
    if (QDateTime(qd, QTime(23, 59, 59), timeZone) < d->mStartDateTime) {
        return false;
    }

    // First handle dates. Exrules override
    if (std::binary_search(d->mExDates.constBegin(), d->mExDates.constEnd(), qd)) {
        return false;
    }

    int i, end;
    // For all-day events a matching exrule excludes the whole day
    // since exclusions take precedence over inclusions, we know it can't occur on that day.
    if (allDay()) {
        for (i = 0, end = d->mExRules.count(); i < end; ++i) {
            if (d->mExRules[i]->recursOn(qd, timeZone)) {
                return false;
            }
        }
    }

    if (std::binary_search(d->mRDates.constBegin(), d->mRDates.constEnd(), qd)) {
        return true;
    }

    // Check if it might recur today at all.
    bool recurs = (startDate() == qd);
    for (i = 0, end = d->mRDateTimes.count(); i < end && !recurs; ++i) {
        recurs = (d->mRDateTimes[i].toTimeZone(timeZone).date() == qd);
    }
    for (i = 0, end = d->mRRules.count(); i < end && !recurs; ++i) {
        recurs = d->mRRules[i]->recursOn(qd, timeZone);
    }
    // If the event wouldn't recur at all, simply return false, don't check ex*
    if (!recurs) {
        return false;
    }

    // Check if there are any times for this day excluded, either by exdate or exrule:
    bool exon = false;
    for (i = 0, end = d->mExDateTimes.count(); i < end && !exon; ++i) {
        exon = (d->mExDateTimes[i].toTimeZone(timeZone).date() == qd);
    }
    if (!allDay()) { // we have already checked all-day times above
        for (i = 0, end = d->mExRules.count(); i < end && !exon; ++i) {
            exon = d->mExRules[i]->recursOn(qd, timeZone);
        }
    }

    if (!exon) {
        // Simple case, nothing on that day excluded, return the value from before
        return recurs;
    } else {
        // Harder part: I don't think there is any way other than to calculate the
        // whole list of items for that day.
        // TODO: consider whether it would be more efficient to call
        //      Rule::recurTimesOn() instead of Rule::recursOn() from the start
        TimeList timesForDay(recurTimesOn(qd, timeZone));
        return !timesForDay.isEmpty();
    }
}

bool Recurrence::recursAt(const QDateTime &dt) const
{
    // Convert to recurrence's time zone for date comparisons, and for more efficient time comparisons
    const auto dtrecur = dt.toTimeZone(d->mStartDateTime.timeZone());

    // if it's excluded anyway, don't bother to check if it recurs at all.
    if (std::binary_search(d->mExDateTimes.constBegin(), d->mExDateTimes.constEnd(), dtrecur)
        || std::binary_search(d->mExDates.constBegin(), d->mExDates.constEnd(), dtrecur.date())) {
        return false;
    }
    int i, end;
    for (i = 0, end = d->mExRules.count(); i < end; ++i) {
        if (d->mExRules[i]->recursAt(dtrecur)) {
            return false;
        }
    }

    // Check explicit recurrences, then rrules.
    if (startDateTime() == dtrecur || std::binary_search(d->mRDateTimes.constBegin(), d->mRDateTimes.constEnd(), dtrecur)) {
        return true;
    }
    for (i = 0, end = d->mRRules.count(); i < end; ++i) {
        if (d->mRRules[i]->recursAt(dtrecur)) {
            return true;
        }
    }

    return false;
}

/** Calculates the cumulative end of the whole recurrence (rdates and rrules).
    If any rrule is infinite, or the recurrence doesn't have any rrules or
    rdates, an invalid date is returned. */
QDateTime Recurrence::endDateTime() const
{
    QList<QDateTime> dts;
    dts << startDateTime();
    if (!d->mRDates.isEmpty()) {
        dts << QDateTime(d->mRDates.last(), QTime(0, 0, 0), d->mStartDateTime.timeZone());
    }
    if (!d->mRDateTimes.isEmpty()) {
        dts << d->mRDateTimes.last();
    }
    for (int i = 0, end = d->mRRules.count(); i < end; ++i) {
        auto rl = d->mRRules[i]->endDt();
        // if any of the rules is infinite, the whole recurrence is
        if (!rl.isValid()) {
            return QDateTime();
        }
        dts << rl;
    }
    sortAndRemoveDuplicates(dts);
    return dts.isEmpty() ? QDateTime() : dts.last();
}

/** Calculates the cumulative end of the whole recurrence (rdates and rrules).
    If any rrule is infinite, or the recurrence doesn't have any rrules or
    rdates, an invalid date is returned. */
QDate Recurrence::endDate() const
{
    QDateTime end(endDateTime());
    return end.isValid() ? end.date() : QDate();
}

void Recurrence::setEndDate(const QDate &date)
{
    QDateTime dt(date, d->mStartDateTime.time(), d->mStartDateTime.timeZone());
    if (allDay()) {
        dt.setTime(QTime(23, 59, 59));
    }
    setEndDateTime(dt);
}

void Recurrence::setEndDateTime(const QDateTime &dateTime)
{
    if (d->mRecurReadOnly) {
        return;
    }
    RecurrenceRule *rrule = defaultRRule(true);
    if (!rrule) {
        return;
    }

    // If the recurrence rule has a duration, and we're trying to set an invalid end date,
    // we have to skip setting it to avoid setting the field dirty.
    // The end date is already invalid since the duration is set and end date/duration
    // are mutually exclusive.
    // We can't use inequality check below, because endDt() also returns a valid date
    // for a duration (it is calculated from the duration).
    if (rrule->duration() > 0 && !dateTime.isValid()) {
        return;
    }

    if (dateTime != rrule->endDt()) {
        rrule->setEndDt(dateTime);
        updated();
    }
}

int Recurrence::duration() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->duration() : 0;
}

int Recurrence::durationTo(const QDateTime &datetime) const
{
    // Emulate old behavior: This is just an interface to the first rule!
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->durationTo(datetime) : 0;
}

int Recurrence::durationTo(const QDate &date) const
{
    return durationTo(QDateTime(date, QTime(23, 59, 59), d->mStartDateTime.timeZone()));
}

void Recurrence::setDuration(int duration)
{
    if (d->mRecurReadOnly) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(true);
    if (!rrule) {
        return;
    }

    if (duration != rrule->duration()) {
        rrule->setDuration(duration);
        updated();
    }
}

void Recurrence::shiftTimes(const QTimeZone &oldTz, const QTimeZone &newTz)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mStartDateTime = d->mStartDateTime.toTimeZone(oldTz);
    d->mStartDateTime.setTimeZone(newTz);

    int i, end;
    for (i = 0, end = d->mRDateTimes.count(); i < end; ++i) {
        d->mRDateTimes[i] = d->mRDateTimes[i].toTimeZone(oldTz);
        d->mRDateTimes[i].setTimeZone(newTz);
    }
    for (i = 0, end = d->mExDateTimes.count(); i < end; ++i) {
        d->mExDateTimes[i] = d->mExDateTimes[i].toTimeZone(oldTz);
        d->mExDateTimes[i].setTimeZone(newTz);
    }
    for (i = 0, end = d->mRRules.count(); i < end; ++i) {
        d->mRRules[i]->shiftTimes(oldTz, newTz);
    }
    for (i = 0, end = d->mExRules.count(); i < end; ++i) {
        d->mExRules[i]->shiftTimes(oldTz, newTz);
    }
}

void Recurrence::unsetRecurs()
{
    if (d->mRecurReadOnly) {
        return;
    }
    qDeleteAll(d->mRRules);
    d->mRRules.clear();
    updated();
}

void Recurrence::clear()
{
    if (d->mRecurReadOnly) {
        return;
    }
    qDeleteAll(d->mRRules);
    d->mRRules.clear();
    qDeleteAll(d->mExRules);
    d->mExRules.clear();
    d->mRDates.clear();
    d->mRDateTimes.clear();
    d->mExDates.clear();
    d->mExDateTimes.clear();
    d->mCachedType = rMax;
    updated();
}

void Recurrence::setRecurReadOnly(bool readOnly)
{
    d->mRecurReadOnly = readOnly;
}

bool Recurrence::recurReadOnly() const
{
    return d->mRecurReadOnly;
}

QDate Recurrence::startDate() const
{
    return d->mStartDateTime.date();
}

void Recurrence::setStartDateTime(const QDateTime &start, bool isAllDay)
{
    if (d->mRecurReadOnly) {
        return;
    }
    d->mStartDateTime = start;
    setAllDay(isAllDay); // set all RRULEs and EXRULEs

    int i, end;
    for (i = 0, end = d->mRRules.count(); i < end; ++i) {
        d->mRRules[i]->setStartDt(start);
    }
    for (i = 0, end = d->mExRules.count(); i < end; ++i) {
        d->mExRules[i]->setStartDt(start);
    }
    updated();
}

int Recurrence::frequency() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->frequency() : 0;
}

// Emulate the old behaviour. Make this methods just an interface to the
// first rrule
void Recurrence::setFrequency(int freq)
{
    if (d->mRecurReadOnly || freq <= 0) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(true);
    if (rrule) {
        rrule->setFrequency(freq);
    }
    updated();
}

// WEEKLY

int Recurrence::weekStart() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->weekStart() : 1;
}

// Emulate the old behavior
QBitArray Recurrence::days() const
{
    QBitArray days(7);
    days.fill(0);
    RecurrenceRule *rrule = defaultRRuleConst();
    if (rrule) {
        const QList<RecurrenceRule::WDayPos> &bydays = rrule->byDays();
        for (int i = 0; i < bydays.size(); ++i) {
            if (bydays.at(i).pos() == 0) {
                days.setBit(bydays.at(i).day() - 1);
            }
        }
    }
    return days;
}

// MONTHLY

// Emulate the old behavior
QList<int> Recurrence::monthDays() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    if (rrule) {
        return rrule->byMonthDays();
    } else {
        return QList<int>();
    }
}

// Emulate the old behavior
QList<RecurrenceRule::WDayPos> Recurrence::monthPositions() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->byDays() : QList<RecurrenceRule::WDayPos>();
}

// YEARLY

QList<int> Recurrence::yearDays() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->byYearDays() : QList<int>();
}

QList<int> Recurrence::yearDates() const
{
    return monthDays();
}

QList<int> Recurrence::yearMonths() const
{
    RecurrenceRule *rrule = defaultRRuleConst();
    return rrule ? rrule->byMonths() : QList<int>();
}

QList<RecurrenceRule::WDayPos> Recurrence::yearPositions() const
{
    return monthPositions();
}

RecurrenceRule *Recurrence::setNewRecurrenceType(RecurrenceRule::PeriodType type, int freq)
{
    if (d->mRecurReadOnly || freq <= 0) {
        return nullptr;
    }

    // Ignore the call if nothing has change
    if (defaultRRuleConst() && defaultRRuleConst()->recurrenceType() == type && frequency() == freq) {
        return nullptr;
    }

    qDeleteAll(d->mRRules);
    d->mRRules.clear();
    updated();
    RecurrenceRule *rrule = defaultRRule(true);
    if (!rrule) {
        return nullptr;
    }
    rrule->setRecurrenceType(type);
    rrule->setFrequency(freq);
    rrule->setDuration(-1);
    return rrule;
}

void Recurrence::setMinutely(int _rFreq)
{
    if (setNewRecurrenceType(RecurrenceRule::rMinutely, _rFreq)) {
        updated();
    }
}

void Recurrence::setHourly(int _rFreq)
{
    if (setNewRecurrenceType(RecurrenceRule::rHourly, _rFreq)) {
        updated();
    }
}

void Recurrence::setDaily(int _rFreq)
{
    if (setNewRecurrenceType(RecurrenceRule::rDaily, _rFreq)) {
        updated();
    }
}

void Recurrence::setWeekly(int freq, int weekStart)
{
    RecurrenceRule *rrule = setNewRecurrenceType(RecurrenceRule::rWeekly, freq);
    if (!rrule) {
        return;
    }
    rrule->setWeekStart(weekStart);
    updated();
}

void Recurrence::setWeekly(int freq, const QBitArray &days, int weekStart)
{
    setWeekly(freq, weekStart);
    addMonthlyPos(0, days);
}

void Recurrence::addWeeklyDays(const QBitArray &days)
{
    addMonthlyPos(0, days);
}

void Recurrence::setMonthly(int freq)
{
    if (setNewRecurrenceType(RecurrenceRule::rMonthly, freq)) {
        updated();
    }
}

void Recurrence::addMonthlyPos(short pos, const QBitArray &days)
{
    // Allow 53 for yearly!
    if (d->mRecurReadOnly || pos > 53 || pos < -53) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(false);
    if (!rrule) {
        return;
    }
    bool changed = false;
    QList<RecurrenceRule::WDayPos> positions = rrule->byDays();

    for (int i = 0; i < 7; ++i) {
        if (days.testBit(i)) {
            RecurrenceRule::WDayPos p(pos, i + 1);
            if (!positions.contains(p)) {
                changed = true;
                positions.append(p);
            }
        }
    }
    if (changed) {
        rrule->setByDays(positions);
        updated();
    }
}

void Recurrence::addMonthlyPos(short pos, ushort day)
{
    // Allow 53 for yearly!
    if (d->mRecurReadOnly || pos > 53 || pos < -53) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(false);
    if (!rrule) {
        return;
    }
    QList<RecurrenceRule::WDayPos> positions = rrule->byDays();

    RecurrenceRule::WDayPos p(pos, day);
    if (!positions.contains(p)) {
        positions.append(p);
        setMonthlyPos(positions);
    }
}

void Recurrence::setMonthlyPos(const QList<RecurrenceRule::WDayPos> &monthlyDays)
{
    if (d->mRecurReadOnly) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(true);
    if (!rrule) {
        return;
    }

    // TODO: sort lists
    // the position inside the list has no meaning, so sort the list before testing if it changed

    if (monthlyDays != rrule->byDays()) {
        rrule->setByDays(monthlyDays);
        updated();
    }
}

void Recurrence::addMonthlyDate(short day)
{
    if (d->mRecurReadOnly || day > 31 || day < -31) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(true);
    if (!rrule) {
        return;
    }

    QList<int> monthDays = rrule->byMonthDays();
    if (!monthDays.contains(day)) {
        monthDays.append(day);
        setMonthlyDate(monthDays);
    }
}

void Recurrence::setMonthlyDate(const QList<int> &monthlyDays)
{
    if (d->mRecurReadOnly) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(true);
    if (!rrule) {
        return;
    }

    QList<int> mD(monthlyDays);
    QList<int> rbD(rrule->byMonthDays());

    sortAndRemoveDuplicates(mD);
    sortAndRemoveDuplicates(rbD);

    if (mD != rbD) {
        rrule->setByMonthDays(monthlyDays);
        updated();
    }
}

void Recurrence::setYearly(int freq)
{
    if (setNewRecurrenceType(RecurrenceRule::rYearly, freq)) {
        updated();
    }
}

// Daynumber within year
void Recurrence::addYearlyDay(int day)
{
    RecurrenceRule *rrule = defaultRRule(false); // It must already exist!
    if (!rrule) {
        return;
    }

    QList<int> days = rrule->byYearDays();
    if (!days.contains(day)) {
        days << day;
        setYearlyDay(days);
    }
}

void Recurrence::setYearlyDay(const QList<int> &days)
{
    RecurrenceRule *rrule = defaultRRule(false); // It must already exist!
    if (!rrule) {
        return;
    }

    QList<int> d(days);
    QList<int> bYD(rrule->byYearDays());

    sortAndRemoveDuplicates(d);
    sortAndRemoveDuplicates(bYD);

    if (d != bYD) {
        rrule->setByYearDays(days);
        updated();
    }
}

// day part of date within year
void Recurrence::addYearlyDate(int day)
{
    addMonthlyDate(day);
}

void Recurrence::setYearlyDate(const QList<int> &dates)
{
    setMonthlyDate(dates);
}

// day part of date within year, given as position (n-th weekday)
void Recurrence::addYearlyPos(short pos, const QBitArray &days)
{
    addMonthlyPos(pos, days);
}

void Recurrence::setYearlyPos(const QList<RecurrenceRule::WDayPos> &days)
{
    setMonthlyPos(days);
}

// month part of date within year
void Recurrence::addYearlyMonth(short month)
{
    if (d->mRecurReadOnly || month < 1 || month > 12) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(false);
    if (!rrule) {
        return;
    }

    QList<int> months = rrule->byMonths();
    if (!months.contains(month)) {
        months << month;
        setYearlyMonth(months);
    }
}

void Recurrence::setYearlyMonth(const QList<int> &months)
{
    if (d->mRecurReadOnly) {
        return;
    }

    RecurrenceRule *rrule = defaultRRule(false);
    if (!rrule) {
        return;
    }

    QList<int> m(months);
    QList<int> bM(rrule->byMonths());

    sortAndRemoveDuplicates(m);
    sortAndRemoveDuplicates(bM);

    if (m != bM) {
        rrule->setByMonths(months);
        updated();
    }
}

TimeList Recurrence::recurTimesOn(const QDate &date, const QTimeZone &timeZone) const
{
    // qDebug() << "recurTimesOn(" << date << ")";
    int i, end;
    TimeList times;

    // The whole day is excepted
    if (std::binary_search(d->mExDates.constBegin(), d->mExDates.constEnd(), date)) {
        return times;
    }

    // EXRULE takes precedence over RDATE entries, so for all-day events,
    // a matching excule also excludes the whole day automatically
    if (allDay()) {
        for (i = 0, end = d->mExRules.count(); i < end; ++i) {
            if (d->mExRules[i]->recursOn(date, timeZone)) {
                return times;
            }
        }
    }

    QDateTime dt = startDateTime().toTimeZone(timeZone);
    if (dt.date() == date) {
        times << dt.time();
    }

    bool foundDate = false;
    for (i = 0, end = d->mRDateTimes.count(); i < end; ++i) {
        dt = d->mRDateTimes[i].toTimeZone(timeZone);
        if (dt.date() == date) {
            times << dt.time();
            foundDate = true;
        } else if (foundDate) {
            break; // <= Assume that the rdatetime list is sorted
        }
    }
    for (i = 0, end = d->mRRules.count(); i < end; ++i) {
        times += d->mRRules[i]->recurTimesOn(date, timeZone);
    }
    sortAndRemoveDuplicates(times);

    foundDate = false;
    TimeList extimes;
    for (i = 0, end = d->mExDateTimes.count(); i < end; ++i) {
        dt = d->mExDateTimes[i].toTimeZone(timeZone);
        if (dt.date() == date) {
            extimes << dt.time();
            foundDate = true;
        } else if (foundDate) {
            break;
        }
    }
    if (!allDay()) { // we have already checked all-day times above
        for (i = 0, end = d->mExRules.count(); i < end; ++i) {
            extimes += d->mExRules[i]->recurTimesOn(date, timeZone);
        }
    }
    sortAndRemoveDuplicates(extimes);
    inplaceSetDifference(times, extimes);
    return times;
}

QList<QDateTime> Recurrence::timesInInterval(const QDateTime &start, const QDateTime &end) const
{
    int i, count;
    QList<QDateTime> times;
    for (i = 0, count = d->mRRules.count(); i < count; ++i) {
        times += d->mRRules[i]->timesInInterval(start, end);
    }

    // add rdatetimes that fit in the interval
    for (i = 0, count = d->mRDateTimes.count(); i < count; ++i) {
        if (d->mRDateTimes[i] >= start && d->mRDateTimes[i] <= end) {
            times += d->mRDateTimes[i];
        }
    }

    // add rdates that fit in the interval
    QDateTime kdt = d->mStartDateTime;
    for (i = 0, count = d->mRDates.count(); i < count; ++i) {
        kdt.setDate(d->mRDates[i]);
        if (kdt >= start && kdt <= end) {
            times += kdt;
        }
    }

    // Recurrence::timesInInterval(...) doesn't explicitly add mStartDateTime to the list
    // of times to be returned. It calls mRRules[i]->timesInInterval(...) which include
    // mStartDateTime.
    // So, If we have rdates/rdatetimes but don't have any rrule we must explicitly
    // add mStartDateTime to the list, otherwise we won't see the first occurrence.
    if ((!d->mRDates.isEmpty() || !d->mRDateTimes.isEmpty()) && d->mRRules.isEmpty() && start <= d->mStartDateTime && end >= d->mStartDateTime) {
        times += d->mStartDateTime;
    }

    sortAndRemoveDuplicates(times);

    // Remove excluded times
    int idt = 0;
    int enddt = times.count();
    for (i = 0, count = d->mExDates.count(); i < count && idt < enddt; ++i) {
        while (idt < enddt && times[idt].date() < d->mExDates[i]) {
            ++idt;
        }
        while (idt < enddt && times[idt].date() == d->mExDates[i]) {
            times.removeAt(idt);
            --enddt;
        }
    }
    QList<QDateTime> extimes;
    for (i = 0, count = d->mExRules.count(); i < count; ++i) {
        extimes += d->mExRules[i]->timesInInterval(start, end);
    }
    extimes += d->mExDateTimes;
    sortAndRemoveDuplicates(extimes);
    inplaceSetDifference(times, extimes);
    return times;
}

QDateTime Recurrence::getNextDateTime(const QDateTime &preDateTime) const
{
    QDateTime nextDT = preDateTime;
    // prevent infinite loops, e.g. when an exrule extinguishes an rrule (e.g.
    // the exrule is identical to the rrule). If an occurrence is found, break
    // out of the loop by returning that QDateTime
    // TODO_Recurrence: Is a loop counter of 1000 really okay? I mean for secondly
    // recurrence, an exdate might exclude more than 1000 intervals!
    int loop = 0;
    while (loop < 1000) {
        // Outline of the algo:
        //   1) Find the next date/time after preDateTime when the event could recur
        //     1.0) Add the start date if it's after preDateTime
        //     1.1) Use the next occurrence from the explicit RDATE lists
        //     1.2) Add the next recurrence for each of the RRULEs
        //   2) Take the earliest recurrence of these = QDateTime nextDT
        //   3) If that date/time is not excluded, either explicitly by an EXDATE or
        //      by an EXRULE, return nextDT as the next date/time of the recurrence
        //   4) If it's excluded, start all at 1), but starting at nextDT (instead
        //      of preDateTime). Loop at most 1000 times.
        ++loop;
        // First, get the next recurrence from the RDate lists
        QList<QDateTime> dates;
        if (nextDT < startDateTime()) {
            dates << startDateTime();
        }

        // Assume that the rdatetime list is sorted
        const auto it = std::upper_bound(d->mRDateTimes.constBegin(), d->mRDateTimes.constEnd(), nextDT);
        if (it != d->mRDateTimes.constEnd()) {
            dates << *it;
        }

        QDateTime kdt(startDateTime());
        for (const auto &date : qAsConst(d->mRDates)) {
            kdt.setDate(date);
            if (kdt > nextDT) {
                dates << kdt;
                break;
            }
        }

        // Add the next occurrences from all RRULEs.
        for (const auto &rule : qAsConst(d->mRRules)) {
            QDateTime dt = rule->getNextDate(nextDT);
            if (dt.isValid()) {
                dates << dt;
            }
        }

        // Take the first of these (all others can't be used later on)
        sortAndRemoveDuplicates(dates);
        if (dates.isEmpty()) {
            return QDateTime();
        }
        nextDT = dates.first();

        // Check if that date/time is excluded explicitly or by an exrule:
        if (!std::binary_search(d->mExDates.constBegin(), d->mExDates.constEnd(), nextDT.date())
            && !std::binary_search(d->mExDateTimes.constBegin(), d->mExDateTimes.constEnd(), nextDT)) {
            bool allowed = true;
            for (const auto &rule : qAsConst(d->mExRules)) {
                allowed = allowed && !rule->recursAt(nextDT);
            }
            if (allowed) {
                return nextDT;
            }
        }
    }

    // Couldn't find a valid occurrences in 1000 loops, something is wrong!
    return QDateTime();
}

QDateTime Recurrence::getPreviousDateTime(const QDateTime &afterDateTime) const
{
    QDateTime prevDT = afterDateTime;
    // prevent infinite loops, e.g. when an exrule extinguishes an rrule (e.g.
    // the exrule is identical to the rrule). If an occurrence is found, break
    // out of the loop by returning that QDateTime
    int loop = 0;
    while (loop < 1000) {
        // Outline of the algo:
        //   1) Find the next date/time after preDateTime when the event could recur
        //     1.1) Use the next occurrence from the explicit RDATE lists
        //     1.2) Add the next recurrence for each of the RRULEs
        //   2) Take the earliest recurrence of these = QDateTime nextDT
        //   3) If that date/time is not excluded, either explicitly by an EXDATE or
        //      by an EXRULE, return nextDT as the next date/time of the recurrence
        //   4) If it's excluded, start all at 1), but starting at nextDT (instead
        //      of preDateTime). Loop at most 1000 times.
        ++loop;
        // First, get the next recurrence from the RDate lists
        QList<QDateTime> dates;
        if (prevDT > startDateTime()) {
            dates << startDateTime();
        }

        const auto it = strictLowerBound(d->mRDateTimes.constBegin(), d->mRDateTimes.constEnd(), prevDT);
        if (it != d->mRDateTimes.constEnd()) {
            dates << *it;
        }

        QDateTime kdt(startDateTime());
        for (const auto &date : qAsConst(d->mRDates)) {
            kdt.setDate(date);
            if (kdt < prevDT) {
                dates << kdt;
                break;
            }
        }

        // Add the previous occurrences from all RRULEs.
        for (const auto &rule : qAsConst(d->mRRules)) {
            QDateTime dt = rule->getPreviousDate(prevDT);
            if (dt.isValid()) {
                dates << dt;
            }
        }

        // Take the last of these (all others can't be used later on)
        sortAndRemoveDuplicates(dates);
        if (dates.isEmpty()) {
            return QDateTime();
        }
        prevDT = dates.last();

        // Check if that date/time is excluded explicitly or by an exrule:
        if (!std::binary_search(d->mExDates.constBegin(), d->mExDates.constEnd(), prevDT.date())
            && !std::binary_search(d->mExDateTimes.constBegin(), d->mExDateTimes.constEnd(), prevDT)) {
            bool allowed = true;
            for (const auto &rule : qAsConst(d->mExRules)) {
                allowed = allowed && !rule->recursAt(prevDT);
            }
            if (allowed) {
                return prevDT;
            }
        }
    }

    // Couldn't find a valid occurrences in 1000 loops, something is wrong!
    return QDateTime();
}

/***************************** PROTECTED FUNCTIONS ***************************/

RecurrenceRule::List Recurrence::rRules() const
{
    return d->mRRules;
}

void Recurrence::addRRule(RecurrenceRule *rrule)
{
    if (d->mRecurReadOnly || !rrule) {
        return;
    }

    rrule->setAllDay(d->mAllDay);
    d->mRRules.append(rrule);
    rrule->addObserver(this);
    updated();
}

void Recurrence::removeRRule(RecurrenceRule *rrule)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mRRules.removeAll(rrule);
    rrule->removeObserver(this);
    updated();
}

void Recurrence::deleteRRule(RecurrenceRule *rrule)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mRRules.removeAll(rrule);
    delete rrule;
    updated();
}

RecurrenceRule::List Recurrence::exRules() const
{
    return d->mExRules;
}

void Recurrence::addExRule(RecurrenceRule *exrule)
{
    if (d->mRecurReadOnly || !exrule) {
        return;
    }

    exrule->setAllDay(d->mAllDay);
    d->mExRules.append(exrule);
    exrule->addObserver(this);
    updated();
}

void Recurrence::removeExRule(RecurrenceRule *exrule)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mExRules.removeAll(exrule);
    exrule->removeObserver(this);
    updated();
}

void Recurrence::deleteExRule(RecurrenceRule *exrule)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mExRules.removeAll(exrule);
    delete exrule;
    updated();
}

QList<QDateTime> Recurrence::rDateTimes() const
{
    return d->mRDateTimes;
}

void Recurrence::setRDateTimes(const QList<QDateTime> &rdates)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mRDateTimes = rdates;
    sortAndRemoveDuplicates(d->mRDateTimes);
    updated();
}

void Recurrence::addRDateTime(const QDateTime &rdate)
{
    if (d->mRecurReadOnly) {
        return;
    }

    setInsert(d->mRDateTimes, rdate);
    updated();
}

DateList Recurrence::rDates() const
{
    return d->mRDates;
}

void Recurrence::setRDates(const DateList &rdates)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mRDates = rdates;
    sortAndRemoveDuplicates(d->mRDates);
    updated();
}

void Recurrence::addRDate(const QDate &rdate)
{
    if (d->mRecurReadOnly) {
        return;
    }

    setInsert(d->mRDates, rdate);
    updated();
}

QList<QDateTime> Recurrence::exDateTimes() const
{
    return d->mExDateTimes;
}

void Recurrence::setExDateTimes(const QList<QDateTime> &exdates)
{
    if (d->mRecurReadOnly) {
        return;
    }

    d->mExDateTimes = exdates;
    sortAndRemoveDuplicates(d->mExDateTimes);
}

void Recurrence::addExDateTime(const QDateTime &exdate)
{
    if (d->mRecurReadOnly) {
        return;
    }

    setInsert(d->mExDateTimes, exdate);
    updated();
}

DateList Recurrence::exDates() const
{
    return d->mExDates;
}

void Recurrence::setExDates(const DateList &exdates)
{
    if (d->mRecurReadOnly) {
        return;
    }

    DateList l = exdates;
    sortAndRemoveDuplicates(l);

    if (d->mExDates != l) {
        d->mExDates = l;
        updated();
    }
}

void Recurrence::addExDate(const QDate &exdate)
{
    if (d->mRecurReadOnly) {
        return;
    }

    setInsert(d->mExDates, exdate);
    updated();
}

void Recurrence::recurrenceChanged(RecurrenceRule *)
{
    updated();
}

// %%%%%%%%%%%%%%%%%% end:Recurrencerule %%%%%%%%%%%%%%%%%%

void Recurrence::dump() const
{
    int i;
    int count = d->mRRules.count();
    qDebug() << "  -)" << count << "RRULEs:";
    for (i = 0; i < count; ++i) {
        qDebug() << "    -) RecurrenceRule: ";
        d->mRRules[i]->dump();
    }
    count = d->mExRules.count();
    qDebug() << "  -)" << count << "EXRULEs:";
    for (i = 0; i < count; ++i) {
        qDebug() << "    -) ExceptionRule :";
        d->mExRules[i]->dump();
    }

    count = d->mRDates.count();
    qDebug() << "  -)" << count << "Recurrence Dates:";
    for (i = 0; i < count; ++i) {
        qDebug() << "    " << d->mRDates[i];
    }
    count = d->mRDateTimes.count();
    qDebug() << "  -)" << count << "Recurrence Date/Times:";
    for (i = 0; i < count; ++i) {
        qDebug() << "    " << d->mRDateTimes[i];
    }
    count = d->mExDates.count();
    qDebug() << "  -)" << count << "Exceptions Dates:";
    for (i = 0; i < count; ++i) {
        qDebug() << "    " << d->mExDates[i];
    }
    count = d->mExDateTimes.count();
    qDebug() << "  -)" << count << "Exception Date/Times:";
    for (i = 0; i < count; ++i) {
        qDebug() << "    " << d->mExDateTimes[i];
    }
}

Recurrence::RecurrenceObserver::~RecurrenceObserver()
{
}

Q_CORE_EXPORT QDataStream &KCalendarCore::operator<<(QDataStream &out, KCalendarCore::Recurrence *r)
{
    if (!r) {
        return out;
    }

    serializeQDateTimeList(out, r->d->mRDateTimes);
    serializeQDateTimeList(out, r->d->mExDateTimes);
    out << r->d->mRDates;
    serializeQDateTimeAsKDateTime(out, r->d->mStartDateTime);
    out << r->d->mCachedType << r->d->mAllDay << r->d->mRecurReadOnly << r->d->mExDates << r->d->mExRules.count() << r->d->mRRules.count();

    for (RecurrenceRule *rule : qAsConst(r->d->mExRules)) {
        out << rule;
    }

    for (RecurrenceRule *rule : qAsConst(r->d->mRRules)) {
        out << rule;
    }

    return out;
}

Q_CORE_EXPORT QDataStream &KCalendarCore::operator>>(QDataStream &in, KCalendarCore::Recurrence *r)
{
    if (!r) {
        return in;
    }

    int rruleCount, exruleCount;

    deserializeQDateTimeList(in, r->d->mRDateTimes);
    deserializeQDateTimeList(in, r->d->mExDateTimes);
    in >> r->d->mRDates;
    deserializeKDateTimeAsQDateTime(in, r->d->mStartDateTime);
    in >> r->d->mCachedType >> r->d->mAllDay >> r->d->mRecurReadOnly >> r->d->mExDates >> exruleCount >> rruleCount;

    r->d->mExRules.clear();
    r->d->mRRules.clear();

    for (int i = 0; i < exruleCount; ++i) {
        RecurrenceRule *rule = new RecurrenceRule();
        rule->addObserver(r);
        in >> rule;
        r->d->mExRules.append(rule);
    }

    for (int i = 0; i < rruleCount; ++i) {
        RecurrenceRule *rule = new RecurrenceRule();
        rule->addObserver(r);
        in >> rule;
        r->d->mRRules.append(rule);
    }

    return in;
}
