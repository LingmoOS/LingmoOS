/*
 *  SPDX-FileCopyrightText: 2022 Glen Ditchfield <GJDitchfield@acm.org>
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "incidencebase.h"

#include <QTest>
#include <QTimeZone>

using namespace KCalendarCore;

class TestIdentical: public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testTrue_data()
    {
        QTest::addColumn<QDateTime>("dt1");
        QTest::addColumn<QDateTime>("dt2");

        QTest::newRow("local") << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone::LocalTime)
                               << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone::LocalTime);

        QTest::newRow("tz") << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone("Atlantic/Azores"))
                            << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone("Atlantic/Azores"));

        QTest::newRow("invalid") << QDateTime() << QDateTime();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        QTest::newRow("utc-tz") << QDateTime(QDate(2023, 11, 18), QTime(13, 31), Qt::UTC) << QDateTime(QDate(2023, 11, 18), QTime(13, 31), QTimeZone::utc());
        QTest::newRow("tz-newoffset") << QDateTime(QDate(2023, 11, 18), QTime(13, 31), Qt::UTC)
                                      << QDateTime(QDate(2023, 11, 18), QTime(13, 31), QTimeZone::fromSecondsAheadOfUtc(0));
        QTest::newRow("utc-newoffset") << QDateTime(QDate(2023, 11, 18), QTime(13, 31), Qt::UTC)
                                       << QDateTime(QDate(2023, 11, 18), QTime(13, 31), QTimeZone::fromSecondsAheadOfUtc(0));
        QTest::newRow("utc-oldoffset-newoffset") << QDateTime(QDate(2023, 11, 18), QTime(13, 31), Qt::OffsetFromUTC, 0)
                                                 << QDateTime(QDate(2023, 11, 18), QTime(13, 31), QTimeZone::fromSecondsAheadOfUtc(0));

        QTest::newRow("offset") << QDateTime(QDate(2023, 11, 18), QTime(13, 31), Qt::OffsetFromUTC, 3600)
                                << QDateTime(QDate(2023, 11, 18), QTime(13, 31), QTimeZone::fromSecondsAheadOfUtc(3600));
#pragma GCC diagnostic pop
    }

    void testTrue()
    {
        QFETCH(QDateTime, dt1);
        QFETCH(QDateTime, dt2);

        QVERIFY(identical(dt1, dt2));
        QVERIFY(identical(dt2, dt1));
    }

    void testFalse_data()
    {
        QTest::addColumn<QDateTime>("dt1");
        QTest::addColumn<QDateTime>("dt2");

        QTest::newRow("date") << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone::LocalTime)
                              << QDateTime(QDate(2022, 02, 23), QTime(22, 22, 22), QTimeZone::LocalTime);
        QTest::newRow("time") << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone::LocalTime)
                              << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 23), QTimeZone::LocalTime);

        // Note:  QTimeZone::LocalTime is used for "floating" date-times; "this time
        // in the current system time zone".
        QTest::newRow("timeSpec") << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone::LocalTime)
                                  << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone::systemTimeZone());

        // Both zones are UTC+0.
        QTest::newRow("timeZone")
            << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone("Africa/Abidjan"))
            << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QTimeZone("Africa/Accra"));

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        QTest::newRow("invalid timeSpec")
            << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QDateTime().timeSpec())
            << QDateTime();
#pragma GCC diagnostic pop
        QTest::newRow("invalid timeZone")
            << QDateTime(QDate(2022, 02, 22), QTime(22, 22, 22), QDateTime().timeZone())
            << QDateTime();
    }

    void testFalse()
    {
        QFETCH(QDateTime, dt1);
        QFETCH(QDateTime, dt2);
        QVERIFY(!identical(dt1, dt2));
    }
};

QTEST_MAIN(TestIdentical)
#include "testidentical.moc"
