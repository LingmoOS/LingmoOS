/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2005-2007 David Jarvie <djarvie@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "testicaltimezones.h"
#include "icaltimezones_p.h"

#include <QDateTime>

#include <QTest>

QTEST_MAIN(ICalTimeZonesTest)

extern "C" {
#include <libical/ical.h>
}
using namespace KCalendarCore;

static icalcomponent *loadCALENDAR(const char *vcal);

// First daylight savings time has an end date, takes a break for a year,
// and is then replaced by another
static const char *VTZ_Western =
    "BEGIN:VTIMEZONE\r\n"
    "TZID:Test-Dummy-Western\r\n"
    "LAST-MODIFIED:19870101T000000Z\r\n"
    "TZURL:http://tz.reference.net/dummies/western\r\n"
    "LOCATION:Zedland/Tryburgh\r\n"
    "X-LIC-LOCATION:Wyland/Tryburgh\r\n"
    "BEGIN:STANDARD\r\n"
    "DTSTART:19671029T020000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10\r\n"
    "TZOFFSETFROM:-0400\r\n"
    "TZOFFSETTO:-0500\r\n"
    "TZNAME:WST\r\n"
    "END:STANDARD\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "DTSTART:19870405T020000\r\n"
    "RRULE:FREQ=YEARLY;UNTIL=19970406T070000Z;BYDAY=1SU;BYMONTH=4\r\n"
    "TZOFFSETFROM:-0500\r\n"
    "TZOFFSETTO:-0400\r\n"
    "TZNAME:WDT1\r\n"
    "END:DAYLIGHT\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "DTSTART:19990425T020000\r\n"
    "RDATE;VALUE=DATE-TIME:20000430T020000\r\n"
    "TZOFFSETFROM:-0500\r\n"
    "TZOFFSETTO:-0400\r\n"
    "TZNAME:WDT2\r\n"
    "END:DAYLIGHT\r\n"
    "END:VTIMEZONE\r\n";

// Standard time only
static const char *VTZ_other =
    "BEGIN:VTIMEZONE\r\n"
    "TZID:Test-Dummy-Other\r\n"
    "TZURL:http://tz.reference.net/dummies/other\r\n"
    "X-LIC-LOCATION:Wyland/Tryburgh\r\n"
    "BEGIN:STANDARD\r\n"
    "DTSTART:19500101T000000\r\n"
    "RDATE;VALUE=DATE-TIME:19500101T000000\r\n"
    "TZOFFSETFROM:+0000\r\n"
    "TZOFFSETTO:+0300\r\n"
    "TZNAME:OST\r\n"
    "END:STANDARD\r\n"
    "END:VTIMEZONE\r\n";

static const char *VTZ_other_DST =
    "BEGIN:VTIMEZONE\r\n"
    "TZID:Test-Dummy-Other-DST\r\n"
    "BEGIN:STANDARD\r\n"
    "DTSTART:19500101T000000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=1SU;BYMONTH=11\r\n"
    "TZOFFSETFROM:+0000\r\n"
    "TZOFFSETTO:+0300\r\n"
    "TZNAME:OST\r\n"
    "END:STANDARD\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "DTSTART:19500501T000000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=3SU;BYMONTH=5\r\n"
    "TZOFFSETFROM:+0200\r\n"
    "TZOFFSETTO:+0500\r\n"
    "TZNAME:DST\r\n"
    "END:DAYLIGHT\r\n"
    "END:VTIMEZONE\r\n";

static const char *VTZ_Prague =
    "BEGIN:VTIMEZONE\r\n"
    "TZID:Europe/Prague\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "TZNAME:CEST\r\n"
    "TZOFFSETFROM:+0000\r\n"
    "TZOFFSETTO:+0200\r\n"
    "DTSTART:19790401T010000\r\n"
    "RDATE;VALUE=DATE-TIME:19790401T010000\r\n"
    "END:DAYLIGHT\r\n"
    "BEGIN:STANDARD\r\n"
    "TZNAME:CET\r\n"
    "TZOFFSETFROM:+0200\r\n"
    "TZOFFSETTO:+0100\r\n"
    "DTSTART:19971026T030000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10\r\n"
    "END:STANDARD\r\n"
    "BEGIN:STANDARD\r\n"
    "TZNAME:CET\r\n"
    "TZOFFSETFROM:+0200\r\n"
    "TZOFFSETTO:+0100\r\n"
    "DTSTART:19790930T030000\r\n"
    "RRULE:FREQ=YEARLY;UNTIL=19961027T030000;BYDAY=-1SU;BYMONTH=9\r\n"
    "RDATE;VALUE=DATE-TIME:19950924T030000\r\n"
    "END:STANDARD\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "TZNAME:CEST\r\n"
    "TZOFFSETFROM:+0100\r\n"
    "TZOFFSETTO:+0200\r\n"
    "DTSTART:19810329T020000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=3\r\n"
    "END:DAYLIGHT\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "TZNAME:CEST\r\n"
    "TZOFFSETFROM:+0100\r\n"
    "TZOFFSETTO:+0200\r\n"
    "DTSTART:19800406T020000\r\n"
    "RDATE;VALUE=DATE-TIME:19800406T020000\r\n"
    "END:DAYLIGHT\r\n"
    "END:VTIMEZONE\r\n";

// When there's an extra transition from +0000 to +0100
// in 1978 (FreeBSD and old Debian), we get one more
// transition and slightly different RRULEs
#ifdef Q_OS_FREEBSD
static const char *VTZ_PragueExtra =
    "BEGIN:VTIMEZONE\r\n"
    "TZID:Europe/Prague\r\n"
    "BEGIN:STANDARD\r\n"
    "TZNAME:CET\r\n"
    "TZOFFSETFROM:+0000\r\n"
    "TZOFFSETTO:+0100\r\n"
    "DTSTART:19781231T230000\r\n"
    "RDATE:19781231T230000\r\n"
    "END:STANDARD\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "TZNAME:CEST\r\n"
    "TZOFFSETFROM:+0100\r\n"
    "TZOFFSETTO:+0200\r\n"
    "DTSTART:19810329T020000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=3\r\n"
    "END:DAYLIGHT\r\n"
    "BEGIN:DAYLIGHT\r\n"
    "TZNAME:CEST\r\n"
    "TZOFFSETFROM:+0100\r\n"
    "TZOFFSETTO:+0200\r\n"
    "DTSTART:19790401T020000\r\n"
    "RDATE:19790401T020000\r\n"
    "RDATE:19800406T020000\r\n"
    "END:DAYLIGHT\r\n"
    "BEGIN:STANDARD\r\n"
    "TZNAME:CET\r\n"
    "TZOFFSETFROM:+0200\r\n"
    "TZOFFSETTO:+0100\r\n"
    "DTSTART:19971026T030000\r\n"
    "RRULE:FREQ=YEARLY;BYDAY=-1SU;BYMONTH=10\r\n"
    "END:STANDARD\r\n"
    "BEGIN:STANDARD\r\n"
    "TZNAME:CET\r\n"
    "TZOFFSETFROM:+0200\r\n"
    "TZOFFSETTO:+0100\r\n"
    "DTSTART:19790930T030000\r\n"
    "RRULE:FREQ=YEARLY;UNTIL=19961027T030000;BYDAY=-1SU;BYMONTH=9\r\n"
    "RDATE:19950924T030000\r\n"
    "END:STANDARD\r\n"
    "END:VTIMEZONE\r\n";
#endif

// CALENDAR component header and footer
static const char *calendarHeader =
    "BEGIN:VCALENDAR\r\n"
    "PRODID:-//Libkcal//NONSGML ICalTimeZonesTest//EN\r\n"
    "VERSION:2.0\r\n";
static const char *calendarFooter = "END:CALENDAR\r\n";

///////////////////////////
// ICalTimeZoneSource tests
///////////////////////////

void ICalTimeZonesTest::initTestCase()
{
    qputenv("TZ", "Europe/Zurich");
}

void ICalTimeZonesTest::parse_data()
{
    QTest::addColumn<QByteArray>("vtimezone");
    QTest::addColumn<QDateTime>("onDate");
    QTest::addColumn<QByteArray>("origTz");
    QTest::addColumn<QByteArray>("expTz");

    if (false) {
        // TODO Fix this test
        QTest::newRow("dummy-western") << QByteArray(VTZ_Western) << QDateTime{} << QByteArray("Test-Dummy-Western") << QByteArray("America/Toronto");
    }
    QTest::newRow("dummy-other") << QByteArray(VTZ_other) << QDateTime{} << QByteArray("Test-Dummy-Other") << QByteArray("UTC+03:00");
    QTest::newRow("dummy-other-dst DST") << QByteArray(VTZ_other_DST) << QDateTime({2017, 03, 10}, {}) << QByteArray("Test-Dummy-Other-DST")
                                         << QByteArray("UTC+03:00");
    QTest::newRow("dummy-other-dst STD") << QByteArray(VTZ_other_DST) << QDateTime({2017, 07, 05}, {}) << QByteArray("Test-Dummy-Other-DST")
                                         << QByteArray("UTC+05:00");
    QTest::newRow("dummy-other-dst DST after") << QByteArray(VTZ_other_DST) << QDateTime({2017, 12, 24}, {}) << QByteArray("Test-Dummy-Other-DST")
                                               << QByteArray("UTC+03:00");
    QTest::newRow("iana") << QByteArray() << QDateTime({2017, 9, 14}, {}) << QByteArray("Europe/Zurich") << QByteArray("Europe/Zurich");
}

void ICalTimeZonesTest::parse()
{
    QFETCH(QByteArray, vtimezone);
    QFETCH(QDateTime, onDate);
    QFETCH(QByteArray, origTz);
    QFETCH(QByteArray, expTz);

    QByteArray calText(calendarHeader);
    calText += vtimezone;
    calText += calendarFooter;

    auto vcalendar = loadCALENDAR(calText.constData());

    ICalTimeZoneCache timezones;
    ICalTimeZoneParser parser(&timezones);
    parser.parse(vcalendar);

    icalcomponent_free(vcalendar);

    QCOMPARE(timezones.tzForTime(onDate, origTz).id(), expTz);
}

void ICalTimeZonesTest::write()
{
    /* By picking a date close to the TZ transition, we avoid
     * picking up FreeBSD's spurious transition at the end of
     * 1978 (see testPragueTransitions, below). The starting date
     * **was** 1970, which ought to get a starting TZ transition in
     * 1979 (the previous one was 1949, which is out-of-scope).
     * However, that gets one extra transition of FreeBSD,
     * which fails the test.
     */
    {
        auto vtimezone = ICalTimeZoneParser::vcaltimezoneFromQTimeZone(QTimeZone("Europe/Prague"), QDateTime({1979, 2, 1}, {0, 0}));
        QCOMPARE(vtimezone, QByteArray(VTZ_Prague).replace(";VALUE=DATE-TIME", "")); // krazy:exclude=doublequote_chars
    }

    /* By picking a date which overlaps the spurious TZ transition,
     * we get a different output, but only on FreeBSD (and old Debian).
     */
    {
        auto vtimezone = ICalTimeZoneParser::vcaltimezoneFromQTimeZone(QTimeZone("Europe/Prague"), QDateTime({1970, 1, 1}, {0, 0}));
#ifdef Q_OS_FREEBSD
        // The result is quite different: besides the extra
        // transition, the RRULEs that are generated differ as well.
        auto expect = QByteArray(VTZ_PragueExtra);
#else
        auto expect = QByteArray(VTZ_Prague);
#endif
        expect.replace(";VALUE=DATE-TIME", ""); // krazy:exclude=doublequote_chars
        QCOMPARE(vtimezone, expect);
    }
}

icalcomponent *loadCALENDAR(const char *vcal)
{
    icalcomponent *calendar = icalcomponent_new_from_string(const_cast<char *>(vcal));
    if (calendar) {
        if (icalcomponent_isa(calendar) == ICAL_VCALENDAR_COMPONENT) {
            return calendar;
        }
        icalcomponent_free(calendar);
    }
    return nullptr;
}

void ICalTimeZonesTest::testPragueTransitions()
{
    QTimeZone prague("Europe/Prague");
    QVERIFY(prague.isValid());

    /* The transitions for Prague, according to tzdata version 2020a,
     * from 1949 to 1979, are the following, from the command
     *      `cd /usr/share/zoneinfo ; zdump -v Europe/Prague | grep 19[47]9`
     * It was manually verified that there were no transitions in
     * intermediate years.
     *
     * There are therefore 2 transitions between june 1949 and june 1979:
     *  - fall back to CET in october 1949
     *  - spring forward to CEST in april 1979
     */
    /*
    Europe/Prague  Sat Apr  9 00:59:59 1949 UTC = Sat Apr  9 01:59:59 1949 CET isdst=0 gmtoff=3600
    Europe/Prague  Sat Apr  9 01:00:00 1949 UTC = Sat Apr  9 03:00:00 1949 CEST isdst=1 gmtoff=7200
    Europe/Prague  Sun Oct  2 00:59:59 1949 UTC = Sun Oct  2 02:59:59 1949 CEST isdst=1 gmtoff=7200
    Europe/Prague  Sun Oct  2 01:00:00 1949 UTC = Sun Oct  2 02:00:00 1949 CET isdst=0 gmtoff=3600
    Europe/Prague  Sun Apr  1 00:59:59 1979 UTC = Sun Apr  1 01:59:59 1979 CET isdst=0 gmtoff=3600
    Europe/Prague  Sun Apr  1 01:00:00 1979 UTC = Sun Apr  1 03:00:00 1979 CEST isdst=1 gmtoff=7200
    Europe/Prague  Sun Sep 30 00:59:59 1979 UTC = Sun Sep 30 02:59:59 1979 CEST isdst=1 gmtoff=7200
    Europe/Prague  Sun Sep 30 01:00:00 1979 UTC = Sun Sep 30 02:00:00 1979 CET isdst=0 gmtoff=3600
    */

    const auto &transitions = prague.transitions(QDateTime({1949, 6, 6}, {0, 0}), QDateTime({1979, 6, 6}, {0, 0}));
    QVERIFY(transitions.count() > 0);
    const auto &earliest = transitions.first().atUtc;
    QCOMPARE(earliest.date(), QDate(1949, 10, 2));
    QCOMPARE(transitions.last().atUtc.date(), QDate(1979, 4, 1));

    bool occursIn1978 = false;
    for (const auto &transition : transitions) {
        if (transition.atUtc.date().year() == 1978) {
            occursIn1978 = true;
        }
    }

/* Except that the FreeBSD zic (zone-info-compiler, which is separate from
 * zdump) produces a slightly different output file than zic on Linux,
 * for instance they have different sizes:
 *  2312 Prague-freebsd
 *  2338 Prague-linux
 * The `zdump -v` output is the same for the two files. The Linux version
 * contains one extra tzh_ttisgmtcnt and one extra tzh_ttisstdcnt entry.
 *
 * Whatever the precise cause, on loading the TZ file, on FreeBSD
 * Qt deduces an extra transition, at the end of 1978, with no change in
 * offzet or zone name:
 *  QDateTime(1949-10-02 01:00:00.000 UTC Qt::UTC) "CET" 3600
 *  QDateTime(1978-12-31 23:00:00.000 UTC Qt::UTC) "CET" 3600
 *  QDateTime(1979-04-01 01:00:00.000 UTC Qt::UTC) "CEST" 7200
 *
 * This additional transition makes the test fail.
 */
#ifdef Q_OS_FREEBSD
    QEXPECT_FAIL("", "FreeBSD (and old Debian) zic produces extra transition", Continue);
#endif
    QCOMPARE(transitions.count(), 2);

#ifdef Q_OS_FREEBSD
    QEXPECT_FAIL("", "FreeBSD (and old Debian) zic produces extra transition", Continue);
#endif
    QVERIFY(!occursIn1978);
}

#include "moc_testicaltimezones.cpp"
