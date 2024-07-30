/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "phraseanditerator.h"
#include "vectorpositioninfoiterator.h"
#include "positioninfo.h"

#include <QTest>

using namespace Baloo;

class PhraseAndIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void testNullIterators();
};

void PhraseAndIteratorTest::test()
{
    // Term 1
    PositionInfo pi2_1; // pi<doc_id>_<term>
    pi2_1.docId = 2;
    pi2_1.positions = {5, 9};

    PositionInfo pi4_1;
    pi4_1.docId = 4;
    pi4_1.positions = {4, 2};

    QVector<PositionInfo> vec1;
    vec1 << pi2_1 << pi4_1;

    // Term 2
    PositionInfo pi2_2;
    pi2_2.docId = 2;
    pi2_2.positions = {6, 7};

    PositionInfo pi4_2;
    pi4_2.docId = 4;
    pi4_2.positions = {6, 2};

    PositionInfo pi7_2;
    pi7_2.docId = 7;
    pi7_2.positions = {1, 4, 2};

    QVector<PositionInfo> vec2;
    vec2 << pi2_2 << pi4_2 << pi7_2;

    VectorPositionInfoIterator* it1 = new VectorPositionInfoIterator(vec1);
    VectorPositionInfoIterator* it2 = new VectorPositionInfoIterator(vec2);

    QVector<VectorPositionInfoIterator*> vec = {it1, it2};
    PhraseAndIterator it(vec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    // The Query is "term1 term2". term1 must appear one position before term2
    QVector<quint64> result = {2};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

void PhraseAndIteratorTest::testNullIterators()
{
    // Term 1
    PositionInfo pi2_1;
    pi2_1.docId = 2;
    pi2_1.positions = {5, 9};

    QVector<PositionInfo> vec1;
    vec1 << pi2_1;

    // Term 2
    PositionInfo pi2_2;
    pi2_2.docId = 2;
    pi2_2.positions = {6, 7};

    QVector<PositionInfo> vec2;
    vec2 << pi2_2;

    VectorPositionInfoIterator* it1 = new VectorPositionInfoIterator(vec1);
    VectorPositionInfoIterator* it2 = new VectorPositionInfoIterator(vec2);

    QVector<VectorPositionInfoIterator*> vec = {it1, nullptr, it2};
    PhraseAndIterator it(vec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

QTEST_MAIN(PhraseAndIteratorTest)

#include "phraseanditeratortest.moc"
