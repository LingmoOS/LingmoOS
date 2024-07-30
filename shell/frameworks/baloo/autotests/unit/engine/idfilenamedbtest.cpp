/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "idfilenamedb.h"
#include "dbtest.h"

using namespace Baloo;

class IdFilenameDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        IdFilenameDB db(IdFilenameDB::create(m_txn), m_txn);

        IdFilenameDB::FilePath path;
        path.parentId = 5;
        path.name = "fire";

        db.put(1, path);

        QCOMPARE(db.get(1), path);

        db.del(1);
        QCOMPARE(db.get(1), IdFilenameDB::FilePath());
    }
};

QTEST_MAIN(IdFilenameDBTest)

#include "idfilenamedbtest.moc"
