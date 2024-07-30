/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "andpostingiterator.h"
#include "orpostingiterator.h"
#include "vectorpostingiterator.h"

#include <QTest>

using namespace Baloo;

class PostingIteratorTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void test();
    void test2();
};

void PostingIteratorTest::test()
{
    QVector<quint64> l1 = {1, 3, 5, 7};
    QVector<quint64> l2 = {2, 3, 4, 9, 11};
    QVector<quint64> l3 = {4, 7};

    VectorPostingIterator* it1 = new VectorPostingIterator(l1);
    VectorPostingIterator* it2 = new VectorPostingIterator(l2);
    VectorPostingIterator* it3 = new VectorPostingIterator(l3);

    QVector<PostingIterator*> orvec = {it2, it3};
    OrPostingIterator* orit = new OrPostingIterator(orvec);
    QVector<PostingIterator*> andvec = {it1, orit};
    AndPostingIterator it(andvec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    QVector<quint64> result = {3, 7};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

void PostingIteratorTest::test2()
{
    QVector<quint64> l1 = {1, 3, 5, 7};
    QVector<quint64> l2 = {2, 3, 4, 9, 11};
    QVector<quint64> l3 = {3, 7};

    VectorPostingIterator* it1 = new VectorPostingIterator(l1);
    VectorPostingIterator* it2 = new VectorPostingIterator(l2);
    VectorPostingIterator* it3 = new VectorPostingIterator(l3);

    QVector<PostingIterator*> orvec = {new OrPostingIterator({it2}), new OrPostingIterator({it3}) };
    OrPostingIterator* orit = new OrPostingIterator(orvec);
    QVector<PostingIterator*> andvec = {it1, orit};
    AndPostingIterator it(andvec);
    QCOMPARE(it.docId(), static_cast<quint64>(0));

    QVector<quint64> result = {3, 7};
    for (quint64 val : result) {
        QCOMPARE(it.next(), static_cast<quint64>(val));
        QCOMPARE(it.docId(), static_cast<quint64>(val));
    }
    QCOMPARE(it.next(), static_cast<quint64>(0));
    QCOMPARE(it.docId(), static_cast<quint64>(0));
}

QTEST_MAIN(PostingIteratorTest)

#include "postingiteratortest.moc"
