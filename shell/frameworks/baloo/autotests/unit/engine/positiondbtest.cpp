/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "positiondb.h"
#include "dbtest.h"
#include "positioninfo.h"
#include "vectorpositioninfoiterator.h"

using namespace Baloo;

class PositionDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        QByteArray word("fire");
        PositionInfo pos1;
        pos1.docId = 1;
        pos1.positions = QVector<uint>() << 1 << 5 << 6;

        PositionInfo pos2;
        pos2.docId = 5;
        pos2.positions = QVector<uint>() << 41 << 96 << 116;

        QVector<PositionInfo> list = {pos1, pos2};

        db.put(word, list);
        QVector<PositionInfo> res = db.get(word);
        QCOMPARE(res, list);
    }

    void testIter() {
        PositionDB db(PositionDB::create(m_txn), m_txn);

        QByteArray word("fire");
        PositionInfo pos1;
        pos1.docId = 1;
        pos1.positions = QVector<uint>() << 1 << 5 << 6;

        PositionInfo pos2;
        pos2.docId = 5;
        pos2.positions = QVector<uint>() << 41 << 96 << 116;

        QVector<PositionInfo> list = {pos1, pos2};

        db.put(word, list);

        std::unique_ptr<VectorPositionInfoIterator> it{db.iter(word)};
        QCOMPARE(it->docId(), static_cast<quint64>(0));
        QVERIFY(it->positions().isEmpty());

        QCOMPARE(it->next(), static_cast<quint64>(1));
        QCOMPARE(it->docId(), static_cast<quint64>(1));
        QCOMPARE(it->positions(), pos1.positions);

        QCOMPARE(it->next(), static_cast<quint64>(5));
        QCOMPARE(it->docId(), static_cast<quint64>(5));
        QCOMPARE(it->positions(), pos2.positions);

        QCOMPARE(it->next(), static_cast<quint64>(0));
        QCOMPARE(it->docId(), static_cast<quint64>(0));
        QVERIFY(it->positions().isEmpty());
    }
};

QTEST_MAIN(PositionDBTest)

#include "positiondbtest.moc"
