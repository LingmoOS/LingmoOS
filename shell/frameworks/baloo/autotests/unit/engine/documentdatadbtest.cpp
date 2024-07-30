/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "documentdatadb.h"
#include "dbtest.h"

using namespace Baloo;

class DocumentDataDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        DocumentDataDB db(DocumentDataDB::create(m_txn), m_txn);

        QByteArray arr = "/home/blah";
        db.put(1, arr);

        QCOMPARE(db.get(1), arr);
        QVERIFY(db.contains(1));

        db.del(1);
        QCOMPARE(db.get(1), QByteArray());
        QVERIFY(!db.contains(1));
    }
};

QTEST_MAIN(DocumentDataDBTest)

#include "documentdatadbtest.moc"
