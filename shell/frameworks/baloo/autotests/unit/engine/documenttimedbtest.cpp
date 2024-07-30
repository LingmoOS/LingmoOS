/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documenttimedb.h"
#include "dbtest.h"

using namespace Baloo;

class DocumentTimeDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testAllowZeroTime();
};

void DocumentTimeDBTest::test()
{
    DocumentTimeDB db(DocumentTimeDB::create(m_txn), m_txn);

    DocumentTimeDB::TimeInfo info;
    info.mTime = 5;
    info.cTime = 6;

    db.put(1, info);
    QCOMPARE(db.get(1), info);

    db.del(1);
    QCOMPARE(db.get(1), DocumentTimeDB::TimeInfo());
}

void DocumentTimeDBTest::testAllowZeroTime()
{
    DocumentTimeDB db(DocumentTimeDB::create(m_txn), m_txn);

    // we must be able to handle zero time, aka 1970...
    DocumentTimeDB::TimeInfo info;
    info.mTime = 0;
    info.cTime = 0;

    db.put(1, info);
    QCOMPARE(db.get(1), info);

    db.del(1);
    QCOMPARE(db.get(1), DocumentTimeDB::TimeInfo());
}

QTEST_MAIN(DocumentTimeDBTest)

#include "documenttimedbtest.moc"
