/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documentiddb.h"
#include "dbtest.h"

using namespace Baloo;

class DocIdDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testFetchItems();

    void testSize()
    {
        DocumentIdDB db(DocumentIdDB::create("foo", m_txn), m_txn);

        db.put(1);
        db.put(6);
        db.put(8);
        QCOMPARE(db.size(), static_cast<uint>(3));
    }
};

void DocIdDBTest::test()
{
    DocumentIdDB db(DocumentIdDB::create("foo", m_txn), m_txn);

    QCOMPARE(db.contains(1), false);
    db.put(1);
    QCOMPARE(db.contains(1), true);

    db.del(1);
    QCOMPARE(db.contains(1), false);
}

void DocIdDBTest::testFetchItems()
{
    DocumentIdDB db(DocumentIdDB::create("foo", m_txn), m_txn);

    db.put(1);
    db.put(6);
    db.put(8);

    QVector<quint64> acVec = db.fetchItems(10);
    QVector<quint64> exVec = {1, 6, 8};

    QCOMPARE(acVec, exVec);
}

QTEST_MAIN(DocIdDBTest)

#include "documentiddbtest.moc"
