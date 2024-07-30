/*
  This file is part of the kcalcore library.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "event.h"
#include "freebusy.h"
#include "icalformat.h"

#include <QDebug>
#include <QTest>

using namespace KCalendarCore;

class TestFb : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testFbLoad()
    {
        const QString fbString = QStringLiteral(
            "BEGIN:VCALENDAR\n"
            "PRODID:-//proko2//freebusy 1.0//EN\n"
            "METHOD:PUBLISH\n"
            "VERSION:2.0\n"
            "BEGIN:VFREEBUSY\n"
            "ORGANIZER:MAILTO:test3@kdab.net\n"
            "X-KDE-Foo:bla\n"
            "DTSTAMP:20071202T152453Z\n"
            "URL:http://mail.kdab.net/freebusy/test3%40kdab.net.ifb\n"
            "DTSTART:19700101T000000Z\n"
            "DTEND:200700101T000000Z\n"
            "COMMENT:This is a dummy vfreebusy that indicates an empty calendar\n"
            "FREEBUSY:19700101T000000Z/19700101T000000Z\n"
            "FREEBUSY;X-UID=bGlia2NhbC0xODk4MjgxNTcuMTAxMA==;X-\n"
            " SUMMARY=RW1wbG95ZWUgbWVldGluZw==;X-LOCATION=Um9vb\n"
            " SAyMTM=:20080131T170000Z/20080131T174500Z\n"
            "END:VFREEBUSY\n"
            "END:VCALENDAR\n");

        ICalFormat format;
        FreeBusy::Ptr fb = format.parseFreeBusy(fbString);
        QCOMPARE(fb->fullBusyPeriods().size(), 2);
        QCOMPARE(fb->dtStart(), QDateTime({1970, 1, 1}, {0, 0}, QTimeZone::UTC));
        const FreeBusyPeriod::List lst = fb->fullBusyPeriods();
        const auto &freebusy1 = lst.at(0);
        QCOMPARE(freebusy1.start(), QDateTime({1970, 1, 1}, {0, 0}, QTimeZone::UTC));
        QCOMPARE(freebusy1.end(), QDateTime({1970, 1, 1}, {0, 0}, QTimeZone::UTC));
        QCOMPARE(freebusy1.summary(), QString());
        QCOMPARE(freebusy1.location(), QString());

        const auto &freebusy2 = lst.at(1);
        QCOMPARE(freebusy2.start(), QDateTime({2008, 1, 31}, {17, 0}, QTimeZone::UTC));
        QCOMPARE(freebusy2.end(), QDateTime({2008, 1, 31}, {17, 45}, QTimeZone::UTC));
        QCOMPARE(freebusy2.summary(), QLatin1String("Employee meeting"));
        QCOMPARE(freebusy2.location(), QLatin1String("Room 213"));

        const QMap<QByteArray, QString> props = fb->customProperties();
        QCOMPARE(props.size(), 1);
        QCOMPARE(props.firstKey(), "X-KDE-Foo");
        QCOMPARE(props.first(), QLatin1String("bla"));
    }
};

QTEST_APPLESS_MAIN(TestFb)

#include "testfb.moc"
