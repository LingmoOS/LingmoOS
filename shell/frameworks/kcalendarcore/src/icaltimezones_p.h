/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2005-2007 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCALCORE_ICALTIMEZONES_P_H
#define KCALCORE_ICALTIMEZONES_P_H

#include "incidencebase.h"
#include "kcalendarcore_export.h"

#include <QHash>
#include <QList>
#include <QTimeZone>

#ifndef ICALCOMPONENT_H
typedef struct icalcomponent_impl icalcomponent;
#endif
#ifndef ICALTIMEZONE_DEFINED
#define ICALTIMEZONE_DEFINED
typedef struct _icaltimezone icaltimezone;
#endif

namespace KCalendarCore
{
class ICalTimeZonePhase;

class ICalTimeZonePhase
{
public:
    void dump();

    QSet<QByteArray> abbrevs; // abbreviations of the phase
    int utcOffset = 0; // UTC offset of the phase
    QList<QDateTime> transitions; // times on which transition into this phase occurs
};

class ICalTimeZone
{
public:
    void dump();

    QByteArray id; // original TZID
    QTimeZone qZone; // QTimeZone mapped from TZID
    ICalTimeZonePhase standard; // standard time
    ICalTimeZonePhase daylight; // DST time
};

class KCALENDARCORE_EXPORT ICalTimeZoneCache
{
public:
    explicit ICalTimeZoneCache();

    void insert(const QByteArray &id, const ICalTimeZone &tz);

    QTimeZone tzForTime(const QDateTime &dt, const QByteArray &tzid) const;

private:
    QHash<QByteArray, ICalTimeZone> mCache;
};

using TimeZoneEarliestDate = QHash<QTimeZone, QDateTime>;

class KCALENDARCORE_EXPORT ICalTimeZoneParser
{
public:
    explicit ICalTimeZoneParser(ICalTimeZoneCache *cache);

    void parse(icalcomponent *calendar);

    static void updateTzEarliestDate(const IncidenceBase::Ptr &incidence, TimeZoneEarliestDate *earliestDate);

    static icaltimezone *icaltimezoneFromQTimeZone(const QTimeZone &qtz, const QDateTime &earliest);
    static QByteArray vcaltimezoneFromQTimeZone(const QTimeZone &qtz, const QDateTime &earliest);

private:
    static icalcomponent *icalcomponentFromQTimeZone(const QTimeZone &qtz, const QDateTime &earliest);

    ICalTimeZone parseTimeZone(icalcomponent *zone);
    bool parsePhase(icalcomponent *c, bool daylight, ICalTimeZonePhase &phase);
    QTimeZone resolveICalTimeZone(const ICalTimeZone &icalZone);

    ICalTimeZoneCache *mCache;
};

} // namespace KCalendarCore

inline size_t qHash(const QTimeZone &tz, size_t seed = 0)
{
    return qHash(tz.id(), seed);
}

#endif
