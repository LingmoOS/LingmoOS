/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2010 Bernhard Beschow <bbeschow@cs.tu-berlin.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plaintextsearch_test.h"
#include "moc_plaintextsearch_test.cpp"

#include <katedocument.h>
#include <kateplaintextsearch.h>

#include <QStandardPaths>
#include <QTest>

QTEST_MAIN(PlainTextSearchTest)

QtMessageHandler PlainTextSearchTest::s_msgHandler = nullptr;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    switch (type) {
    case QtDebugMsg:
        /* do nothing */
        break;
    default:
        PlainTextSearchTest::s_msgHandler(type, context, msg);
    }
}

void PlainTextSearchTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    s_msgHandler = qInstallMessageHandler(myMessageOutput);
}

void PlainTextSearchTest::cleanupTestCase()
{
    qInstallMessageHandler(nullptr);
}

PlainTextSearchTest::PlainTextSearchTest()
    : QObject()
{
}

PlainTextSearchTest::~PlainTextSearchTest()
{
}

void PlainTextSearchTest::init()
{
    m_doc = new KTextEditor::DocumentPrivate(false, false, nullptr, this);
    m_search = new KatePlainTextSearch(m_doc, Qt::CaseSensitive, false);
}

void PlainTextSearchTest::cleanup()
{
    delete m_search;
    delete m_doc;
}

void PlainTextSearchTest::testSearchBackward_data()
{
    QTest::addColumn<KTextEditor::Range>("searchRange");
    QTest::addColumn<KTextEditor::Range>("expectedResult");

    QTest::newRow("") << KTextEditor::Range(0, 0, 1, 10) << KTextEditor::Range(1, 6, 1, 10);
    QTest::newRow("") << KTextEditor::Range(0, 0, 1, 5) << KTextEditor::Range(1, 0, 1, 4);
    QTest::newRow("") << KTextEditor::Range(0, 0, 1, 0) << KTextEditor::Range(0, 10, 0, 14);
}

void PlainTextSearchTest::testSearchBackward()
{
    QFETCH(KTextEditor::Range, searchRange);
    QFETCH(KTextEditor::Range, expectedResult);

    m_doc->setText(
        QStringLiteral("aaaa aaaa aaaa\n"
                       "aaaa  aaaa"));

    QCOMPARE(m_search->search(QLatin1String("aaaa"), searchRange, true), expectedResult);
}

void PlainTextSearchTest::testSingleLineDocument_data()
{
    QTest::addColumn<KTextEditor::Range>("searchRange");
    QTest::addColumn<KTextEditor::Range>("forwardResult");
    QTest::addColumn<KTextEditor::Range>("backwardResult");

    QTest::newRow("[a a a a a a a a a a a a]") << KTextEditor::Range(0, 0, 0, 23) << KTextEditor::Range(0, 0, 0, 5) << KTextEditor::Range(0, 18, 0, 23);
    QTest::newRow("[a a a a a a a a a a a ]a") << KTextEditor::Range(0, 0, 0, 22) << KTextEditor::Range(0, 0, 0, 5) << KTextEditor::Range(0, 16, 0, 21);
    QTest::newRow("a[ a a a a a a a a a a a]") << KTextEditor::Range(0, 1, 0, 23) << KTextEditor::Range(0, 2, 0, 7) << KTextEditor::Range(0, 18, 0, 23);
    QTest::newRow("a[ a a a a a a a a a a ]a") << KTextEditor::Range(0, 1, 0, 22) << KTextEditor::Range(0, 2, 0, 7) << KTextEditor::Range(0, 16, 0, 21);
    QTest::newRow("[a a a a] a a a a a a a a") << KTextEditor::Range(0, 0, 0, 7) << KTextEditor::Range(0, 0, 0, 5) << KTextEditor::Range(0, 2, 0, 7);
    QTest::newRow("[a a a ]a a a a a a a a a") << KTextEditor::Range(0, 0, 0, 6) << KTextEditor::Range(0, 0, 0, 5) << KTextEditor::Range(0, 0, 0, 5);
    QTest::newRow("[a a a] a a a a a a a a a") << KTextEditor::Range(0, 0, 0, 5) << KTextEditor::Range(0, 0, 0, 5) << KTextEditor::Range(0, 0, 0, 5);
    QTest::newRow("[a a ]a a a a a a a a a a") << KTextEditor::Range(0, 0, 0, 4) << KTextEditor::Range::invalid() << KTextEditor::Range::invalid();
    QTest::newRow("a a a a a a a a [a a a a]") << KTextEditor::Range(0, 16, 0, 23) << KTextEditor::Range(0, 16, 0, 21) << KTextEditor::Range(0, 18, 0, 23);
    QTest::newRow("a a a a a a a a a[ a a a]") << KTextEditor::Range(0, 17, 0, 23) << KTextEditor::Range(0, 18, 0, 23) << KTextEditor::Range(0, 18, 0, 23);
    QTest::newRow("a a a a a a a a a [a a a]") << KTextEditor::Range(0, 18, 0, 23) << KTextEditor::Range(0, 18, 0, 23) << KTextEditor::Range(0, 18, 0, 23);
    QTest::newRow("a a a a a a a a a a[ a a]") << KTextEditor::Range(0, 19, 0, 23) << KTextEditor::Range::invalid() << KTextEditor::Range::invalid();
    QTest::newRow("a a a a a[ a a a a] a a a") << KTextEditor::Range(0, 9, 0, 17) << KTextEditor::Range(0, 10, 0, 15) << KTextEditor::Range(0, 12, 0, 17);
    QTest::newRow("a a a a a[ a a] a a a a a") << KTextEditor::Range(0, 9, 0, 13) << KTextEditor::Range::invalid() << KTextEditor::Range::invalid();
}

void PlainTextSearchTest::testSingleLineDocument()
{
    QFETCH(KTextEditor::Range, searchRange);
    QFETCH(KTextEditor::Range, forwardResult);
    QFETCH(KTextEditor::Range, backwardResult);

    m_doc->setText(QStringLiteral("a a a a a a a a a a a a"));

    QCOMPARE(m_search->search(QLatin1String("a a a"), searchRange, false), forwardResult);
    QCOMPARE(m_search->search(QLatin1String("a a a"), searchRange, true), backwardResult);
}

void PlainTextSearchTest::testMultilineSearch_data()
{
    QTest::addColumn<QString>("pattern");
    QTest::addColumn<KTextEditor::Range>("inputRange");
    QTest::addColumn<KTextEditor::Range>("forwardResult");

    QTest::newRow("") << "a a a\na a\na a a" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 0, 2, 5);
    QTest::newRow("") << "a a a\na a\na a " << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 0, 2, 4);
    QTest::newRow("") << "a a a\na a\na a" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 0, 2, 3);
    QTest::newRow("") << "a a a\na a\na" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 0, 2, 1);
    QTest::newRow("") << "a a a\na a\n" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 0, 2, 0);
    QTest::newRow("") << "a a a\na a" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 0, 1, 3);
    QTest::newRow("") << "a a\na a" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 2, 1, 3);
    QTest::newRow("") << "a a\na a\na a" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 2, 2, 3);
    QTest::newRow("") << "\na a\na a" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 5, 2, 3);
    QTest::newRow("") << "\na a\n" << KTextEditor::Range(0, 0, 2, 5) << KTextEditor::Range(0, 5, 2, 0);

    QTest::newRow("") << "a a a\na a\na a a" << KTextEditor::Range(0, 0, 2, 4) << KTextEditor::Range::invalid();
    QTest::newRow("") << "a a a\na a\na a " << KTextEditor::Range(0, 0, 2, 4) << KTextEditor::Range(0, 0, 2, 4);
    QTest::newRow("") << "a a a\na a\n" << KTextEditor::Range(0, 0, 2, 0) << KTextEditor::Range(0, 0, 2, 0);
    QTest::newRow("") << "a a a\na a\n" << KTextEditor::Range(0, 0, 1, 3) << KTextEditor::Range::invalid();
    QTest::newRow("") << "a a\n" << KTextEditor::Range(0, 0, 1, 3) << KTextEditor::Range(0, 2, 1, 0);
    QTest::newRow("") << "a \n" << KTextEditor::Range(0, 0, 1, 3) << KTextEditor::Range::invalid();
}

void PlainTextSearchTest::testMultilineSearch()
{
    QFETCH(QString, pattern);
    QFETCH(KTextEditor::Range, inputRange);
    QFETCH(KTextEditor::Range, forwardResult);

    m_doc->setText(
        QStringLiteral("a a a\n"
                       "a a\n"
                       "a a a"));

    QCOMPARE(m_search->search(pattern, inputRange, false), forwardResult);
}
