/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "regexpsearch_test.h"
#include "moc_regexpsearch_test.cpp"

#include <katedocument.h>
#include <kateregexpsearch.h>

#include <QRegularExpression>

#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(RegExpSearchTest)

#define testNewRow() (QTest::newRow(QStringLiteral("line %1").arg(__LINE__).toLatin1().data()))

using namespace KTextEditor;

RegExpSearchTest::RegExpSearchTest()
    : QObject()
{
    QStandardPaths::setTestModeEnabled(true);
}

RegExpSearchTest::~RegExpSearchTest()
{
}

void RegExpSearchTest::testReplaceEscapeSequences_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");

    testNewRow() << QStringLiteral("\\") << QStringLiteral("\\");
    testNewRow() << QStringLiteral("\\0") << QStringLiteral("0");
    testNewRow() << QStringLiteral("\\00") << QStringLiteral("00");
    testNewRow() << QStringLiteral("\\000") << QStringLiteral("000");
    testNewRow() << QStringLiteral("\\0000") << QString(QChar(0));
    testNewRow() << QStringLiteral("\\0377") << QString(QChar(0377));
    testNewRow() << QStringLiteral("\\0378") << QStringLiteral("0378");
    testNewRow() << QStringLiteral("\\a") << QStringLiteral("\a");
    testNewRow() << QStringLiteral("\\f") << QStringLiteral("\f");
    testNewRow() << QStringLiteral("\\n") << QStringLiteral("\n");
    testNewRow() << QStringLiteral("\\r") << QStringLiteral("\r");
    testNewRow() << QStringLiteral("\\t") << QStringLiteral("\t");
    testNewRow() << QStringLiteral("\\v") << QStringLiteral("\v");
    testNewRow() << QStringLiteral("\\x") << QStringLiteral("x");
    testNewRow() << QStringLiteral("\\x0") << QStringLiteral("x0");
    testNewRow() << QStringLiteral("\\x00") << QStringLiteral("x00");
    testNewRow() << QStringLiteral("\\x000") << QStringLiteral("x000");
    testNewRow() << QStringLiteral("\\x0000") << QString(QChar(0x0000));
    testNewRow() << QStringLiteral("\\x00000") << QString(QChar(0x0000) + QLatin1Char('0'));
    testNewRow() << QStringLiteral("\\xaaaa") << QString(QChar(0xaaaa));
    testNewRow() << QStringLiteral("\\xFFFF") << QString(QChar(0xFFFF));
    testNewRow() << QStringLiteral("\\xFFFg") << QStringLiteral("xFFFg");
}

void RegExpSearchTest::testReplaceEscapeSequences()
{
    QFETCH(QString, pattern);
    QFETCH(QString, expected);

    const QString result1 = KateRegExpSearch::escapePlaintext(pattern);
    const QString result2 = KateRegExpSearch::buildReplacement(pattern, QStringList(), 0);

    QCOMPARE(result1, expected);
    QCOMPARE(result2, expected);
}

void RegExpSearchTest::testReplacementReferences_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");
    QTest::addColumn<QStringList>("capturedTexts");

    testNewRow() << QStringLiteral("\\0") << QStringLiteral("b") << (QStringList() << QStringLiteral("b"));
    testNewRow() << QStringLiteral("\\00") << QStringLiteral("b0") << (QStringList() << QStringLiteral("b"));
    testNewRow() << QStringLiteral("\\000") << QStringLiteral("b00") << (QStringList() << QStringLiteral("b"));
    testNewRow() << QStringLiteral("\\0000") << QString(QChar(0)) << (QStringList() << QStringLiteral("b"));
    testNewRow() << QStringLiteral("\\1") << QStringLiteral("1") << (QStringList() << QStringLiteral("b"));
    testNewRow() << QStringLiteral("\\0") << QStringLiteral("b") << (QStringList() << QStringLiteral("b") << QStringLiteral("c"));
    testNewRow() << QStringLiteral("\\1") << QStringLiteral("c") << (QStringList() << QStringLiteral("b") << QStringLiteral("c"));
}

void RegExpSearchTest::testReplacementReferences()
{
    QFETCH(QString, pattern);
    QFETCH(QString, expected);
    QFETCH(QStringList, capturedTexts);

    const QString result = KateRegExpSearch::buildReplacement(pattern, capturedTexts, 1);

    QCOMPARE(result, expected);
}

void RegExpSearchTest::testReplacementCaseConversion_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<QString>("expected");

    testNewRow() << QStringLiteral("a\\Uaa") << QStringLiteral("aAA");
    testNewRow() << QStringLiteral("a\\UAa") << QStringLiteral("aAA");
    testNewRow() << QStringLiteral("a\\UaA") << QStringLiteral("aAA");

    testNewRow() << QStringLiteral("a\\Uáa") << QStringLiteral("aÁA");
    testNewRow() << QStringLiteral("a\\UAá") << QStringLiteral("aAÁ");
    testNewRow() << QStringLiteral("a\\UaÁ") << QStringLiteral("aAÁ");

    testNewRow() << QStringLiteral("a\\uaa") << QStringLiteral("aAa");
    testNewRow() << QStringLiteral("a\\uAa") << QStringLiteral("aAa");
    testNewRow() << QStringLiteral("a\\uaA") << QStringLiteral("aAA");

    testNewRow() << QStringLiteral("a\\uáa") << QStringLiteral("aÁa");
    testNewRow() << QStringLiteral("a\\uÁa") << QStringLiteral("aÁa");
    testNewRow() << QStringLiteral("a\\uáA") << QStringLiteral("aÁA");

    testNewRow() << QStringLiteral("A\\LAA") << QStringLiteral("Aaa");
    testNewRow() << QStringLiteral("A\\LaA") << QStringLiteral("Aaa");
    testNewRow() << QStringLiteral("A\\LAa") << QStringLiteral("Aaa");

    testNewRow() << QStringLiteral("A\\LÁA") << QStringLiteral("Aáa");
    testNewRow() << QStringLiteral("A\\LaÁ") << QStringLiteral("Aaá");
    testNewRow() << QStringLiteral("A\\LÁa") << QStringLiteral("Aáa");

    testNewRow() << QStringLiteral("A\\lAA") << QStringLiteral("AaA");
    testNewRow() << QStringLiteral("A\\lAa") << QStringLiteral("Aaa");
    testNewRow() << QStringLiteral("A\\laA") << QStringLiteral("AaA");

    testNewRow() << QStringLiteral("A\\lÁA") << QStringLiteral("AáA");
    testNewRow() << QStringLiteral("A\\lÁa") << QStringLiteral("Aáa");
    testNewRow() << QStringLiteral("A\\láA") << QStringLiteral("AáA");

    testNewRow() << QStringLiteral("a\\Ubb\\EaA") << QStringLiteral("aBBaA");
    testNewRow() << QStringLiteral("A\\LBB\\EAa") << QStringLiteral("AbbAa");

    testNewRow() << QStringLiteral("a\\Ubb\\EáA") << QStringLiteral("aBBáA");
    testNewRow() << QStringLiteral("A\\LBB\\EÁa") << QStringLiteral("AbbÁa");
}

void RegExpSearchTest::testReplacementCaseConversion()
{
    QFETCH(QString, pattern);
    QFETCH(QString, expected);

    const QString result = KateRegExpSearch::buildReplacement(pattern, QStringList(), 1);

    QCOMPARE(result, expected);
}

void RegExpSearchTest::testReplacementCounter_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<int>("counter");
    QTest::addColumn<QString>("expected");

    testNewRow() << QStringLiteral("a\\#b") << 1 << QStringLiteral("a1b");
    testNewRow() << QStringLiteral("a\\#b") << 10 << QStringLiteral("a10b");
    testNewRow() << QStringLiteral("a\\#####b") << 1 << QStringLiteral("a00001b");
}

void RegExpSearchTest::testReplacementCounter()
{
    QFETCH(QString, pattern);
    QFETCH(int, counter);
    QFETCH(QString, expected);

    const QString result = KateRegExpSearch::buildReplacement(pattern, QStringList(), counter);

    QCOMPARE(result, expected);
}

void RegExpSearchTest::testAnchoredRegexp_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<Range>("inputRange");
    QTest::addColumn<bool>("backwards");
    QTest::addColumn<Range>("expected");

    testNewRow() << QStringLiteral("fe") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("fe") << Range(0, 0, 0, 8) << true << Range(0, 6, 0, 8);

    testNewRow() << QStringLiteral("^fe") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("^fe") << Range(0, 0, 0, 1) << false << Range::invalid();
    testNewRow() << QStringLiteral("^fe") << Range(0, 0, 0, 2) << false << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("^fe") << Range(0, 3, 0, 8) << false << Range::invalid(); // only match at line start
    testNewRow() << QStringLiteral("^fe") << Range(0, 0, 0, 2) << true << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("^fe") << Range(0, 0, 0, 1) << true << Range::invalid();
    testNewRow() << QStringLiteral("^fe") << Range(0, 0, 0, 2) << true << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("^fe") << Range(0, 3, 0, 8) << true << Range::invalid();

    testNewRow() << QStringLiteral("fe$") << Range(0, 0, 0, 8) << false << Range(0, 6, 0, 8);
    testNewRow() << QStringLiteral("fe$") << Range(0, 7, 0, 8) << false << Range::invalid();
    testNewRow() << QStringLiteral("fe$") << Range(0, 6, 0, 8) << false << Range(0, 6, 0, 8);
    testNewRow() << QStringLiteral("fe$") << Range(0, 0, 0, 5) << false << Range::invalid(); // only match at line end, fails
    testNewRow() << QStringLiteral("fe$") << Range(0, 0, 0, 8) << true << Range(0, 6, 0, 8);
    testNewRow() << QStringLiteral("fe$") << Range(0, 7, 0, 8) << true << Range::invalid();
    testNewRow() << QStringLiteral("fe$") << Range(0, 6, 0, 8) << true << Range(0, 6, 0, 8);
    testNewRow() << QStringLiteral("fe$") << Range(0, 0, 0, 5) << true << Range::invalid();

    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 3, 0, 8) << false << Range::invalid();
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 0, 0, 5) << false << Range::invalid();
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 3, 0, 5) << false << Range::invalid();
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 0, 0, 8) << true << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 3, 0, 8) << true << Range::invalid();
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 0, 0, 5) << true << Range::invalid();
    testNewRow() << QStringLiteral("^fe fe fe$") << Range(0, 3, 0, 5) << true << Range::invalid();

    testNewRow() << QStringLiteral("^fe( fe)*$") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("^fe( fe)*") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("fe( fe)*$") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("fe( fe)*") << Range(0, 0, 0, 8) << false << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("^fe( fe)*$") << Range(0, 3, 0, 8) << false << Range::invalid();
    testNewRow() << QStringLiteral("fe( fe)*$") << Range(0, 3, 0, 8) << false << Range(0, 3, 0, 8);
    testNewRow() << QStringLiteral("^fe( fe)*$") << Range(0, 0, 0, 5) << false << Range::invalid();
    // fails because the whole line is fed to QRegularExpression, then matches
    // that end beyond the search range are rejected, see KateRegExpSearch::searchText()
    // testNewRow() << "^fe( fe)*"  << Range(0, 0, 0, 5) << false << Range(0, 0, 0, 5);

    testNewRow() << QStringLiteral("^fe( fe)*$") << Range(0, 0, 0, 8) << true << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("^fe( fe)*") << Range(0, 0, 0, 8) << true << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("fe( fe)*$") << Range(0, 0, 0, 8) << true << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("fe( fe)*") << Range(0, 0, 0, 8) << true << Range(0, 0, 0, 8);
    testNewRow() << QStringLiteral("^fe( fe)*$") << Range(0, 3, 0, 8) << true << Range::invalid();
    testNewRow() << QStringLiteral("fe( fe)*$") << Range(0, 3, 0, 8) << true << Range(0, 3, 0, 8);
    testNewRow() << QStringLiteral("^fe( fe)*$") << Range(0, 0, 0, 5) << true << Range::invalid();

    testNewRow() << QStringLiteral("^fe|fe$") << Range(0, 0, 0, 5) << false << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("^fe|fe$") << Range(0, 3, 0, 8) << false << Range(0, 6, 0, 8);
    testNewRow() << QStringLiteral("^fe|fe$") << Range(0, 0, 0, 5) << true << Range(0, 0, 0, 2);
    testNewRow() << QStringLiteral("^fe|fe$") << Range(0, 3, 0, 8) << true << Range(0, 6, 0, 8);
}

void RegExpSearchTest::testAnchoredRegexp()
{
    QFETCH(QString, pattern);
    QFETCH(Range, inputRange);
    QFETCH(bool, backwards);
    QFETCH(Range, expected);

    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("fe fe fe"));

    KateRegExpSearch searcher(&doc);

    static int i = 0;
    if (i == 34 || i == 36) {
        qDebug() << i;
    }
    i++;

    const Range result = searcher.search(pattern, inputRange, backwards, QRegularExpression::CaseInsensitiveOption)[0];

    QCOMPARE(result, expected);
}

void RegExpSearchTest::testSearchForward()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("  \\piinfercong"));

    KateRegExpSearch searcher(&doc);
    QList<KTextEditor::Range> result = searcher.search(QStringLiteral("\\\\piinfer(\\w)"), Range(0, 2, 0, 15), false);

    QCOMPARE(result.at(0), Range(0, 2, 0, 11));
    QCOMPARE(doc.text(result.at(1)), QLatin1Char('c'));

    // Test Unicode
    doc.setText(QStringLiteral("  \\piinferćong"));
    result = searcher.search(QStringLiteral("\\\\piinfer(\\w)"), Range(0, 2, 0, 15), false);

    QCOMPARE(result.at(0), Range(0, 2, 0, 11));
    QCOMPARE(doc.text(result.at(1)), QStringLiteral("ć"));
}

void RegExpSearchTest::testSearchBackwardInSelection()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("foobar foo bar foo bar foo"));

    KateRegExpSearch searcher(&doc);
    const Range result = searcher.search(QStringLiteral("foo"), Range(0, 0, 0, 15), true)[0];

    QCOMPARE(result, Range(0, 7, 0, 10));
}

void RegExpSearchTest::test()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("\\newcommand{\\piReductionOut}"));

    KateRegExpSearch searcher(&doc);
    const QList<Range> result = searcher.search(QStringLiteral("\\\\piReduction(\\S)"), Range(0, 10, 0, 28), true);

    QCOMPARE(result.size(), 2);
    QCOMPARE(result[0], Range(0, 12, 0, 25));
    QCOMPARE(result[1], Range(0, 24, 0, 25));
    QCOMPARE(doc.text(result[0]), QStringLiteral("\\piReductionO"));
    QCOMPARE(doc.text(result[1]), QStringLiteral("O"));
}

void RegExpSearchTest::testUnicode()
{
    KTextEditor::DocumentPrivate doc;
    doc.setText(QStringLiteral("\\newcommand{\\piReductionOÓut}"));

    KateRegExpSearch searcher(&doc);
    const QList<Range> result = searcher.search(QStringLiteral("\\\\piReduction(\\w)(\\w)"), Range(0, 10, 0, 28), true);

    QCOMPARE(result.size(), 3);
    QCOMPARE(result.at(0), Range(0, 12, 0, 26));
    QCOMPARE(result.at(1), Range(0, 24, 0, 25));
    QCOMPARE(result.at(2), Range(0, 25, 0, 26));
    QCOMPARE(doc.text(result.at(0)), QStringLiteral("\\piReductionOÓ"));
    QCOMPARE(doc.text(result.at(1)), QStringLiteral("O"));
    QCOMPARE(doc.text(result.at(2)), QStringLiteral("Ó"));
}
