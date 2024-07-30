/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2021 Waqar Ahmed <waqar.17a@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kfuzzymatchertest.h"

#include <QString>
#include <QStringList>
#include <QTest>

#include <algorithm>

#include "kfuzzymatcher.h"

QTEST_MAIN(KFuzzyMatcherTest)

void KFuzzyMatcherTest::testMatchSimple_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("inputstr");
    QTest::addColumn<bool>("expected");

    QTest::newRow("AbcD") << QStringLiteral("AbcD") << QStringLiteral("AbCdefg") << true;
    QTest::newRow("WithSpace") << QStringLiteral("Wa qa") << QStringLiteral("Wa qar") << true;
    QTest::newRow("RTL") << QStringLiteral("ارو") << QStringLiteral("اردو") << true;
    QTest::newRow("WithSep") << QStringLiteral("tf") << QStringLiteral("the_file") << true;
    QTest::newRow("Umlaut") << QStringLiteral("Häu") << QStringLiteral("Häuser") << true;
    QTest::newRow("Unmatched") << QStringLiteral("Name") << QStringLiteral("Nam") << false;
    QTest::newRow("Empty Pattern") << QStringLiteral("") << QStringLiteral("Nam") << true;
}

void KFuzzyMatcherTest::testMatchSimple()
{
    QFETCH(QString, pattern);
    QFETCH(QString, inputstr);
    QFETCH(bool, expected);

    QVERIFY(KFuzzyMatcher::matchSimple(pattern, inputstr) == expected);
}

void KFuzzyMatcherTest::testMatch_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QStringList>("input");
    QTest::addColumn<QStringList>("expected");
    QTest::addColumn<int>("size");
    // clang-format off
    QTest::newRow("pattern=sort") << QStringLiteral("sort")
                          << QStringList{
                                QStringLiteral("Sort"),
                                QStringLiteral("Some other right test"),
                                QStringLiteral("Soup rate"),
                                QStringLiteral("Someother"),
                                QStringLiteral("irrelevant"),
                              }
                          << QStringList{
                                QStringLiteral("Sort"),
                                QStringLiteral("Some other right test"),
                                QStringLiteral("Soup rate"),
                              }
                          << 3;


    QTest::newRow("pattern=kateapp") << QStringLiteral("kaapp")
                          << QStringList{
                                QStringLiteral("kateapp.cpp"),
                                QStringLiteral("kate_application"),
                                QStringLiteral("kateapp.h"),
                                QStringLiteral("katepap.c")
                              }
                          << QStringList{
                                QStringLiteral("kate_application"),
                                QStringLiteral("kateapp.h"),
                                QStringLiteral("kateapp.cpp")
                             }
                          << 3;

    QTest::newRow("pattern=this") << QStringLiteral("this")
                          << QStringList{
                                QStringLiteral("th"),
                                QStringLiteral("ths"),
                                QStringLiteral("thsi")
                              }
                          << QStringList{
                             }
                          << 0;

    QTest::newRow("pattern=marath") << QStringLiteral("marath")
                          << QStringList{
                             QStringLiteral("Maralen of the Mornsong"),
                             QStringLiteral("Silumgar, the Drifting Death"),
                             QStringLiteral("Maralen of the Mornsong Avatar"),
                             QStringLiteral("Marshaling the Troops"),
                             QStringLiteral("Homeward Path"),
                             QStringLiteral("Marath, Will of the Wild"),
                             QStringLiteral("Marshal's Anthem"),
                             QStringLiteral("Marchesa, the Black Rose"),
                             QStringLiteral("Mark for Death"),
                             QStringLiteral("Master Apothecary"),
                             QStringLiteral("Mazirek, Kraul Death Priest"),
                             QStringLiteral("Akroma, Angel of Wrath"),
                             QStringLiteral("Akroma, Angel of Wrath Avatar"),
                             QStringLiteral("Commander's Authority"),
                             QStringLiteral("Shaman of the Great Hunt"),
                             QStringLiteral("Halimar Wavewatch"),
                             QStringLiteral("Pyromancer's Swath")
                              }
                          << QStringList{
                             QStringLiteral("Marath, Will of the Wild"),
                             QStringLiteral("Maralen of the Mornsong"),
                             QStringLiteral("Maralen of the Mornsong Avatar"),
                             QStringLiteral("Marshal's Anthem"),
                             QStringLiteral("Marshaling the Troops"),
                             QStringLiteral("Marchesa, the Black Rose"),
                             QStringLiteral("Mark for Death"),
                             QStringLiteral("Master Apothecary"),
                             QStringLiteral("Mazirek, Kraul Death Priest"),
                             QStringLiteral("Akroma, Angel of Wrath"),
                             QStringLiteral("Akroma, Angel of Wrath Avatar"),
                             QStringLiteral("Commander's Authority"),
                             QStringLiteral("Homeward Path"),
                             QStringLiteral("Shaman of the Great Hunt"),
                             QStringLiteral("Halimar Wavewatch"),
                             QStringLiteral("Pyromancer's Swath"),
                             QStringLiteral("Silumgar, the Drifting Death")
                             }
                          << 17;

    // This tests our recursive best match
    QTest::newRow("pattern=lll") << QStringLiteral("lll")
                          << QStringList{
                                QStringLiteral("SVisualLoggerLogsList.h"),
                                QStringLiteral("SimpleFileLogger.cpp"),
                                QStringLiteral("StringHandlerLogList.txt"),
                                QStringLiteral("LeapFromLostAllan"),
                                QStringLiteral("BumpLLL"),
                              }
                          << QStringList{
                             QStringLiteral("SVisualLoggerLogsList.h"),
                             QStringLiteral("LeapFromLostAllan"),
                             QStringLiteral("BumpLLL"),
                             QStringLiteral("StringHandlerLogList.txt"),
                             QStringLiteral("SimpleFileLogger.cpp"),
                             }
                          << 5;

    QTest::newRow("pattern=") << QStringLiteral("")
                          << QStringList{
                                QStringLiteral("th"),
                                QStringLiteral("ths"),
                                QStringLiteral("thsi")
                              }
                          << QStringList{
                             QStringLiteral("th"),
                             QStringLiteral("ths"),
                             QStringLiteral("thsi")
                             }
                          << 3;
    // clang-format on
}

static QStringList matchHelper(const QString &pattern, const QStringList &input)
{
    QList<QPair<QString, int>> actual;
    for (int i = 0; i < input.size(); ++i) {
        KFuzzyMatcher::Result res = KFuzzyMatcher::match(pattern, input.at(i));
        if (res.matched) {
            actual.push_back({input.at(i), res.score});
        }
    }

    // sort descending based on score
    std::sort(actual.begin(), actual.end(), [](const QPair<QString, int> &l, const QPair<QString, int> &r) {
        return l.second > r.second;
    });

    QStringList actualOut;
    for (const auto &s : actual) {
        actualOut << s.first;
    }
    return actualOut;
}

void KFuzzyMatcherTest::testMatch()
{
    QFETCH(QString, pattern);
    QFETCH(QStringList, input);
    QFETCH(QStringList, expected);
    QFETCH(int, size);

    const QStringList actual = matchHelper(pattern, input);

    QCOMPARE(actual.size(), size);
    QCOMPARE(actual, expected);
}

void KFuzzyMatcherTest::testMatchedRanges_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("string");

    using Range = QPair<int, int>;
    QTest::addColumn<QList<Range>>("expectedRanges");

    QTest::addColumn<bool>("matchingOnly");

    QTest::newRow("Emtpy") << QStringLiteral("") << QStringLiteral("") << QList<Range>{} << true;
    QTest::newRow("Hello") << QStringLiteral("Hlo") << QStringLiteral("Hello") << QList<Range>{{0, 1}, {3, 2}} << true;
    QTest::newRow("lll") << QStringLiteral("lll") << QStringLiteral("SVisualLoggerLogsList") << QList<Range>{{7, 1}, {13, 1}, {17, 1}} << true;
    QTest::newRow("Sort") << QStringLiteral("sort") << QStringLiteral("SorT") << QList<Range>{{0, 4}} << true;
    QTest::newRow("Unmatching") << QStringLiteral("git") << QStringLiteral("gti") << QList<Range>{} << true;
    QTest::newRow("UnmatchingWithAllMatches") << QStringLiteral("git") << QStringLiteral("gti") << QList<Range>{{0, 1}, {2, 1}} << false;
}

void KFuzzyMatcherTest::testMatchedRanges()
{
    QFETCH(QString, pattern);
    QFETCH(QString, string);
    QFETCH(bool, matchingOnly);
    using Range = QPair<int, int>;
    QFETCH(QList<Range>, expectedRanges);

    const auto matchMode = matchingOnly ? KFuzzyMatcher::RangeType::FullyMatched : KFuzzyMatcher::RangeType::All;

    auto resultRanges = KFuzzyMatcher::matchedRanges(pattern, string, matchMode);
    QCOMPARE(resultRanges.size(), expectedRanges.size());

    bool res = std::equal(expectedRanges.begin(), expectedRanges.end(), resultRanges.begin(), [](const Range &l, const KFuzzyMatcher::Range &r) {
        return l.first == r.start && l.second == r.length;
    });
    QVERIFY(res);
}

#include "moc_kfuzzymatchertest.cpp"
