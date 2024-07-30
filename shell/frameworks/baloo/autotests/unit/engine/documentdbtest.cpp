/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documentdb.h"
#include "dbtest.h"

using namespace Baloo;

class DocumentDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test();
};

void DocumentDBTest::test()
{
    DocumentDB db(DocumentDB::create("db", m_txn), m_txn);

    QVector<QByteArray> list = {"a", "aab", "abc"};
    db.put(1, list);

    QCOMPARE(db.get(1), list);
}

QTEST_MAIN(DocumentDBTest)

#include "documentdbtest.moc"
