/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "postingdb.h"
#include "dbtest.h"

using namespace Baloo;

class PostingDBTest : public DBTest
{
    Q_OBJECT
private Q_SLOTS:
    void test() {
        PostingDB db(PostingDB::create(m_txn), m_txn);

        QByteArray word("fire");
        PostingList list = {1, 5, 6};

        db.put(word, list);
        QCOMPARE(db.get(word), list);
    }

    void testTermIter() {
        PostingDB db(PostingDB::create(m_txn), m_txn);

        db.put("abc", {1, 4, 5, 9, 11});
        db.put("fir", {1, 3, 5});
        db.put("fire", {1, 8, 9});
        db.put("fore", {2, 3, 5});

        std::unique_ptr<PostingIterator> it{db.iter("fir")};
        QVERIFY(it);

        QVector<quint64> result = {1, 3, 5};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }

    void testPrefixIter() {
        PostingDB db(PostingDB::create(m_txn), m_txn);

        db.put("abc", {1, 4, 5, 9, 11});
        db.put("fir", {1, 3, 5});
        db.put("fire", {1, 8, 9});
        db.put("fore", {2, 3, 5});

        std::unique_ptr<PostingIterator> it{db.prefixIter("fi")};
        QVERIFY(it);

        QVector<quint64> result = {1, 3, 5, 8, 9};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }

    void testRegExpIter() {
        PostingDB db(PostingDB::create(m_txn), m_txn);

        db.put("abc", {1, 4, 5, 9, 11});
        db.put("fir", {1, 3, 5, 7});
        db.put("fire", {1, 8});
        db.put("fore", {2, 3, 5});
        db.put("zib", {4, 5, 6});

        std::unique_ptr<PostingIterator> it{db.regexpIter(QRegularExpression(QStringLiteral(".re")), QByteArray("f"))};
        QVERIFY(it);

        QVector<quint64> result = {1, 2, 3, 5, 8};
        for (quint64 val : result) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        // Non existing
        it.reset(db.regexpIter(QRegularExpression(QStringLiteral("dub")), QByteArray("f")));
        QVERIFY(it == nullptr);
    }

    void testCompIter() {
        PostingDB db(PostingDB::create(m_txn), m_txn);

        db.put("abc", {1, 4, 5, 9, 11});
        db.put("R1", {1, 3, 5, 7});
        db.put("R2", {1, 8});
        db.put("R3", {2, 3, 5});
        db.put("R10", {10, 12});
        // X20- is the internal encoding for KFileMetaData::Property::LineCount
        db.put("X20-90", {1, 2});
        db.put("X20-1000", {10, 11});

        std::unique_ptr<PostingIterator> it{db.compIter("R", 2, PostingDB::GreaterEqual)};
        QVERIFY(it);

        QVector<quint64> result = {1, 2, 3, 5, 8, 10, 12};
        for (quint64 val : std::as_const(result)) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it.reset(db.compIter("R", 2, PostingDB::LessEqual));
        QVERIFY(it);
        result = {1, 3, 5, 7, 8};
        for (quint64 val : std::as_const(result)) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it.reset(db.compIter("R", 10, PostingDB::GreaterEqual));
        QVERIFY(it);
        result = {10, 12};
        for (quint64 val : std::as_const(result)) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it.reset(db.compIter("X20-", 80, PostingDB::GreaterEqual));
        QVERIFY(it);
        result = {1, 2, 10, 11};
        for (quint64 val : std::as_const(result)) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }

        it.reset(db.compIter("X20-", 100, PostingDB::GreaterEqual));
        QVERIFY(it);
        result = {10, 11};
        for (quint64 val : std::as_const(result)) {
            QCOMPARE(it->next(), static_cast<quint64>(val));
            QCOMPARE(it->docId(), static_cast<quint64>(val));
        }
    }

    void testFetchTermsStartingWith() {
        PostingDB db(PostingDB::create(m_txn), m_txn);

        db.put("abc", {1, 4, 5, 9, 11});
        db.put("fir", {1, 3, 5, 7});
        db.put("fire", {1, 8});
        db.put("fore", {2, 3, 5});
        db.put("zib", {4, 5, 6});

        QVector<QByteArray> list = {"fir", "fire", "fore"};
        QCOMPARE(db.fetchTermsStartingWith("f"), list);
    }
};

QTEST_MAIN(PostingDBTest)

#include "postingdbtest.moc"
