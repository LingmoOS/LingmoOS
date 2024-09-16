/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2005-2007 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "icalformat.h"
#include "icalformat_p.h"
#include "icaltimezones_p.h"
#include "recurrence.h"
#include "recurrencehelper_p.h"
#include "recurrencerule.h"

#include <QByteArray>
#include <QDateTime>
#include <QDebug>

extern "C" {
//#include <icaltimezone.h>
#include <libical/ical.h>
}

using namespace KCalendarCore;

// Minimum repetition counts for VTIMEZONE RRULEs
static const int minRuleCount = 5; // for any RRULE
static const int minPhaseCount = 8; // for separate STANDARD/DAYLIGHT component

// Convert an ical time to QDateTime, preserving the UTC indicator
static QDateTime toQDateTime(const icaltimetype &t)
{
    return QDateTime(QDate(t.year, t.month, t.day),
                     QTime(t.hour, t.minute, t.second),
                     (icaltime_is_utc(t) ? Qt::UTC : Qt::LocalTime));
}

// Maximum date for time zone data.
// It's not sensible to try to predict them very far in advance, because
// they can easily change. Plus, it limits the processing required.
static QDateTime MAX_DATE()
{
    static QDateTime dt;
    if (!dt.isValid()) {
        dt = QDateTime(QDate::currentDate().addYears(20), QTime(0, 0, 0));
    }
    return dt;
}

static icaltimetype writeLocalICalDateTime(const QDateTime &utc, int offset)
{
    const QDateTime local = utc.addSecs(offset);
    icaltimetype t = icaltime_null_time();
    t.year = local.date().year();
    t.month = local.date().month();
    t.day = local.date().day();
    t.hour = local.time().hour();
    t.minute = local.time().minute();
    t.second = local.time().second();
    t.is_date = 0;
    t.zone = nullptr;
    return t;
}

namespace KCalendarCore {
void ICalTimeZonePhase::dump()
{
    qDebug() << "       ~~~ ICalTimeZonePhase ~~~";
    qDebug() << "       Abbreviations:" << abbrevs;
    qDebug() << "       UTC offset:" << utcOffset;
    qDebug() << "       Transitions:" << transitions;
    qDebug() << "       ~~~~~~~~~~~~~~~~~~~~~~~~~";
}

void ICalTimeZone::dump()
{
    qDebug() << "~~~ ICalTimeZone ~~~";
    qDebug() << "ID:" << id;
    qDebug() << "QZONE:" << qZone.id();
    qDebug() << "STD:";
    standard.dump();
    qDebug() << "DST:";
    daylight.dump();
    qDebug() << "~~~~~~~~~~~~~~~~~~~~";
}

ICalTimeZoneCache::ICalTimeZoneCache()
{
}

void ICalTimeZoneCache::insert(const QByteArray &id, const ICalTimeZone &tz)
{
    mCache.insert(id, tz);
}

namespace {
template<typename T>
typename T::const_iterator greatestSmallerThan(const T &c, const typename T::value_type &v)
{
    auto it = std::lower_bound(c.cbegin(), c.cend(), v);
    if (it != c.cbegin()) {
        return --it;
    }
    return c.cend();
}

} // namespace

QTimeZone ICalTimeZoneCache::tzForTime(const QDateTime &dt, const QByteArray &tzid) const
{
    if (QTimeZone::isTimeZoneIdAvailable(tzid)) {
        return QTimeZone(tzid);
    }

    const ICalTimeZone tz = mCache.value(tzid);
    if (!tz.qZone.isValid()) {
        return QTimeZone::systemTimeZone();
    }

    // If the matched timezone is one of the UTC offset timezones, we need to make
    // sure it's in the correct DTS.
    // The lookup in ICalTimeZoneParser will only find TZ in standard time, but
    // if the datetim in question fits in the DTS zone, we need to use another UTC
    // offset timezone
    if (tz.qZone.id().startsWith("UTC")) { // krazy:exclude=strings
        // Find the nearest standard and DST transitions that occur BEFORE the "dt"
        const auto stdPrev = greatestSmallerThan(tz.standard.transitions, dt);
        const auto dstPrev = greatestSmallerThan(tz.daylight.transitions, dt);
        if (stdPrev != tz.standard.transitions.cend() && dstPrev != tz.daylight.transitions.cend()) {
            if (*dstPrev > *stdPrev) {
                // Previous DTS is closer to "dt" than previous standard, which
                // means we are in DTS right now
                const auto tzids = QTimeZone::availableTimeZoneIds(tz.daylight.utcOffset);
                auto dtsTzId = std::find_if(tzids.cbegin(), tzids.cend(), [](const QByteArray &id) {
                    return id.startsWith("UTC"); // krazy:exclude=strings
                });
                if (dtsTzId != tzids.cend()) {
                    return QTimeZone(*dtsTzId);
                }
            }
        }
    }

    return tz.qZone;
}

ICalTimeZoneParser::ICalTimeZoneParser(ICalTimeZoneCache *cache)
    : mCache(cache)
{
}

void ICalTimeZoneParser::updateTzEarliestDate(const IncidenceBase::Ptr &incidence, TimeZoneEarliestDate *earliest)
{
    for (auto role : {IncidenceBase::RoleStartTimeZone, IncidenceBase::RoleEndTimeZone}) {
        const auto dt = incidence->dateTime(role);
        if (dt.isValid()) {
            if (dt.timeZone() == QTimeZone::utc()) {
                continue;
            }
            const auto prev = earliest->value(incidence->dtStart().timeZone());
            if (!prev.isValid() || incidence->dtStart() < prev) {
                earliest->insert(incidence->dtStart().timeZone(), prev);
            }
        }
    }
}

icalcomponent *ICalTimeZoneParser::icalcomponentFromQTimeZone(const QTimeZone &tz, const QDateTime &earliest)
{
    // VTIMEZONE RRULE types
    enum {
        DAY_OF_MONTH = 0x01,
        WEEKDAY_OF_MONTH = 0x02,
        LAST_WEEKDAY_OF_MONTH = 0x04,
    };

    // Write the time zone data into an iCal component
    icalcomponent *tzcomp = icalcomponent_new(ICAL_VTIMEZONE_COMPONENT);
    icalcomponent_add_property(tzcomp, icalproperty_new_tzid(tz.id().constData()));
    //    icalcomponent_add_property(tzcomp, icalproperty_new_location( tz.name().toUtf8() ));

    // Compile an ordered list of transitions so that we can know the phases
    // which occur before and after each transition.
    QTimeZone::OffsetDataList transits = tz.transitions(QDateTime(), MAX_DATE());
    if (transits.isEmpty()) {
        // If there is no way to compile a complete list of transitions
        // transitions() can return an empty list
        // In that case try get one transition to write a valid VTIMEZONE entry.
        if (transits.isEmpty()) {
            qDebug() << "No transition information available VTIMEZONE will be invalid.";
        }
    }
    if (earliest.isValid()) {
        // Remove all transitions earlier than those we are interested in
        for (int i = 0, end = transits.count(); i < end; ++i) {
            if (transits.at(i).atUtc >= earliest) {
                if (i > 0) {
                    transits.erase(transits.begin(), transits.begin() + i);
                }
                break;
            }
        }
    }
    int trcount = transits.count();
    QVector<bool> transitionsDone(trcount, false);

    // Go through the list of transitions and create an iCal component for each
    // distinct combination of phase after and UTC offset before the transition.
    icaldatetimeperiodtype dtperiod;
    dtperiod.period = icalperiodtype_null_period();
    for (;;) {
        int i = 0;
        for (; i < trcount && transitionsDone[i]; ++i) {
            ;
        }
        if (i >= trcount) {
            break;
        }
        // Found a phase combination which hasn't yet been processed
        const int preOffset = (i > 0) ? transits.at(i - 1).offsetFromUtc : 0;
        const auto &transit = transits.at(i);
        if (transit.offsetFromUtc == preOffset) {
            transitionsDone[i] = true;
            while (++i < trcount) {
                if (transitionsDone[i] || transits.at(i).offsetFromUtc != transit.offsetFromUtc
                    || transits.at(i).daylightTimeOffset != transit.daylightTimeOffset || transits.at(i - 1).offsetFromUtc != preOffset) {
                    continue;
                }
                transitionsDone[i] = true;
            }
            continue;
        }
        const bool isDst = transit.daylightTimeOffset > 0;
        icalcomponent *phaseComp = icalcomponent_new(isDst ? ICAL_XDAYLIGHT_COMPONENT : ICAL_XSTANDARD_COMPONENT);
        if (!transit.abbreviation.isEmpty()) {
            icalcomponent_add_property(phaseComp, icalproperty_new_tzname(static_cast<const char *>(transit.abbreviation.toUtf8().constData())));
        }
        icalcomponent_add_property(phaseComp, icalproperty_new_tzoffsetfrom(preOffset));
        icalcomponent_add_property(phaseComp, icalproperty_new_tzoffsetto(transit.offsetFromUtc));
        // Create a component to hold initial RRULE if any, plus all RDATEs
        icalcomponent *phaseComp1 = icalcomponent_new_clone(phaseComp);
        icalcomponent_add_property(phaseComp1, icalproperty_new_dtstart(writeLocalICalDateTime(transits.at(i).atUtc, preOffset)));
        bool useNewRRULE = false;

        // Compile the list of UTC transition dates/times, and check
        // if the list can be reduced to an RRULE instead of multiple RDATEs.
        QTime time;
        QDate date;
        int year = 0, month = 0, daysInMonth = 0, dayOfMonth = 0; // avoid compiler warnings
        int dayOfWeek = 0; // Monday = 1
        int nthFromStart = 0; // nth (weekday) of month
        int nthFromEnd = 0; // nth last (weekday) of month
        int newRule;
        int rule = 0;
        QList<QDateTime> rdates; // dates which (probably) need to be written as RDATEs
        QList<QDateTime> times;
        QDateTime qdt = transits.at(i).atUtc; // set 'qdt' for start of loop
        times += qdt;
        transitionsDone[i] = true;
        do {
            if (!rule) {
                // Initialise data for detecting a new rule
                rule = DAY_OF_MONTH | WEEKDAY_OF_MONTH | LAST_WEEKDAY_OF_MONTH;
                time = qdt.time();
                date = qdt.date();
                year = date.year();
                month = date.month();
                daysInMonth = date.daysInMonth();
                dayOfWeek = date.dayOfWeek(); // Monday = 1
                dayOfMonth = date.day();
                nthFromStart = (dayOfMonth - 1) / 7 + 1; // nth (weekday) of month
                nthFromEnd = (daysInMonth - dayOfMonth) / 7 + 1; // nth last (weekday) of month
            }
            if (++i >= trcount) {
                newRule = 0;
                times += QDateTime(); // append a dummy value since last value in list is ignored
            } else {
                if (transitionsDone[i] || transits.at(i).offsetFromUtc != transit.offsetFromUtc
                    || transits.at(i).daylightTimeOffset != transit.daylightTimeOffset || transits.at(i - 1).offsetFromUtc != preOffset) {
                    continue;
                }
                transitionsDone[i] = true;
                qdt = transits.at(i).atUtc;
                if (!qdt.isValid()) {
                    continue;
                }
                newRule = rule;
                times += qdt;
                date = qdt.date();
                if (qdt.time() != time || date.month() != month || date.year() != ++year) {
                    newRule = 0;
                } else {
                    const int day = date.day();
                    if ((newRule & DAY_OF_MONTH) && day != dayOfMonth) {
                        newRule &= ~DAY_OF_MONTH;
                    }
                    if (newRule & (WEEKDAY_OF_MONTH | LAST_WEEKDAY_OF_MONTH)) {
                        if (date.dayOfWeek() != dayOfWeek) {
                            newRule &= ~(WEEKDAY_OF_MONTH | LAST_WEEKDAY_OF_MONTH);
                        } else {
                            if ((newRule & WEEKDAY_OF_MONTH) && (day - 1) / 7 + 1 != nthFromStart) {
                                newRule &= ~WEEKDAY_OF_MONTH;
                            }
                            if ((newRule & LAST_WEEKDAY_OF_MONTH) && (daysInMonth - day) / 7 + 1 != nthFromEnd) {
                                newRule &= ~LAST_WEEKDAY_OF_MONTH;
                            }
                        }
                    }
                }
            }
            if (!newRule) {
                // The previous rule (if any) no longer applies.
                // Write all the times up to but not including the current one.
                // First check whether any of the last RDATE values fit this rule.
                int yr = times[0].date().year();
                while (!rdates.isEmpty()) {
                    qdt = rdates.last();
                    date = qdt.date();
                    if (qdt.time() != time || date.month() != month || date.year() != --yr) {
                        break;
                    }
                    const int day = date.day();
                    if (rule & DAY_OF_MONTH) {
                        if (day != dayOfMonth) {
                            break;
                        }
                    } else {
                        if (date.dayOfWeek() != dayOfWeek || ((rule & WEEKDAY_OF_MONTH) && (day - 1) / 7 + 1 != nthFromStart)
                            || ((rule & LAST_WEEKDAY_OF_MONTH) && (daysInMonth - day) / 7 + 1 != nthFromEnd)) {
                            break;
                        }
                    }
                    times.prepend(qdt);
                    rdates.pop_back();
                }
                if (times.count() > (useNewRRULE ? minPhaseCount : minRuleCount)) {
                    // There are enough dates to combine into an RRULE
                    icalrecurrencetype r;
                    icalrecurrencetype_clear(&r);
                    r.freq = ICAL_YEARLY_RECURRENCE;
                    r.by_month[0] = month;
                    if (rule & DAY_OF_MONTH) {
                        r.by_month_day[0] = dayOfMonth;
                    } else if (rule & WEEKDAY_OF_MONTH) {
                        r.by_day[0] = (dayOfWeek % 7 + 1) + (nthFromStart * 8); // Sunday = 1
                    } else if (rule & LAST_WEEKDAY_OF_MONTH) {
                        r.by_day[0] = -(dayOfWeek % 7 + 1) - (nthFromEnd * 8); // Sunday = 1
                    }
                    r.until = writeLocalICalDateTime(times.takeAt(times.size() - 1), preOffset);
                    icalproperty *prop = icalproperty_new_rrule(r);
                    if (useNewRRULE) {
                        // This RRULE doesn't start from the phase start date, so set it into
                        // a new STANDARD/DAYLIGHT component in the VTIMEZONE.
                        icalcomponent *c = icalcomponent_new_clone(phaseComp);
                        icalcomponent_add_property(c, icalproperty_new_dtstart(writeLocalICalDateTime(times[0], preOffset)));
                        icalcomponent_add_property(c, prop);
                        icalcomponent_add_component(tzcomp, c);
                    } else {
                        icalcomponent_add_property(phaseComp1, prop);
                    }
                } else {
                    // Save dates for writing as RDATEs
                    for (int t = 0, tend = times.count() - 1; t < tend; ++t) {
                        rdates += times[t];
                    }
                }
                useNewRRULE = true;
                // All date/time values but the last have been added to the VTIMEZONE.
                // Remove them from the list.
                qdt = times.last(); // set 'qdt' for start of loop
                times.clear();
                times += qdt;
            }
            rule = newRule;
        } while (i < trcount);

        // Write remaining dates as RDATEs
        for (int rd = 0, rdend = rdates.count(); rd < rdend; ++rd) {
            dtperiod.time = writeLocalICalDateTime(rdates[rd], preOffset);
            icalcomponent_add_property(phaseComp1, icalproperty_new_rdate(dtperiod));
        }
        icalcomponent_add_component(tzcomp, phaseComp1);
        icalcomponent_free(phaseComp);
    }

    return tzcomp;
}

icaltimezone *ICalTimeZoneParser::icaltimezoneFromQTimeZone(const QTimeZone &tz, const QDateTime &earliest)
{
    auto itz = icaltimezone_new();
    icaltimezone_set_component(itz, icalcomponentFromQTimeZone(tz, earliest));
    return itz;
}

void ICalTimeZoneParser::parse(icalcomponent *calendar)
{
    for (auto *c = icalcomponent_get_first_component(calendar, ICAL_VTIMEZONE_COMPONENT); c;
         c = icalcomponent_get_next_component(calendar, ICAL_VTIMEZONE_COMPONENT)) {
        auto icalZone = parseTimeZone(c);
        // icalZone.dump();
        if (!icalZone.id.isEmpty()) {
            if (!icalZone.qZone.isValid()) {
                icalZone.qZone = resolveICalTimeZone(icalZone);
            }
            if (!icalZone.qZone.isValid()) {
                qWarning() << "Failed to map" << icalZone.id << "to a known IANA timezone";
                continue;
            }
            mCache->insert(icalZone.id, icalZone);
        }
    }
}

QTimeZone ICalTimeZoneParser::resolveICalTimeZone(const ICalTimeZone &icalZone)
{
    const auto phase = icalZone.standard;
    const auto now = QDateTime::currentDateTimeUtc();

    const auto candidates = QTimeZone::availableTimeZoneIds(phase.utcOffset);
    QMap<int, QTimeZone> matchedCandidates;
    for (const auto &tzid : candidates) {
        const QTimeZone candidate(tzid);
        // This would be a fallback, candidate has transitions, but the phase does not
        if (candidate.hasTransitions() == phase.transitions.isEmpty()) {
            matchedCandidates.insert(0, candidate);
            continue;
        }

        // Without transitions, we can't do any more precise matching, so just
        // accept this candidate and be done with it
        if (!candidate.hasTransitions() && phase.transitions.isEmpty()) {
            return candidate;
        }

        // Calculate how many transitions this candidate shares with the phase.
        // The candidate with the most matching transitions will win.
        auto begin = std::lower_bound(phase.transitions.cbegin(), phase.transitions.cend(), now.addYears(-20));
        // If no transition older than 20 years is found, we will start from beginning
        if (begin == phase.transitions.cend()) {
            begin = phase.transitions.cbegin();
        }
        auto end = std::upper_bound(begin, phase.transitions.cend(), now);
        int matchedTransitions = 0;
        for (auto it = begin; it != end; ++it) {
            const auto &transition = *it;
            const QTimeZone::OffsetDataList candidateTransitions = candidate.transitions(transition, transition);
            if (candidateTransitions.isEmpty()) {
                continue;
            }
            ++matchedTransitions; // 1 point for a matching transition
            const auto candidateTransition = candidateTransitions[0];
            // FIXME: THIS IS HOW IT SHOULD BE:
            // const auto abvs = transition.abbreviations();
            const auto abvs = phase.abbrevs;
            for (const auto &abv : abvs) {
                if (candidateTransition.abbreviation == QString::fromUtf8(abv)) {
                    matchedTransitions += 1024; // lots of points for a transition with a matching abbreviation
                    break;
                }
            }
        }
        matchedCandidates.insert(matchedTransitions, candidate);
    }

    if (!matchedCandidates.isEmpty()) {
        return matchedCandidates.value(matchedCandidates.lastKey());
    }

    return {};
}

ICalTimeZone ICalTimeZoneParser::parseTimeZone(icalcomponent *vtimezone)
{
    ICalTimeZone icalTz;

    if (auto tzidProp = icalcomponent_get_first_property(vtimezone, ICAL_TZID_PROPERTY)) {
        icalTz.id = icalproperty_get_value_as_string(tzidProp);

        // If the VTIMEZONE is a known IANA time zone don't bother parsing the rest
        // of the VTIMEZONE, get QTimeZone directly from Qt
        if (QTimeZone::isTimeZoneIdAvailable(icalTz.id)) {
            icalTz.qZone = QTimeZone(icalTz.id);
            return icalTz;
        } else {
            // Not IANA, but maybe we can match it from Windows ID?
            const auto ianaTzid = QTimeZone::windowsIdToDefaultIanaId(icalTz.id);
            if (!ianaTzid.isEmpty()) {
                icalTz.qZone = QTimeZone(ianaTzid);
                return icalTz;
            }
        }
    }

    for (icalcomponent *c = icalcomponent_get_first_component(vtimezone, ICAL_ANY_COMPONENT); c;
         c = icalcomponent_get_next_component(vtimezone, ICAL_ANY_COMPONENT)) {
        icalcomponent_kind kind = icalcomponent_isa(c);
        switch (kind) {
        case ICAL_XSTANDARD_COMPONENT:
            // qDebug() << "---standard phase: found";
            parsePhase(c, false, icalTz.standard);
            break;
        case ICAL_XDAYLIGHT_COMPONENT:
            // qDebug() << "---daylight phase: found";
            parsePhase(c, true, icalTz.daylight);
            break;

        default:
            qDebug() << "Unknown component:" << int(kind);
            break;
        }
    }

    return icalTz;
}

bool ICalTimeZoneParser::parsePhase(icalcomponent *c, bool daylight, ICalTimeZonePhase &phase)
{
    // Read the observance data for this standard/daylight savings phase
    int utcOffset = 0;
    int prevOffset = 0;
    bool recurs = false;
    bool found_dtstart = false;
    bool found_tzoffsetfrom = false;
    bool found_tzoffsetto = false;
    icaltimetype dtstart = icaltime_null_time();
    QSet<QByteArray> abbrevs;

    // Now do the ical reading.
    icalproperty *p = icalcomponent_get_first_property(c, ICAL_ANY_PROPERTY);
    while (p) {
        icalproperty_kind kind = icalproperty_isa(p);
        switch (kind) {
        case ICAL_TZNAME_PROPERTY: { // abbreviated name for this time offset
            // TZNAME can appear multiple times in order to provide language
            // translations of the time zone offset name.

            // TODO: Does this cope with multiple language specifications?
            QByteArray name = icalproperty_get_tzname(p);
            // Outlook (2000) places "Standard Time" and "Daylight Time" in the TZNAME
            // strings, which is totally useless. So ignore those.
            if ((!daylight && name == "Standard Time") || (daylight && name == "Daylight Time")) {
                break;
            }
            abbrevs.insert(name);
            break;
        }
        case ICAL_DTSTART_PROPERTY: // local time at which phase starts
            dtstart = icalproperty_get_dtstart(p);
            found_dtstart = true;
            break;

        case ICAL_TZOFFSETFROM_PROPERTY: // UTC offset immediately before start of phase
            prevOffset = icalproperty_get_tzoffsetfrom(p);
            found_tzoffsetfrom = true;
            break;

        case ICAL_TZOFFSETTO_PROPERTY:
            utcOffset = icalproperty_get_tzoffsetto(p);
            found_tzoffsetto = true;
            break;

        case ICAL_RDATE_PROPERTY:
        case ICAL_RRULE_PROPERTY:
            recurs = true;
            break;

        default:
            break;
        }
        p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
    }

    // Validate the phase data
    if (!found_dtstart || !found_tzoffsetfrom || !found_tzoffsetto) {
        qDebug() << "DTSTART/TZOFFSETFROM/TZOFFSETTO missing";
        return false;
    }

    // Convert DTSTART to QDateTime, and from local time to UTC
    dtstart.second -= prevOffset;
    dtstart = icaltime_convert_to_zone(dtstart, icaltimezone_get_utc_timezone());
    const QDateTime utcStart = toQDateTime(icaltime_normalize(dtstart)); // UTC

    phase.abbrevs.unite(abbrevs);
    phase.utcOffset = utcOffset;
    phase.transitions += utcStart;

    if (recurs) {
        /* RDATE or RRULE is specified. There should only be one or the other, but
         * it doesn't really matter - the code can cope with both.
         * Note that we had to get DTSTART, TZOFFSETFROM, TZOFFSETTO before reading
         * recurrences.
         */
        const QDateTime maxTime(MAX_DATE());
        Recurrence recur;
        icalproperty *p = icalcomponent_get_first_property(c, ICAL_ANY_PROPERTY);
        while (p) {
            icalproperty_kind kind = icalproperty_isa(p);
            switch (kind) {
            case ICAL_RDATE_PROPERTY: {
                icaltimetype t = icalproperty_get_rdate(p).time;
                if (icaltime_is_date(t)) {
                    // RDATE with a DATE value inherits the (local) time from DTSTART
                    t.hour = dtstart.hour;
                    t.minute = dtstart.minute;
                    t.second = dtstart.second;
                    t.is_date = 0;
                }
                // RFC2445 states that RDATE must be in local time,
                // but we support UTC as well to be safe.
                if (!icaltime_is_utc(t)) {
                    t.second -= prevOffset; // convert to UTC
                    t = icaltime_convert_to_zone(t, icaltimezone_get_utc_timezone());
                    t = icaltime_normalize(t);
                }
                phase.transitions += toQDateTime(t);
                break;
            }
            case ICAL_RRULE_PROPERTY: {
                RecurrenceRule r;
                ICalFormat icf;
                ICalFormatImpl impl(&icf);
                impl.readRecurrence(icalproperty_get_rrule(p), &r);
                r.setStartDt(utcStart);
                // The end date time specified in an RRULE must be in UTC.
                // We can not guarantee correctness if this is not the case.
                if (r.duration() == 0 && r.endDt().timeSpec() != Qt::UTC) {
                    qWarning() << "UNTIL in RRULE must be specified in UTC";
                    break;
                }
                const auto dts = r.timesInInterval(utcStart, maxTime);
                for (int i = 0, end = dts.count(); i < end; ++i) {
                    phase.transitions += dts[i];
                }
                break;
            }
            default:
                break;
            }
            p = icalcomponent_get_next_property(c, ICAL_ANY_PROPERTY);
        }
        sortAndRemoveDuplicates(phase.transitions);
    }

    return true;
}

QByteArray ICalTimeZoneParser::vcaltimezoneFromQTimeZone(const QTimeZone &qtz, const QDateTime &earliest)
{
    auto icalTz = icalcomponentFromQTimeZone(qtz, earliest);
    const QByteArray result(icalcomponent_as_ical_string(icalTz));
    icalmemory_free_ring();
    icalcomponent_free(icalTz);
    return result;
}

} // namespace KCalendarCore
