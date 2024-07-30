/*
    SPDX-FileCopyrightText: 2010 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.net>
    SPDX-FileContributor: Stephen Kelly <stephen@kdab.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "proxymodeltestsuite/proxymodeltest.h"
#include <kdescendantsproxymodel.h>

#include <QApplication>
#include <QDebug>

class TestData : public ProxyModelTestData
{
    Q_OBJECT
public:
    TestData(ProxyModelTest *parent = nullptr)
        : ProxyModelTestData(parent)
    {
    }

    void
    newInsertWithDataChangedTest(const QString &name, const IndexFinder &indexFinder, int start, int end, int rowCount, const QList<int> &indexWithDataChanged)
    {
        processTestName(name);

        SignalList signalList;
        PersistentChangeList persistentList;

        signalList << m_proxyModelTest->getSignal(RowsAboutToBeInserted, indexFinder, start, end);
        signalList << m_proxyModelTest->getSignal(RowsInserted, indexFinder, start, end);

        for (int index : indexWithDataChanged) {
            const IndexFinder changedFinder({index});
            signalList << m_proxyModelTest->getSignal(DataChanged, changedFinder, changedFinder);
        }

        if (rowCount - 1 + (end - start + 1) > end) {
            persistentList << m_proxyModelTest->getChange(indexFinder, start, rowCount - 1, end - start + 1);
        }

        QTest::newRow(name.toLatin1()) << signalList << persistentList;
    }

    void
    newRemoveWithDataChangedTest(const QString &name, const IndexFinder &indexFinder, int start, int end, int rowCount, const QList<int> &indexWithDataChanged)
    {
        processTestName(name);

        SignalList signalList;
        PersistentChangeList persistentList;

        signalList << m_proxyModelTest->getSignal(RowsAboutToBeRemoved, indexFinder, start, end);
        signalList << m_proxyModelTest->getSignal(RowsRemoved, indexFinder, start, end);

        for (int index : indexWithDataChanged) {
            const IndexFinder changedFinder({index});
            signalList << m_proxyModelTest->getSignal(DataChanged, changedFinder, changedFinder);
        }

        persistentList << m_proxyModelTest->getChange(indexFinder, start, end, -1, true);
        if (rowCount - 1 != end) {
            persistentList << m_proxyModelTest->getChange(indexFinder, end + 1, rowCount - 1, -1 * (end - start + 1));
        }

        QTest::newRow(name.toLatin1()) << signalList << persistentList;
    }

public Q_SLOTS:
    void testInsertWhenEmptyData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;

        newInsertTest(QStringLiteral("insert01"), indexFinder, 0, 0, 0);
        newInsertTest(QStringLiteral("insert02"), indexFinder, 0, 9, 0);

        // The test suite can't handle tree insertion yet, so we skip it for now.
        skipTestData(QStringLiteral("insert03"));

        //     processTestName("insert03");

        //     SignalList signalList;
        //     PersistentChangeList persistentList;

        //     partialTest(&signalList, &persistentList, indexFinder, 0, 4, 0);
        //     partialTest(&signalList, &persistentList, indexFinder, 1, 1, 5);
        //     partialTest(&signalList, &persistentList, indexFinder, 4, 4, 6);
        //     partialTest(&signalList, &persistentList, indexFinder, 7, 7, 7);
        //     partialTest(&signalList, &persistentList, indexFinder, 5, 5, 8);

        //     QTest::newRow("insert03") << signalList << persistentList;
    }

    void testInsertInRootData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;

        newInsertTest(QStringLiteral("insert01"), indexFinder, 0, 0, 43);
        newInsertTest(QStringLiteral("insert02"), indexFinder, 0, 9, 43);
        newInsertWithDataChangedTest(QStringLiteral("insert03"), indexFinder, 43, 43, 43, QList<int>({42}));
        newInsertWithDataChangedTest(QStringLiteral("insert04"), indexFinder, 43, 52, 43, QList<int>({42}));
        newInsertWithDataChangedTest(QStringLiteral("insert05"), indexFinder, 7, 7, 43, QList<int>({6}));
        newInsertWithDataChangedTest(QStringLiteral("insert06"), indexFinder, 7, 16, 43, QList<int>({6}));
        skipTestData(QStringLiteral("insert07"));
        skipTestData(QStringLiteral("insert08"));
        skipTestData(QStringLiteral("insert09"));
        skipTestData(QStringLiteral("insert10"));
        skipTestData(QStringLiteral("insert11"));
        skipTestData(QStringLiteral("insert12"));
        skipTestData(QStringLiteral("insert13"));
        skipTestData(QStringLiteral("insert14"));
        skipTestData(QStringLiteral("insert15"));
        skipTestData(QStringLiteral("insert16"));
        skipTestData(QStringLiteral("insert17"));
        skipTestData(QStringLiteral("insert18"));
    }

    void testInsertInTopLevelData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;

        newInsertWithDataChangedTest(QStringLiteral("insert01"), indexFinder, 9, 9, 43, QList<int>({8}));
        newInsertWithDataChangedTest(QStringLiteral("insert02"), indexFinder, 9, 18, 43, QList<int>({8}));
        newInsertWithDataChangedTest(QStringLiteral("insert03"), indexFinder, 37, 37, 43, QList<int>({8, 36}));

        newInsertWithDataChangedTest(QStringLiteral("insert04"), indexFinder, 37, 46, 43, QList<int>({8, 36}));
        newInsertWithDataChangedTest(QStringLiteral("insert05"), indexFinder, 15, 15, 43, QList<int>({8, 14}));
        newInsertWithDataChangedTest(QStringLiteral("insert06"), indexFinder, 15, 24, 43, QList<int>({8, 14}));
        skipTestData(QStringLiteral("insert07"));
        skipTestData(QStringLiteral("insert08"));
        skipTestData(QStringLiteral("insert09"));
        skipTestData(QStringLiteral("insert10"));
        skipTestData(QStringLiteral("insert11"));
        skipTestData(QStringLiteral("insert12"));
        skipTestData(QStringLiteral("insert13"));
        skipTestData(QStringLiteral("insert14"));
        skipTestData(QStringLiteral("insert15"));
        skipTestData(QStringLiteral("insert16"));
        skipTestData(QStringLiteral("insert17"));
        skipTestData(QStringLiteral("insert18"));
    }

    void testInsertInSecondLevelData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;

        newInsertWithDataChangedTest(QStringLiteral("insert01"), indexFinder, 17, 17, 43, QList<int>({16}));
        newInsertWithDataChangedTest(QStringLiteral("insert02"), indexFinder, 17, 26, 43, QList<int>({16}));
        newInsertWithDataChangedTest(QStringLiteral("insert03"), indexFinder, 32, 32, 43, QList<int>({16, 31}));
        newInsertWithDataChangedTest(QStringLiteral("insert04"), indexFinder, 32, 41, 43, QList<int>({16, 31}));
        newInsertWithDataChangedTest(QStringLiteral("insert05"), indexFinder, 23, 23, 43, QList<int>({16, 22}));
        newInsertWithDataChangedTest(QStringLiteral("insert06"), indexFinder, 23, 32, 43, QList<int>({16, 22}));
        skipTestData(QStringLiteral("insert07"));
        skipTestData(QStringLiteral("insert08"));
        skipTestData(QStringLiteral("insert09"));
        skipTestData(QStringLiteral("insert10"));
        skipTestData(QStringLiteral("insert11"));
        skipTestData(QStringLiteral("insert12"));
        skipTestData(QStringLiteral("insert13"));
        skipTestData(QStringLiteral("insert14"));
        skipTestData(QStringLiteral("insert15"));
        skipTestData(QStringLiteral("insert16"));
        skipTestData(QStringLiteral("insert17"));
        skipTestData(QStringLiteral("insert18"));
    }

    void testRemoveFromRootData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;

        newRemoveTest(QStringLiteral("remove01"), indexFinder, 0, 0, 43);
        newRemoveTest(QStringLiteral("remove02"), indexFinder, 0, 7, 43);
        newRemoveWithDataChangedTest(QStringLiteral("remove03"), indexFinder, 42, 42, 43, QList<int>({41}));
    }

    void testRemoveFromTopLevelData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;

        newRemoveWithDataChangedTest(QStringLiteral("remove01"), indexFinder, 9, 9, 43, QList<int>({8}));
        newRemoveWithDataChangedTest(QStringLiteral("remove02"), indexFinder, 9, 15, 43, QList<int>({8}));
        newRemoveWithDataChangedTest(QStringLiteral("remove03"), indexFinder, 36, 36, 43, QList<int>({35, 8, 35}));
    }

    void testRemoveFromSecondLevelData() override
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        static const IndexFinder indexFinder;
        newRemoveWithDataChangedTest(QStringLiteral("remove01"), indexFinder, 17, 17, 43, QList<int>({16}));
        newRemoveWithDataChangedTest(QStringLiteral("remove02"), indexFinder, 17, 23, 43, QList<int>({16}));
        newRemoveWithDataChangedTest(QStringLiteral("remove03"), indexFinder, 31, 31, 43, QList<int>({30, 16, 30}));
    }

    void testMoveFromRootData() override
    {
        dummyTestData();
    }

    void testMoveFromTopLevelData() override
    {
        dummyTestData();
    }

    void testMoveFromSecondLevelData() override
    {
        dummyTestData();
    }

    void testModifyInRootData() override
    {
        dummyTestData();
    }

    void testModifyInTopLevelData() override
    {
        dummyTestData();
    }

    void testModifyInSecondLevelData() override
    {
        dummyTestData();
    }
};

class DescendantsProxyModelTest : public ProxyModelTest
{
    Q_OBJECT
public:
    DescendantsProxyModelTest(QObject *parent = nullptr)
        : ProxyModelTest(parent)
    {
    }

    void setTestData(TestData *data)
    {
        qDebug() << data;
        connectTestSignals(data);
    }

protected:
    QAbstractProxyModel *getProxy() override
    {
        return new KDescendantsProxyModel(this);
    }

private Q_SLOTS:
    void cleanupTestCase()
    {
        doCleanupTestCase();
    }

    void cleanup()
    {
        doCleanup();
    }

private:
    KDescendantsProxyModel *m_proxyModel;
};

PROXYMODELTEST_MAIN(DescendantsProxyModelTest,
                    PROXYMODELTEST_CUSTOM(new TestData, DynamicTree, ImmediatePersistence, "")
                        PROXYMODELTEST_CUSTOM(new TestData, DynamicTree, LazyPersistence, "")
                            PROXYMODELTEST_CUSTOM(new TestData, IntermediateProxy, ImmediatePersistence, "")
                                PROXYMODELTEST_CUSTOM(new TestData, IntermediateProxy, LazyPersistence, ""))

#include "kdescendantsproxymodel_smoketest.moc"
