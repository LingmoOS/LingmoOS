/*
    This file is part of kcontacts.
    SPDX-FileCopyrightText: 2016 Benjamin Löwe <benni@mytum.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "datetimetest.h"
#include <QTest>
#include <QTimeZone>
#include <vcardtool_p.h>

DateTimeTest::DateTimeTest(QObject *parent)
    : QObject(parent)
{
}

DateTimeTest::~DateTimeTest()
{
}

void DateTimeTest::shouldParseDateTime()
{
    using namespace KContacts;

    QDateTime expected;
    QDateTime dt;
    bool timeIsValid;

    dt = VCardTool::parseDateTime(QStringLiteral("2016-01-20T12:33:30+0200"));
    expected = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30), QTimeZone::fromSecondsAheadOfUtc(2 * 3600));
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("2016-01-20T12:33:30+02"));
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("20160120T123330+0200"));
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("2016-01-20T12:33Z"));
    expected = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 0), QTimeZone::UTC);
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("2016-01-20T12:33-0300"));
    expected = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 0), QTimeZone::fromSecondsAheadOfUtc(-3 * 3600));
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("2016-01-20T12:33"), &timeIsValid);
    expected = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 0), QTimeZone::LocalTime);
    QCOMPARE(dt, expected);
    QVERIFY(timeIsValid);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("2016-01-20"), &timeIsValid);
    expected = QDateTime(QDate(2016, 1, 20), QTime(), QTimeZone::LocalTime);
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());
    QVERIFY(!timeIsValid);
    QCOMPARE(dt.time(), QTime(0, 0));

    dt = VCardTool::parseDateTime(QStringLiteral("T1233Z"), &timeIsValid);
    QVERIFY(!dt.isValid());
    QVERIFY(timeIsValid);
    QCOMPARE(dt.time(), QTime(12, 33));

    dt = VCardTool::parseDateTime(QStringLiteral("--0120"));
    expected = QDateTime(QDate(-1, 1, 20).startOfDay());
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());

    dt = VCardTool::parseDateTime(QStringLiteral("--01-20"));
    expected = QDateTime(QDate(-1, 1, 20).startOfDay());
    QCOMPARE(dt, expected);
    QVERIFY(dt.isValid());
}

void DateTimeTest::shouldCreateDateTime()
{
    using namespace KContacts;

    QDateTime dt;
    QString str;
    QString expected;

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30), QTimeZone::UTC);
    str = VCardTool::createDateTime(dt, VCard::v4_0);
    expected = QStringLiteral("20160120T123330Z");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(), QTimeZone::UTC);
    str = VCardTool::createDateTime(dt, VCard::v4_0);
    expected = QStringLiteral("20160120T000000Z");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30), QTimeZone::LocalTime);
    str = VCardTool::createDateTime(dt, VCard::v4_0);
    expected = QStringLiteral("20160120T123330");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30), QTimeZone::fromSecondsAheadOfUtc(-2 * 3600));
    str = VCardTool::createDateTime(dt, VCard::v4_0);
    expected = QStringLiteral("20160120T123330-0200");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30), QTimeZone::fromSecondsAheadOfUtc(+2.5 * 3600));
    str = VCardTool::createDateTime(dt, VCard::v4_0);
    expected = QStringLiteral("20160120T123330+0230");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30), QTimeZone::fromSecondsAheadOfUtc(-2 * 3600));
    str = VCardTool::createDateTime(dt, VCard::v3_0);
    expected = QStringLiteral("2016-01-20T12:33:30-02:00");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(), QTime(12, 33, 30), QTimeZone::fromSecondsAheadOfUtc(-2 * 3600));
    str = VCardTool::createDateTime(dt, VCard::v3_0);
    expected = QString();
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30));
    str = VCardTool::createDateTime(dt, VCard::v3_0, false);
    expected = QStringLiteral("2016-01-20");
    QCOMPARE(str, expected);

    dt = QDateTime(QDate(2016, 1, 20), QTime(12, 33, 30));
    str = VCardTool::createDateTime(dt, VCard::v4_0, false);
    expected = QStringLiteral("20160120");
    QCOMPARE(str, expected);
}

void DateTimeTest::shouldCreateDate()
{
    using namespace KContacts;

    QDate d;
    QString str;
    QString expected;

    d = QDate(2016, 1, 20);
    str = VCardTool::createDate(d, VCard::v3_0);
    expected = QStringLiteral("2016-01-20");
    QCOMPARE(str, expected);

    d = QDate(2016, 1, 20);
    str = VCardTool::createDate(d, VCard::v4_0);
    expected = QStringLiteral("20160120");
    QCOMPARE(str, expected);

    d = QDate(-1, 1, 20);
    str = VCardTool::createDate(d, VCard::v3_0);
    expected = QStringLiteral("--01-20");
    QCOMPARE(str, expected);

    d = QDate(-1, 1, 20);
    str = VCardTool::createDate(d, VCard::v4_0);
    expected = QStringLiteral("--0120");
    QCOMPARE(str, expected);
}

QTEST_MAIN(DateTimeTest)

#include "moc_datetimetest.cpp"
