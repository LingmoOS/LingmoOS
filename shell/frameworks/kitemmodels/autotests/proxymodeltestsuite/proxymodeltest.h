/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROXY_MODEL_TEST_H
#define PROXY_MODEL_TEST_H

#include <QAbstractProxyModel>
#include <QTest>

#include "dynamictreemodel.h"
#include "indexfinder.h"
#include "modelcommander.h"
#include "modelspy.h"
#include "persistentchangelist.h"

#include "proxymodeltestsuite_export.h"

typedef QList<QVariantList> SignalList;

Q_DECLARE_METATYPE(SignalList)

enum Persistence {
    LazyPersistence,
    ImmediatePersistence,
};

enum SourceModel {
    DynamicTree,
    IntermediateProxy,
};

class PROXYMODELTESTSUITE_EXPORT BuiltinTestDataInterface
{
public:
    virtual ~BuiltinTestDataInterface()
    {
    }

private:
    virtual void testInsertWhenEmptyData() = 0;
    virtual void testInsertInRootData() = 0;
    virtual void testInsertInTopLevelData() = 0;
    virtual void testInsertInSecondLevelData() = 0;

    virtual void testRemoveFromRootData() = 0;
    virtual void testRemoveFromTopLevelData() = 0;
    virtual void testRemoveFromSecondLevelData() = 0;

    virtual void testMoveFromRootData() = 0;
    virtual void testMoveFromTopLevelData() = 0;
    virtual void testMoveFromSecondLevelData() = 0;

    virtual void testModifyInRootData() = 0;
    virtual void testModifyInTopLevelData() = 0;
    virtual void testModifyInSecondLevelData() = 0;
};

class PROXYMODELTESTSUITE_EXPORT BuiltinTestInterface : BuiltinTestDataInterface
{
public:
    ~BuiltinTestInterface() override
    {
    }

private:
    virtual void testInsertWhenEmpty_data() = 0;
    virtual void testInsertWhenEmpty() = 0;

    virtual void testInsertInRoot_data() = 0;
    virtual void testInsertInRoot() = 0;

    virtual void testInsertInTopLevel_data() = 0;
    virtual void testInsertInTopLevel() = 0;

    virtual void testInsertInSecondLevel_data() = 0;
    virtual void testInsertInSecondLevel() = 0;

    virtual void testRemoveFromRoot_data() = 0;
    virtual void testRemoveFromRoot() = 0;

    virtual void testRemoveFromTopLevel_data() = 0;
    virtual void testRemoveFromTopLevel() = 0;

    virtual void testRemoveFromSecondLevel_data() = 0;
    virtual void testRemoveFromSecondLevel() = 0;

    virtual void testMoveFromRoot_data() = 0;
    virtual void testMoveFromRoot() = 0;

    virtual void testMoveFromTopLevel_data() = 0;
    virtual void testMoveFromTopLevel() = 0;

    virtual void testMoveFromSecondLevel_data() = 0;
    virtual void testMoveFromSecondLevel() = 0;

    virtual void testModifyInRoot_data() = 0;
    virtual void testModifyInRoot() = 0;

    virtual void testModifyInTopLevel_data() = 0;
    virtual void testModifyInTopLevel() = 0;

    virtual void testModifyInSecondLevel_data() = 0;
    virtual void testModifyInSecondLevel() = 0;
};

class PROXYMODELTESTSUITE_EXPORT ProxyModelTest : public QObject, protected BuiltinTestInterface
{
    Q_OBJECT
public:
    ProxyModelTest(QObject *parent = nullptr);
    ~ProxyModelTest() override
    {
    }

    void setLazyPersistence(Persistence persistence);
    void setUseIntermediateProxy(SourceModel sourceModel);

    DynamicTreeModel *rootModel() const
    {
        return m_rootModel;
    }
    QAbstractItemModel *sourceModel() const
    {
        return m_sourceModel;
    }
    QAbstractProxyModel *proxyModel() const
    {
        return m_proxyModel;
    }
    ModelSpy *modelSpy() const
    {
        return m_modelSpy;
    }

    PersistentIndexChange getChange(IndexFinder sourceFinder, int start, int end, int difference, bool toInvalid = false);
    QVariantList noSignal() const
    {
        return QVariantList() << NoSignal;
    }
    QVariantList getSignal(SignalType type, IndexFinder parentFinder, int start, int end) const
    {
        return QVariantList() << type << QVariant::fromValue(parentFinder) << start << end;
    }
    QVariantList getSignal(SignalType type, IndexFinder srcFinder, int start, int end, IndexFinder destFinder, int destStart) const
    {
        return QVariantList() << type << QVariant::fromValue(srcFinder) << start << end << QVariant::fromValue(destFinder) << destStart;
    }
    QVariantList getSignal(SignalType type, IndexFinder topLeftFinder, IndexFinder bottomRightFinder) const
    {
        return QVariantList() << type << QVariant::fromValue(topLeftFinder) << QVariant::fromValue(bottomRightFinder);
    }

protected:
    virtual QAbstractProxyModel *getProxy() = 0;

    void doCleanupTestCase()
    {
        cleanupTestCase();
    }
    void doCleanup()
    {
        cleanup();
    }

Q_SIGNALS:
    void testInsertWhenEmptyData() override;
    void testInsertInRootData() override;
    void testInsertInTopLevelData() override;
    void testInsertInSecondLevelData() override;

    void testRemoveFromRootData() override;
    void testRemoveFromTopLevelData() override;
    void testRemoveFromSecondLevelData() override;

    void testMoveFromRootData() override;
    void testMoveFromTopLevelData() override;
    void testMoveFromSecondLevelData() override;

    void testModifyInRootData() override;
    void testModifyInTopLevelData() override;
    void testModifyInSecondLevelData() override;

protected Q_SLOTS:
    void testMappings();
    void verifyModel(const QModelIndex &parent, int start, int end);
    void verifyModel(const QModelIndex &parent, int start, int end, const QModelIndex &destParent, int dest);
    void verifyModel(const QModelIndex &topLeft, const QModelIndex &bottomRight);

private Q_SLOTS:
    void init();
    void cleanup();
    void cleanupTestCase();

    void testEmptyModel();
    void testSourceReset();
    void testDestroyModel();

    void testInsertWhenEmpty_data() override
    {
        testInsertWhenEmptyData();
    }
    void testInsertWhenEmpty() override
    {
        doTest();
    }

    void testInsertInRoot_data() override
    {
        testInsertInRootData();
    }
    void testInsertInRoot() override
    {
        doTest();
    }

    void testInsertInTopLevel_data() override
    {
        testInsertInTopLevelData();
    }
    void testInsertInTopLevel() override
    {
        doTest();
    }

    void testInsertInSecondLevel_data() override
    {
        testInsertInSecondLevelData();
    }
    void testInsertInSecondLevel() override
    {
        doTest();
    }

    void testRemoveFromRoot_data() override
    {
        testRemoveFromRootData();
    }
    void testRemoveFromRoot() override
    {
        doTest();
    }

    void testRemoveFromTopLevel_data() override
    {
        testRemoveFromTopLevelData();
    }
    void testRemoveFromTopLevel() override
    {
        doTest();
    }

    void testRemoveFromSecondLevel_data() override
    {
        testRemoveFromSecondLevelData();
    }
    void testRemoveFromSecondLevel() override
    {
        doTest();
    }

    void testMoveFromRoot_data() override
    {
        testMoveFromRootData();
    }
    void testMoveFromRoot() override
    {
        doTest();
    }

    void testMoveFromTopLevel_data() override
    {
        testMoveFromTopLevelData();
    }
    void testMoveFromTopLevel() override
    {
        doTest();
    }

    void testMoveFromSecondLevel_data() override
    {
        testMoveFromSecondLevelData();
    }
    void testMoveFromSecondLevel() override
    {
        doTest();
    }

    void testModifyInRoot_data() override
    {
        testModifyInRootData();
    }
    void testModifyInRoot() override
    {
        doTest();
    }

    void testModifyInTopLevel_data() override
    {
        testModifyInTopLevelData();
    }
    void testModifyInTopLevel() override
    {
        doTest();
    }

    void testModifyInSecondLevel_data() override
    {
        testModifyInSecondLevelData();
    }
    void testModifyInSecondLevel() override
    {
        doTest();
    }

protected:
    void connectTestSignals(QObject *receiver);
    void disconnectTestSignals(QObject *receiver);

    void connectProxy(QAbstractProxyModel *proxyModel);
    void doTestMappings(const QModelIndex &parent);
    void initRootModel(DynamicTreeModel *rootModel, const QString &currentTest, const QString &currentTag);

    void doTest();
    void handleSignal(QVariantList expected);
    QVariantList getResultSignal();
    int getChange(bool sameParent, int start, int end, int currentPosition, int destinationStart);
    QStringList dataTags() const
    {
        return m_dataTags;
    }
    void verifyExecutedTests();

private:
    DynamicTreeModel *m_rootModel;
    QAbstractItemModel *m_sourceModel;
    QAbstractProxyModel *m_proxyModel;
    QAbstractProxyModel *m_intermediateProxyModel;
    ModelSpy *m_modelSpy;
    ModelCommander *m_modelCommander;
    QStringList m_dataTags;
    QStringList m_modelCommanderTags;
    QString m_currentTest;
};

class PROXYMODELTESTSUITE_EXPORT ProxyModelTestData : public QObject, BuiltinTestDataInterface
{
    Q_OBJECT
public:
    ProxyModelTestData(ProxyModelTest *parent = nullptr)
        : QObject(parent)
        , m_proxyModelTest(parent)
    {
    }

    static const char *failTag()
    {
        return "fail01";
    }

protected:
    void dummyTestData()
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        QTest::newRow(failTag()) << SignalList() << PersistentChangeList();
        QSKIP("Test not implemented");
    }

    void skipTestData(const QString &name)
    {
        processTestName(name);

        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        QTest::newRow(name.toLatin1()) << (SignalList() << (QVariantList() << "skip")) << PersistentChangeList();
    }

    void processTestName(const QString &name)
    {
        if (m_currentTestFunction != QTest::currentTestFunction()) {
            m_testNames.clear();
            m_currentTestFunction = QTest::currentTestFunction();
        }
        m_testNames.insert(name);
    }

    QStringList namedTests()
    {
        return m_testNames.values();
    }

    void noopTest(const QString &name)
    {
        processTestName(name);

        QTest::newRow(name.toLatin1()) << SignalList() << PersistentChangeList();
    }

    void noopLayoutChangeTest(const QString &name)
    {
        processTestName(name);

        SignalList signalList;

        signalList << (QVariantList() << LayoutAboutToBeChanged);
        signalList << (QVariantList() << LayoutChanged);

        QTest::newRow(name.toLatin1()) << signalList << PersistentChangeList();
    }

    void testForwardingInsertData(const IndexFinder &indexFinder)
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        newInsertTest(QStringLiteral("insert01"), indexFinder, 0, 0, 10);
        newInsertTest(QStringLiteral("insert02"), indexFinder, 0, 9, 10);
        newInsertTest(QStringLiteral("insert03"), indexFinder, 10, 10, 10);
        newInsertTest(QStringLiteral("insert04"), indexFinder, 10, 19, 10);
        newInsertTest(QStringLiteral("insert05"), indexFinder, 4, 4, 10);
        newInsertTest(QStringLiteral("insert06"), indexFinder, 4, 13, 10);
        newInsertTest(QStringLiteral("insert07"), indexFinder, 0, 0, 10);
        newInsertTest(QStringLiteral("insert08"), indexFinder, 10, 10, 10);
        newInsertTest(QStringLiteral("insert09"), indexFinder, 4, 4, 10);
        newInsertTest(QStringLiteral("insert10"), indexFinder, 0, 4, 10);
        newInsertTest(QStringLiteral("insert11"), indexFinder, 10, 14, 10);
        newInsertTest(QStringLiteral("insert12"), indexFinder, 4, 8, 10);
        QList<int> rows = indexFinder.rows();
        rows.append(0);
        newInsertTest(QStringLiteral("insert13"), rows, 0, 0, 0);
        newInsertTest(QStringLiteral("insert14"), rows, 0, 9, 0);
        newInsertTest(QStringLiteral("insert15"), rows, 0, 4, 0);
        rows = indexFinder.rows();
        rows.append(9);
        newInsertTest(QStringLiteral("insert16"), rows, 0, 0, 0);
        newInsertTest(QStringLiteral("insert17"), rows, 0, 9, 0);
        newInsertTest(QStringLiteral("insert18"), rows, 0, 4, 0);
    }

    void testForwardingRemoveData(const IndexFinder &indexFinder)
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        newRemoveTest(QStringLiteral("remove01"), indexFinder, 0, 0, 10);
        newRemoveTest(QStringLiteral("remove02"), indexFinder, 0, 4, 10);
        newRemoveTest(QStringLiteral("remove03"), indexFinder, 9, 9, 10);
    }

    void testForwardingMoveData(const IndexFinder &srcFinder, const IndexFinder &destFinder)
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        //     noopLayoutChangeTest("move01");
        //     noopLayoutChangeTest("move02");
        //     noopLayoutChangeTest("move03");
        //     noopLayoutChangeTest("move04");
        //     noopLayoutChangeTest("move05");

        newMoveTest(QStringLiteral("move01"), srcFinder, 0, 0, 10, destFinder, 5);
        newMoveTest(QStringLiteral("move02"), srcFinder, 4, 4, 10, destFinder, 0);
        newMoveTest(QStringLiteral("move03"), srcFinder, 4, 4, 10, destFinder, 10);
        newMoveTest(QStringLiteral("move04"), srcFinder, 9, 9, 10, destFinder, 4);
        newMoveTest(QStringLiteral("move05"), srcFinder, 9, 9, 10, destFinder, 0);
    }

    void testForwardingModifyData(const IndexFinder &parentFinder)
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        newModifyTest(QStringLiteral("modify01"), parentFinder, 0, 0);
        newModifyTest(QStringLiteral("modify02"), parentFinder, 0, 4);
        newModifyTest(QStringLiteral("modify03"), parentFinder, 9, 9);
        newModifyTest(QStringLiteral("modify04"), parentFinder, 6, 9);
        newModifyTest(QStringLiteral("modify05"), parentFinder, 4, 4);
        newModifyTest(QStringLiteral("modify06"), parentFinder, 3, 7);
        newModifyTest(QStringLiteral("modify07"), parentFinder, 0, 9);
    }

    void newInsertTest(const QString &name, const IndexFinder &indexFinder, int start, int end, int rowCount)
    {
        processTestName(name);

        SignalList signalList;
        PersistentChangeList persistentList;

        signalList << m_proxyModelTest->getSignal(RowsAboutToBeInserted, indexFinder, start, end);
        signalList << m_proxyModelTest->getSignal(RowsInserted, indexFinder, start, end);

        if (rowCount - 1 + (end - start + 1) > end) {
            persistentList << m_proxyModelTest->getChange(indexFinder, start, rowCount - 1, end - start + 1);
        }

        QTest::newRow(name.toLatin1()) << signalList << persistentList;
    }

    void newRemoveTest(const QString &name, const IndexFinder &indexFinder, int start, int end, int rowCount)
    {
        processTestName(name);

        SignalList signalList;
        PersistentChangeList persistentList;

        signalList << m_proxyModelTest->getSignal(RowsAboutToBeRemoved, indexFinder, start, end);
        signalList << m_proxyModelTest->getSignal(RowsRemoved, indexFinder, start, end);

        persistentList << m_proxyModelTest->getChange(indexFinder, start, end, -1, true);
        if (rowCount - 1 != end) {
            persistentList << m_proxyModelTest->getChange(indexFinder, end + 1, rowCount - 1, -1 * (end - start + 1));
        }

        QTest::newRow(name.toLatin1()) << signalList << persistentList;
    }

    void newMoveTest(const QString &name, const IndexFinder &srcFinder, int start, int end, int rowCount, const IndexFinder &destFinder, int destStart)
    {
        Q_UNUSED(rowCount)
        processTestName(name);

        SignalList signalList;
        PersistentChangeList persistentList;

        //     signalList << m_proxyModelTest->getSignal(RowsAboutToBeMoved, srcFinder, start, end, destFinder, destStart);
        //     signalList << ( QVariantList() << LayoutAboutToBeChanged );
        //     signalList << m_proxyModelTest->getSignal(RowsMoved, srcFinder, start, end, destFinder, destStart);
        //     signalList << ( QVariantList() << LayoutChanged );

        signalList << (QVariantList() << LayoutAboutToBeChanged);
        signalList << (QVariantList() << LayoutChanged);

        const bool sameParent = (srcFinder == destFinder);
        const bool movingUp = (start > destStart);

        if (sameParent) {
            if (movingUp) {
                persistentList << m_proxyModelTest->getChange(srcFinder, destStart, start - 1, end - start + 1);
                persistentList << m_proxyModelTest->getChange(srcFinder, start, end, -1 * (start - destStart));
            } else {
                persistentList << m_proxyModelTest->getChange(srcFinder, start, end, destStart - end - 1);
                persistentList << m_proxyModelTest->getChange(srcFinder, end + 1, destStart - 1, -1 * (end - start + 1));
            }
        } else {
            if (movingUp) {
                // TODO
            }
        }

        QTest::newRow(name.toLatin1()) << signalList << persistentList;
    }

    void newModifyTest(const QString &name, const IndexFinder &parentFinder, int top, int bottom)
    {
        processTestName(name);

        SignalList signalList;

        IndexFinder topLeftFinder = parentFinder;
        topLeftFinder.appendRow(top);

        IndexFinder bottomRightFinder = parentFinder;
        bottomRightFinder.appendRow(bottom);

        signalList << m_proxyModelTest->getSignal(DataChanged, topLeftFinder, bottomRightFinder);

        QTest::newRow(name.toLatin1()) << signalList << PersistentChangeList();
    }

    void noop_testInsertWhenEmptyData()
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        noopTest(QStringLiteral("insert01"));
        noopTest(QStringLiteral("insert02"));
        noopTest(QStringLiteral("insert03"));
    }

    void noop_testInsertInRootData()
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        // These commands have no effect because this model shows children of selection.

        noopTest(QStringLiteral("insert01"));
        noopTest(QStringLiteral("insert02"));
        noopTest(QStringLiteral("insert03"));
        noopTest(QStringLiteral("insert04"));
        noopTest(QStringLiteral("insert05"));
        noopTest(QStringLiteral("insert06"));
        noopTest(QStringLiteral("insert07"));
        noopTest(QStringLiteral("insert08"));
        noopTest(QStringLiteral("insert09"));
        noopTest(QStringLiteral("insert10"));
        noopTest(QStringLiteral("insert11"));
        noopTest(QStringLiteral("insert12"));
        noopTest(QStringLiteral("insert13"));
        noopTest(QStringLiteral("insert14"));
        noopTest(QStringLiteral("insert15"));
        noopTest(QStringLiteral("insert16"));
        noopTest(QStringLiteral("insert17"));
        noopTest(QStringLiteral("insert18"));
    }

    void noop_testInsertInTopLevelData()
    {
        // Same test names etc.
        noop_testInsertInRootData();
    }

    void noop_testInsertInSecondLevelData()
    {
        noop_testInsertInRootData();
    }

    void noop_testRemoveFromRootData()
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        // These commands have no effect because this model shows children of selection.

        noopTest(QStringLiteral("remove01"));
        noopTest(QStringLiteral("remove02"));
        noopTest(QStringLiteral("remove03"));
    }

    void noop_testRemoveFromTopLevelData()
    {
        // Same test names etc.
        noop_testRemoveFromRootData();
    }

    void noop_testRemoveFromSecondLevelData()
    {
        noop_testRemoveFromRootData();
    }

    void noop_testMoveFromRootData()
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        // These commands have no effect because this model shows children of selection.

        noopLayoutChangeTest(QStringLiteral("move01"));
        noopLayoutChangeTest(QStringLiteral("move02"));
        noopLayoutChangeTest(QStringLiteral("move03"));
        noopLayoutChangeTest(QStringLiteral("move04"));
        noopLayoutChangeTest(QStringLiteral("move05"));
    }

    void noop_testMoveFromTopLevelData()
    {
        // Same test names etc.
        noop_testMoveFromRootData();
    }

    void noop_testMoveFromSecondLevelData()
    {
        noop_testMoveFromRootData();
    }

    void noop_testModifyInRootData()
    {
        QTest::addColumn<SignalList>("signalList");
        QTest::addColumn<PersistentChangeList>("changeList");

        noopTest(QStringLiteral("modify01"));
        noopTest(QStringLiteral("modify02"));
        noopTest(QStringLiteral("modify03"));
        noopTest(QStringLiteral("modify04"));
        noopTest(QStringLiteral("modify05"));
        noopTest(QStringLiteral("modify06"));
        noopTest(QStringLiteral("modify07"));
    }

    void noop_testModifyInTopLevelData()
    {
        // Same test names etc.
        noop_testModifyInRootData();
    }

    void noop_testModifyInSecondLevelData()
    {
        noop_testModifyInRootData();
    }

    ProxyModelTest *m_proxyModelTest;
    QString m_currentTestFunction;
    QSet<QString> m_testNames;
};

PROXYMODELTESTSUITE_EXPORT uint qHash(const QVariant &var);

/* clang-format off */
#define PROXYMODELTEST(TestData, TemplateArg, IntermediateProxy, LazyPersistence, Config) \
    if (testObjects.isEmpty() || testObjects.contains(testNum)) { \
        proxyModelTestClass->setTestData(new TestData<TemplateArg>(proxyModelTestClass)); \
        proxyModelTestClass->setUseIntermediateProxy(IntermediateProxy); \
        proxyModelTestClass->setLazyPersistence(LazyPersistence); \
        qDebug() << "\n   Running" << proxyModelTestClass->objectName().toLatin1() << testNum << ":\n" \
                 << "  Source Model:      " << #IntermediateProxy << "\n" \
                 << "  Persistence:       " << #LazyPersistence << "\n" Config; \
        result = QTest::qExec(proxyModelTestClass, arguments); \
        if (result != 0) \
            return result; \
    } \
    ++testNum;

#define PROXYMODELTEST_CUSTOM(TestData, IntermediateProxy, LazyPersistence, Config) \
    if (testObjects.isEmpty() || testObjects.contains(testNum)) { \
        proxyModelTestClass->setTestData(TestData); \
        proxyModelTestClass->setUseIntermediateProxy(IntermediateProxy); \
        proxyModelTestClass->setLazyPersistence(LazyPersistence); \
        qDebug() << "\n   Running" << proxyModelTestClass->objectName().toLatin1() << testNum << ":\n" \
                 << "  Source Model:      " << #IntermediateProxy << "\n" \
                 << "  Persistence:       " << #LazyPersistence << "\n" Config; \
        result = QTest::qExec(proxyModelTestClass, arguments); \
        if (result != 0) \
            return result; \
    } \
    ++testNum;

// The DynamicTreeModel uses a unique internalId for the first column of each row.
// In the QSortFilterProxyModel the internalId is shared between all rows of the same parent.
// We test the proxy on top of both so that we know it is not using the internalId of its source model
// which will be different each time the test is run.
#define COMPLETETEST(TestData, TemplateArg, Config) \
    PROXYMODELTEST(TestData, TemplateArg, DynamicTree, ImmediatePersistence, Config) \
    PROXYMODELTEST(TestData, TemplateArg, IntermediateProxy, ImmediatePersistence, Config) \
    PROXYMODELTEST(TestData, TemplateArg, DynamicTree, LazyPersistence, Config) \
    PROXYMODELTEST(TestData, TemplateArg, IntermediateProxy, LazyPersistence, Config)

#define PROXYMODELTEST_MAIN(TestClass, Body) \
    int main(int argc, char *argv[]) \
    { \
        QApplication app(argc, argv); \
        QList<int> testObjects; \
        QStringList arguments; \
        bool ok; \
        const auto lst = app.arguments(); \
        for (const QString &arg : lst) { \
            int testObject = arg.toInt(&ok); \
            if (arg == "-count") \
                continue; \
            if (!ok) { \
                arguments.append(arg); \
                continue; \
            } \
            testObjects.append(testObject); \
        } \
        TestClass *proxyModelTestClass = new TestClass(); \
        proxyModelTestClass->setObjectName(#TestClass); \
        int result = 0; \
        int testNum = 1; \
        \
        Body \
        \
            delete proxyModelTestClass; \
        return result; \
    }

/* clang-format on */
#endif
