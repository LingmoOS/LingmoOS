/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "advancedqueryparser.h"

#include <QTest>

Q_DECLARE_METATYPE(Baloo::Term)

using Term = Baloo::Term;
using AdvancedQueryParser = Baloo::AdvancedQueryParser;

class AdvancedQueryParserTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testSimpleProperty();
    void testSimpleString();
    void testStringAndProperty();
    void testLogicalOps();
    void testNesting();
    void testDateTime();
    void testOperators();
    void testBinaryOperatorMissingFirstArg();
    void testNestedParentheses();
    void testNestedParentheses_data();
    void testOptimizedLogic();
    void testOptimizedLogic_data();
    void testPhrases();
    void testPhrases_data();
    void testIncompleteTokens();
    void testIncompleteTokens_data();
    void testQuoting();
    void testQuoting_data();
    void testSpecialCharacters();
    void testSpecialCharacters_data();
};

void AdvancedQueryParserTest::testSimpleProperty()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("artist:Coldplay"));
    Term expectedTerm(QStringLiteral("artist"), QStringLiteral("Coldplay"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testSimpleString()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("Coldplay"));
    Term expectedTerm(QString(), QStringLiteral("Coldplay"));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testStringAndProperty()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("stars artist:Coldplay fire"));
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("stars")));
    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")));
    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("fire")));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testLogicalOps()
{
    // AND
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("artist:Coldplay AND type:song"));
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), QStringLiteral("song")));

    QCOMPARE(term, expectedTerm);

    // OR
    term = parser.parse(QStringLiteral("artist:Coldplay OR type:song"));
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), QStringLiteral("song")));

    QCOMPARE(term, expectedTerm);

    // AND then OR
    term = parser.parse(QStringLiteral("artist:Coldplay AND type:song OR stars"));
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")) && Term(QStringLiteral("type"), QStringLiteral("song")));
    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("stars")));

    QCOMPARE(term, expectedTerm);

    // OR then AND
    term = parser.parse(QStringLiteral("artist:Coldplay OR type:song AND stars"));
    expectedTerm = Term(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")) || Term(QStringLiteral("type"), QStringLiteral("song")));
    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("stars")));

    QCOMPARE(term, expectedTerm);

    // Multiple ANDs
    term = parser.parse(QStringLiteral("artist:Coldplay AND type:song AND stars"));
    expectedTerm = Term(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), QStringLiteral("song")));
    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("stars")));

    QCOMPARE(term, expectedTerm);

    // Multiple ORs
    term = parser.parse(QStringLiteral("artist:Coldplay OR type:song OR stars"));
    expectedTerm = Term(Term::Or);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), QStringLiteral("song")));
    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("stars")));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testNesting()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("artist:Coldplay AND (type:song OR stars) fire"));
    Term expectedTerm(Term::And);

    expectedTerm.addSubTerm(Term(QStringLiteral("artist"), QStringLiteral("Coldplay")));
    expectedTerm.addSubTerm(Term(QStringLiteral("type"), QStringLiteral("song")) || Term(QString(), QStringLiteral("stars")));
    expectedTerm.addSubTerm(Term(QString(), QStringLiteral("fire")));

    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testDateTime()
{
    // Integers
    AdvancedQueryParser parser;
    Term term;
    Term expectedTerm;

    term = parser.parse(QStringLiteral("modified:2014-12-02"));
    expectedTerm = Term(QStringLiteral("modified"), QStringLiteral("2014-12-02"));
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("modified:\"2014-12-02T23:22:1\""));
    expectedTerm = Term(QStringLiteral("modified"), QStringLiteral("2014-12-02T23:22:1"));
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testOperators()
{
    AdvancedQueryParser parser;
    Term term;
    Term expectedTerm;

    term = parser.parse(QStringLiteral("width:500"));
    expectedTerm = Term(QStringLiteral("width"), QStringLiteral("500"), Term::Contains);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width=500"));
    expectedTerm = Term(QStringLiteral("width"), QStringLiteral("500"), Term::Equal);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width<500"));
    expectedTerm = Term(QStringLiteral("width"), QStringLiteral("500"), Term::Less);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width<=500"));
    expectedTerm = Term(QStringLiteral("width"), QStringLiteral("500"), Term::LessEqual);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width>500"));
    expectedTerm = Term(QStringLiteral("width"), QStringLiteral("500"), Term::Greater);
    QCOMPARE(term, expectedTerm);

    term = parser.parse(QStringLiteral("width>=500"));
    expectedTerm = Term(QStringLiteral("width"), QStringLiteral("500"), Term::GreaterEqual);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testBinaryOperatorMissingFirstArg()
{
    AdvancedQueryParser parser;
    Term term = parser.parse(QStringLiteral("=:2"));
    Term expectedTerm;
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testNestedParentheses()
{
    QFETCH(QString, searchInput);
    QFETCH(QString, failmessage);
    QFETCH(Term, expectedTerm);

    AdvancedQueryParser parser;
    const auto testTerm = parser.parse(searchInput);
    qDebug() << "  result term" << testTerm;
    qDebug() << "expected term" << expectedTerm;
    if (!failmessage.isEmpty()) {
        QEXPECT_FAIL("", qPrintable(failmessage), Continue);
    }

    QCOMPARE(testTerm, expectedTerm);
}

void AdvancedQueryParserTest::testNestedParentheses_data()
{
    QTest::addColumn<QString>("searchInput");
    QTest::addColumn<Term>("expectedTerm");
    QTest::addColumn<QString>("failmessage");

    QTest::newRow("a AND b AND c AND d")
        << QStringLiteral("a AND b AND c AND d")
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{QString(), QStringLiteral("c"), Term::Contains},
            Term{QString(), QStringLiteral("d"), Term::Contains},
        }}
        << QString()
        ;
    QTest::newRow("(a AND b) AND (c OR d)")
        << QStringLiteral("(a AND b) AND (c OR d)")
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{Term::Or, QList<Term>{
                Term{QString(), QStringLiteral("c"), Term::Contains},
                Term{QString(), QStringLiteral("d"), Term::Contains},
            }}
        }}
        << QString()
        ;
    QTest::newRow("(a AND (b AND (c AND d)))")
        << QStringLiteral("(a AND (b AND (c AND d)))")
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{QString(), QStringLiteral("c"), Term::Contains},
            Term{QString(), QStringLiteral("d"), Term::Contains},
        }}
        << QString()
        ;
    // Test 1 for BUG: 392620
    QTest::newRow("a OR ((b AND c) AND d)")
        << QStringLiteral("a OR ((b AND c) AND d)")
        << Term{Term::Or, QList<Term>{
                Term{QString(), QStringLiteral("a"), Term::Contains},
                Term{Term::And, QList<Term>{
                    Term{QString(), QStringLiteral("b"), Term::Contains},
                    Term{QString(), QStringLiteral("c"), Term::Contains},
                    Term{QString(), QStringLiteral("d"), Term::Contains}
                }}
            }}
        << QString()
        ;
    // Test 2 for BUG: 392620
    QTest::newRow("a AND ((b OR c) OR d)")
        << QStringLiteral("a AND ((b OR c) OR d)")
        << Term{Term::And, QList<Term>{
                Term{QString(), QStringLiteral("a"), Term::Contains},
                Term{Term::Or, QList<Term>{
                    Term{QString(), QStringLiteral("b"), Term::Contains},
                    Term{QString(), QStringLiteral("c"), Term::Contains},
                    Term{QString(), QStringLiteral("d"), Term::Contains}
                }}
            }}
        << QString();
        ;
}

void AdvancedQueryParserTest::testOptimizedLogic()
{
    QFETCH(Term, testTerm);
    QFETCH(Term, expectedTerm);
    qDebug() << "  result term" << testTerm;
    qDebug() << "expected term" << expectedTerm;

    QCOMPARE(testTerm, expectedTerm);
}

void AdvancedQueryParserTest::testOptimizedLogic_data()
{
    QTest::addColumn<Term>("testTerm");
    QTest::addColumn<Term>("expectedTerm");

    // a && b && c && d can be combined into one AND term with 4 subterms
    QTest::addRow("a && b && c && d")
        << (Term{QString(), QStringLiteral("a"), Term::Contains}
            && Term{QString(), QStringLiteral("b"), Term::Contains}
            && Term{QString(), QStringLiteral("c"), Term::Contains}
            && Term{QString(), QStringLiteral("d"), Term::Contains})
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{QString(), QStringLiteral("c"), Term::Contains},
            Term{QString(), QStringLiteral("d"), Term::Contains},
        }}
    ;

    // (a AND b) AND (c OR d) can be merged as (a AND b AND (c OR D)
    QTest::addRow("(a && b) && (c || d)")
        << ((Term{QString(), QStringLiteral("a"), Term::Contains}
                && Term{QString(), QStringLiteral("b"), Term::Contains})
            && (Term{QString(), QStringLiteral("c"), Term::Contains}
                || Term{QString(), QStringLiteral("d"), Term::Contains}
            ))
        << Term{Term::And, QList<Term>{
            Term{QString(), QStringLiteral("a"), Term::Contains},
            Term{QString(), QStringLiteral("b"), Term::Contains},
            Term{Term::Or, QList<Term>{
                Term{QString(), QStringLiteral("c"), Term::Contains},
                Term{QString(), QStringLiteral("d"), Term::Contains}
            }}
        }}
    ;
}

void AdvancedQueryParserTest::testPhrases()
{
    QFETCH(QString, input);
    QFETCH(Term, expectedTerm);

    AdvancedQueryParser parser;
    Term term = parser.parse(input);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testPhrases_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Term>("expectedTerm");

    auto addRow = [](const QString& input, const Term& term)
	{ QTest::addRow("%s", qPrintable(input)) << input << term; };

    addRow(QStringLiteral("artist:ColdPlay"),         {QStringLiteral("artist"), QStringLiteral("ColdPlay"), Term::Contains});
    addRow(QStringLiteral("artist:\"ColdPlay\""),     {QStringLiteral("artist"), QStringLiteral("ColdPlay"), Term::Contains});
    addRow(QStringLiteral("artist:\"Foo Fighters\""), {QStringLiteral("artist"), QStringLiteral("Foo Fighters"), Term::Contains});
    addRow(QStringLiteral("artist:\"Foo Fighters\" OR artist:ColdPlay "), {Term::Or, {
	{QStringLiteral("artist"), QStringLiteral("Foo Fighters"), Term::Contains},
	{QStringLiteral("artist"), QStringLiteral("ColdPlay"), Term::Contains},
    }});
}

void AdvancedQueryParserTest::testIncompleteTokens()
{
    QFETCH(QString, input);
    QFETCH(Term, expectedTerm);

    AdvancedQueryParser parser;
    Term term = parser.parse(input);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testIncompleteTokens_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Term>("expectedTerm");

    auto addRow = [](const QString& name, const QString& input, const Term& term)
	{ QTest::addRow("%s", qPrintable(name)) << input << term; };

    addRow(QStringLiteral("ends with quote"),          QStringLiteral("foo \""), {QString(), QStringLiteral("foo"), Term::Auto});
    addRow(QStringLiteral("ends with comparator"),     QStringLiteral("foo>"),   {QStringLiteral("foo"), QString(), Term::Contains});
    addRow(QStringLiteral("ends with opening parens"), QStringLiteral("foo ("),  {QString(), QStringLiteral("foo")});
    addRow(QStringLiteral("ends with closing parens"), QStringLiteral("foo )"),  {QString(), QStringLiteral("foo")});
}

void AdvancedQueryParserTest::testQuoting()
{
    QFETCH(QString, input);
    QFETCH(Term, expectedTerm);

    AdvancedQueryParser parser;
    Term term = parser.parse(input);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testQuoting_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Term>("expectedTerm");

    auto addRow = [](const QString& name, const QString& input, const Term& term)
	{ QTest::addRow("%s", qPrintable(name)) << input << term; };

    addRow(QStringLiteral("empty"),                    QStringLiteral("\"\""),   {QString(), QStringLiteral(""), Term::Auto});
    addRow(QStringLiteral("two quoted"),     QStringLiteral("\"foo\"\"bar\""),   {Term::And, {
	{QStringLiteral(""), QStringLiteral("foo"), Term::Contains},
	{QStringLiteral(""), QStringLiteral("bar"), Term::Contains},
    }});
    addRow(QStringLiteral("two quoted properties"),     QStringLiteral("artist:\"foo\" OR artist:\"bar\""),   {Term::Or, {
	{QStringLiteral("artist"), QStringLiteral("foo"), Term::Contains},
	{QStringLiteral("artist"), QStringLiteral("bar"), Term::Contains},
    }});
    addRow(QStringLiteral("two empty properties"),     QStringLiteral("artist:\"\" AND filename:\"\""),   {Term::And, {
	{QStringLiteral("artist"), QStringLiteral(""), Term::Contains},
	{QStringLiteral("filename"), QStringLiteral(""), Term::Contains},
    }});
    addRow(QStringLiteral("quoted and unquoted"), QStringLiteral("one \"two and three\" four"),   {Term::And, {
	{QStringLiteral(""), QStringLiteral("one"), Term::Contains},
	{QStringLiteral(""), QStringLiteral("two and three"), Term::Contains},
	{QStringLiteral(""), QStringLiteral("four"), Term::Contains},
    }});
    addRow(QStringLiteral("multiple quoted"), QStringLiteral("\"one and two\" \"three and four\""),   {Term::And, {
	{QStringLiteral(""), QStringLiteral("one and two"), Term::Contains},
	{QStringLiteral(""), QStringLiteral("three and four"), Term::Contains},
    }});
}

void AdvancedQueryParserTest::testSpecialCharacters()
{
    QFETCH(QString, input);
    QFETCH(Term, expectedTerm);

    AdvancedQueryParser parser;
    Term term = parser.parse(input);
    QCOMPARE(term, expectedTerm);
}

void AdvancedQueryParserTest::testSpecialCharacters_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<Term>("expectedTerm");

    auto addRow = [](const QString& name, const QString& input, const Term& term)
	{ QTest::addRow("%s", qPrintable(name)) << input << term; };

    addRow(QStringLiteral("mail"), QStringLiteral("foo@bar.com"),
        {QStringLiteral(""), QStringLiteral("foo@bar.com"), Term::Contains});
    addRow(QStringLiteral("mail and other"), QStringLiteral("one foo@bar.com two"), {Term::And, {
        {QStringLiteral(""), QStringLiteral("one"), Term::Contains},
        {QStringLiteral(""), QStringLiteral("foo@bar.com"), Term::Contains},
        {QStringLiteral(""), QStringLiteral("two"), Term::Contains},
    }});
    addRow(QStringLiteral("filename"), QStringLiteral("foo_bar.png"),
        {QStringLiteral(""), QStringLiteral("foo_bar.png"), Term::Contains});
    addRow(QStringLiteral("various phrases"), QStringLiteral("one_two \"foo@bar.com\" two.png"), {Term::And, {
        {QStringLiteral(""), QStringLiteral("one_two"), Term::Contains},
        {QStringLiteral(""), QStringLiteral("foo@bar.com"), Term::Contains},
        {QStringLiteral(""), QStringLiteral("two.png"), Term::Contains},
    }});
}

QTEST_MAIN(AdvancedQueryParserTest)

#include "advancedqueryparsertest.moc"
