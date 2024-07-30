/*
    This file is part of the KDE Baloo Project
    SPDX-FileCopyrightText: 2013-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "query.h"
#include "term.h"

#include <QTest>

using namespace Baloo;

class QuerySerializationTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testBasic();
    void testTerm();
    void testAndTerm();
    void testDateTerm();
    void testDateTimeTerm();

    void testCustomOptions();
};

// Test a simple query with no terms
void QuerySerializationTest::testBasic()
{
    Query query;
    query.setLimit(5);
    query.setOffset(1);
    query.setSearchString(QStringLiteral("Bookie"));
    query.addType(QStringLiteral("File/Audio"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q.limit(), static_cast<uint>(5));
    QCOMPARE(q.offset(), static_cast<uint>(1));
    QCOMPARE(q.searchString(), QLatin1String("Bookie"));
    QCOMPARE(q.types().size(), 2);
    QVERIFY(q.types().contains(QLatin1String("File")));
    QVERIFY(q.types().contains(QLatin1String("Audio")));

    QCOMPARE(q, query);
}

void QuerySerializationTest::testTerm()
{
    Query query;
    query.setSearchString(QStringLiteral("prop:value"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testAndTerm()
{
    Query query;
    query.setSearchString(QStringLiteral("prop1:1 AND prop2:2"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testDateTerm()
{
    Query query;
    query.setSearchString(QStringLiteral("prop:2015-05-01"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testDateTimeTerm()
{
    Query query;
    query.setSearchString(QStringLiteral("prop:2015-05-01T23:44:11"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QCOMPARE(q, query);
}

void QuerySerializationTest::testCustomOptions()
{
    Query query;
    query.addType(QStringLiteral("File"));
    query.setIncludeFolder(QStringLiteral("/home/vishesh"));

    QByteArray json = query.toJSON();
    Query q = Query::fromJSON(json);

    QString includeFolder = q.includeFolder();
    QCOMPARE(includeFolder, QStringLiteral("/home/vishesh"));

    QCOMPARE(query, q);
}

QTEST_MAIN(QuerySerializationTest)

#include "queryserializationtest.moc"
