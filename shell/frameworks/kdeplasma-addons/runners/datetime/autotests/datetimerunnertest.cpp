/*
    SPDX-FileCopyrightText: 2023 Natalie Clarius <natalie_clarius@yahoo.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include <KRunner/AbstractRunnerTest>

#include <QDateTime>
#include <QTimeZone>

#include <qtestcase.h>

using namespace Qt::StringLiterals;
using namespace KRunner;

class DateTimeRunnerTest : public AbstractRunnerTest
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testLocalTimeInfo();
    void testRemoteTimeInfo();
    void testFindTimezones_data();
    void testFindTimezones();
    void testConversion_data();
    void testConversion();
};

#ifndef Q_OS_WIN
void initEnv()
{
    setenv("LC_ALL", "en_US.UTF-8", 1);
    setenv("TZ", "GMT", 1);
}
Q_CONSTRUCTOR_FUNCTION(initEnv)
#endif

void DateTimeRunnerTest::initTestCase()
{
    initProperties();
}

void DateTimeRunnerTest::testLocalTimeInfo()
{
    const QTime localTime = QDateTime::currentDateTime().time();
    const QString timeStr = QLocale().toString(localTime);

    launchQuery("time");

    QCOMPARE(manager->matches().count(), 1);
    QVERIFY(manager->matches().first().text().contains(timeStr));
}

void DateTimeRunnerTest::testRemoteTimeInfo()
{
    // See the definition of GMT in https://data.iana.org/time-zones/releases/tzdata2023c.tar.gz -> etcetera
    //
    // Be consistent with POSIX TZ settings in the Zone names,
    // even though this is the opposite of what many people expect.
    // POSIX has positive signs west of Greenwich, but many people expect
    // positive signs east of Greenwich.For example, TZ = 'Etc/GMT+4' uses
    // the abbreviation "-04" and corresponds to 4 hours behind UT
    // (i.e.west of Greenwich) even though many people would expect it to
    // mean 4 hours ahead of UT(i.e.east of Greenwich).

    const QTime remoteTime = QDateTime::currentDateTime().toTimeZone(QTimeZone("GMT-2")).time();
    const QString timeStr = QLocale().toString(remoteTime, QLocale::ShortFormat);
    constexpr QLatin1String timeDiffStr{"2 hours later"};

    launchQuery("time gmt-2");
    auto matches = manager->matches();
    std::sort(matches.begin(), matches.end(), [](const KRunner::QueryMatch &a, const KRunner::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QCOMPARE(manager->matches().count(), 1);
    QVERIFY2(manager->matches().first().text().contains(timeStr),
             QStringLiteral("first match: %1 timeStr: %2").arg(manager->matches().first().text(), timeStr).toUtf8().constData());
    QVERIFY(manager->matches().first().text().contains(timeDiffStr));
}

void DateTimeRunnerTest::testFindTimezones()
{
    QFETCH(QString, searchTerm);
    QFETCH(int, minMatchCount);
    QFETCH(QString, expectedTimezone);

    launchQuery("time " + searchTerm);
    auto matches = manager->matches();
    // Skip Daylight Saving Time for now, as it causes troubles in the test
    matches.erase(std::remove_if(matches.begin(),
                                 matches.end(),
                                 [](const QueryMatch &match) {
                                     return match.text().contains(QLatin1String("PST8PDT"));
                                 }),
                  matches.end());
    std::sort(matches.begin(), matches.end(), [](const KRunner::QueryMatch &a, const KRunner::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QVERIFY2(matches.size() >= minMatchCount,
             QStringLiteral("searchTerm: %1, matches.size(): %2, minMatchCount: %3")
                 .arg(searchTerm, QString::number(matches.size()), QString::number(minMatchCount))
                 .toUtf8()
                 .constData());
    QVERIFY2(matches.first().text().contains(expectedTimezone),
             QStringLiteral("first match: %1, expectedTimezone: %2").arg(matches.first().text(), expectedTimezone).toUtf8().constData());
}

void DateTimeRunnerTest::testFindTimezones_data()
{
    QTest::addColumn<QString>("searchTerm");
    QTest::addColumn<int>("minMatchCount");
    QTest::addColumn<QString>("expectedTimezone");

    QTest::newRow("Should find time zones by city name") << "Harare" << 1 << "Harare";
    QTest::newRow("Should find time zones by long name") << "Central Africa Time" << 1 << "Central Africa Time";
    QTest::newRow("Should find time zones by short name") << "GMT+2" << 1 << "GMT+2";
    QTest::newRow("Should find time zones by offset name") << "UTC+02:00" << 1 << "UTC+02:00";
    QTest::newRow("Should find time zones by abbreviation, and show all time zones with that abbreviation") << "PST" << 1 << "(PST)";
    QTest::newRow("Should find time zones by country name, and show all time zones in that country") << "Brazil" << 2 << "Brazil - ";
    QTest::newRow("Should find time zones with the 'in' keyword") << "in Harare" << 1 << "Harare";
}

void DateTimeRunnerTest::testConversion()
{
#if QT_VERSION < QT_VERSION_CHECK(6, 6, 0)
    QSKIP("CLDR 42 replaced plain ASCII space U+0020 with U+202F when formatting dates. It's fixed in Qt 6.6.");
#endif
    QFETCH(QString, query);
    QFETCH(QString, expectedSourceTimezone);
    QFETCH(QString, expectedSourceTime);
    QFETCH(QString, expectedTargetTimezone);
    QFETCH(QString, expectedTargetTime);
    QFETCH(QString, expectedTimeDiff);

    launchQuery(query);
    auto matches = manager->matches();
    std::sort(matches.begin(), matches.end(), [](const KRunner::QueryMatch &a, const KRunner::QueryMatch &b) {
        return a.relevance() > b.relevance();
    });

    QVERIFY(!matches.isEmpty());
    qDebug() << matches.first().text();
    const QString targetText = u"%1: %2"_s.arg(expectedTargetTimezone, expectedTargetTime);
    QVERIFY2(matches.first().text().contains(targetText), qUtf8Printable(targetText));
    const QString sourceText = u"%1: %2"_s.arg(expectedSourceTimezone, expectedSourceTime);
    QVERIFY2(matches.first().text().contains(sourceText), qUtf8Printable(sourceText));
    QVERIFY2(matches.first().text().contains(expectedTimeDiff), qUtf8Printable(expectedTimeDiff));
}

void DateTimeRunnerTest::testConversion_data()
{
    const QTimeZone systemTimeZone = QTimeZone::systemTimeZone().isValid() ? QTimeZone::systemTimeZone() : QTimeZone::utc(); // needed for FreeBSD CI
    const QString systemTimeZoneName = systemTimeZone.abbreviation(QDateTime::currentDateTime());

    QTest::addColumn<QString>("query");
    QTest::addColumn<QString>("expectedSourceTimezone");
    QTest::addColumn<QString>("expectedSourceTime");
    QTest::addColumn<QString>("expectedTargetTimezone");
    QTest::addColumn<QString>("expectedTargetTime");
    QTest::addColumn<QString>("expectedTimeDiff");

    QTest::newRow("Should convert from first to second time zone") << "12:00 PM gmt-2 in greenwich"
                                                                   << "GMT-2" << u"12:00 PM"_s << "Greenwich" << u"10:00 AM"_s << "2 hours earlier";
    QTest::newRow("Should convert from first to second time zone with date") << "1/5/23 12:00 PM gmt-2 in greenwich"
                                                                             << "GMT-2" << u"12:00 PM"_s << "Greenwich" << u"10:00 AM"_s << "2 hours earlier";
    QTest::newRow("Should convert from system time zone to second time zone")
        << "12:00 PM in gmt+2" << systemTimeZoneName << u"12:00 PM"_s << "GMT+2" << u"2:00 PM"_s << "2 hours later";
    QTest::newRow("Should convert from first time zone to system time zone")
        << "12:00 PM gmt+2"
        << "GMT+2" << u"12:00 PM"_s << systemTimeZoneName << u"10:00 AM"_s << "2 hours earlier";
}

QTEST_MAIN(DateTimeRunnerTest)

#include "datetimerunnertest.moc"
