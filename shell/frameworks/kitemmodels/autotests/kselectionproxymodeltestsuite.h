/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef SELECTIONPROXYMODELTEST_H
#define SELECTIONPROXYMODELTEST_H

#include "proxymodeltest.h"

#include "kselectionproxymodel.h"
#include "modelselector.h"

class SelectionProxyModelTest : public ProxyModelTest
{
    Q_OBJECT
public:
    SelectionProxyModelTest(QObject *parent = nullptr)
        : ProxyModelTest(parent)
        , m_selectionModel(nullptr)
        , m_modelSelector(nullptr)
    {
    }

    void setTestData(ModelSelector *modelSelector)
    {
        disconnectTestSignals(modelSelector);
        m_modelSelector = modelSelector;
        connectTestSignals(m_modelSelector);
    }

protected:
    QAbstractProxyModel *getProxy() override
    {
        Q_ASSERT(sourceModel());
        m_selectionModel = new QItemSelectionModel(sourceModel());

        m_modelSelector->setWatchedModel(sourceModel());
        m_modelSelector->setSelectionModel(m_selectionModel);
        m_modelSelector->setWatch(true);

        m_proxyModel = new KSelectionProxyModel(m_selectionModel, this);
        QVariant filterBehaviour = m_modelSelector->property("filterBehaviour");
        if (!filterBehaviour.isNull()) {
            m_proxyModel->setFilterBehavior(static_cast<KSelectionProxyModel::FilterBehavior>(filterBehaviour.toInt()));
        }
        return m_proxyModel;
    }

private Q_SLOTS:
    void cleanupTestCase()
    {
        doCleanupTestCase();
        delete m_modelSelector;
        m_modelSelector = nullptr;
    }

    void cleanup()
    {
        doCleanup();
        if (m_modelSelector->selectionModel()) {
            m_modelSelector->selectionModel()->clearSelection();
        }
        m_modelSelector->setWatch(false);
    }

private:
    QItemSelectionModel *m_selectionModel;
    KSelectionProxyModel *m_proxyModel;
    ModelSelector *m_modelSelector;
};

template<typename SelectorStrategy, KSelectionProxyModel::FilterBehavior _filterBehaviour>
class TestData : public SelectorStrategy
{
public:
    TestData(ProxyModelTest *proxyModelTest)
        : SelectorStrategy(proxyModelTest)
    {
        SelectorStrategy::setProperty("filterBehaviour", QVariant(int(_filterBehaviour)));
    }

    void testInsertWhenEmptyData() override
    {
        SelectorStrategy::testInsertWhenEmptyData();
    }
    void testInsertInRootData() override
    {
        SelectorStrategy::testInsertInRootData();
    }
    void testInsertInTopLevelData() override
    {
        SelectorStrategy::testInsertInTopLevelData();
    }
    void testInsertInSecondLevelData() override
    {
        SelectorStrategy::testInsertInSecondLevelData();
    }

    void testRemoveFromRootData() override
    {
        SelectorStrategy::testRemoveFromRootData();
    }
    void testRemoveFromTopLevelData() override
    {
        SelectorStrategy::testRemoveFromTopLevelData();
    }
    void testRemoveFromSecondLevelData() override
    {
        SelectorStrategy::testRemoveFromSecondLevelData();
    }

    void testMoveFromRootData() override
    {
        SelectorStrategy::testMoveFromRootData();
    }
    void testMoveFromTopLevelData() override
    {
        SelectorStrategy::testMoveFromTopLevelData();
    }
    void testMoveFromSecondLevelData() override
    {
        SelectorStrategy::testMoveFromSecondLevelData();
    }

    void testModifyInRootData() override
    {
        SelectorStrategy::testModifyInRootData();
    }
    void testModifyInTopLevelData() override
    {
        SelectorStrategy::testModifyInTopLevelData();
    }
    void testModifyInSecondLevelData() override
    {
        SelectorStrategy::testModifyInSecondLevelData();
    }
};

/**
  @brief Selects only one item in the source model which does not change
*/
template<int num1 = 0, int num2 = 0, int num3 = 0, int num4 = 0, int num5 = 0>
class ImmediateSelectionStrategy : public ModelSelector
{
public:
    ImmediateSelectionStrategy(ProxyModelTest *parent = nullptr)
        : ModelSelector(parent)
    {
        if (num1 > 0) {
            m_selectedRows << num1;
        }
        if (num2 > 0) {
            m_selectedRows << num2;
        }
        if (num3 > 0) {
            m_selectedRows << num3;
        }
        if (num4 > 0) {
            m_selectedRows << num4;
        }
        if (num5 > 0) {
            m_selectedRows << num5;
        }
    }
};

/**
  For testing the proxy when it has no selection.
*/
typedef ImmediateSelectionStrategy<0, 0, 0, 0, 0> NoSelectionStrategy;

/* clang-format off */

#define SELECTIONPROXYTESTDATA(SelectionStrategy, num1, num2, num3, num4, num5, Type) \
    new TestData<SelectionStrategy<num1, num2, num3, num4, num5>, Type>(proxyModelTestClass)

#define SELECTIONPROXYTESTCONFIG(SelectionStrategy, num1, num2, num3, num4, num5, Type) \
    << "  FilterBehavior:    " << #Type << "\n" \
    << "  SelectionStrategy: " << #SelectionStrategy << (num1 > 0 ? #num1 : "") << (num2 > 0 ? #num2 : "") << (num3 > 0 ? #num3 : "") \
    << (num4 > 0 ? #num4 : "") << (num5 > 0 ? #num5 : "") << "\n";

#define SELECTIONPROXYMODELTEST(SelectionStrategy, num1, num2, num3, num4, num5, Type, IntermediateProxy, LazyPersistence) \
    PROXYMODELTEST_CUSTOM(SELECTIONPROXYTESTDATA(SelectionStrategy, num1, num2, num3, num4, num5, Type), \
                          IntermediateProxy, \
                          LazyPersistence, \
                          SELECTIONPROXYTESTCONFIG(SelectionStrategy, num1, num2, num3, num4, num5, Type))

#define SELECTIONCOMPLETETEST(SelectionStrategy, num1, num2, num3, num4, num5, Type) \
    SELECTIONPROXYMODELTEST(SelectionStrategy, num1, num2, num3, num4, num5, Type, DynamicTree, ImmediatePersistence) \
    SELECTIONPROXYMODELTEST(SelectionStrategy, num1, num2, num3, num4, num5, Type, IntermediateProxy, ImmediatePersistence) \
    SELECTIONPROXYMODELTEST(SelectionStrategy, num1, num2, num3, num4, num5, Type, DynamicTree, LazyPersistence) \
    SELECTIONPROXYMODELTEST(SelectionStrategy, num1, num2, num3, num4, num5, Type, IntermediateProxy, LazyPersistence)

#define SIMPLETESTDATA(SelectionStrategy, Type) new TestData<SelectionStrategy, Type>(proxyModelTestClass)

#define SELECTIONPROXYMODELSIMPLETEST(SelectionStrategy, Type) \
    PROXYMODELTEST_CUSTOM(SIMPLETESTDATA(SelectionStrategy, Type), \
                          DynamicTree, \
                          LazyPersistence, \
                          SELECTIONPROXYTESTCONFIG(SelectionStrategy, 0, 0, 0, 0, 0, Type))

#define SELECTIONCOMPLETETEST1(SelectionStrategy, num1, Type) SELECTIONCOMPLETETEST(SelectionStrategy, num1, 0, 0, 0, 0, Type)

#define SELECTIONCOMPLETETEST2(SelectionStrategy, num1, num2, Type) SELECTIONCOMPLETETEST(SelectionStrategy, num1, num2, 0, 0, 0, Type)

#define SELECTIONCOMPLETETEST3(SelectionStrategy, num1, num2, num3, Type) SELECTIONCOMPLETETEST(SelectionStrategy, num1, num2, num3, 0, 0, Type)

#define SELECTIONCOMPLETETEST4(SelectionStrategy, num1, num2, num3, num4, Type) SELECTIONCOMPLETETEST(SelectionStrategy, num1, num2, num3, num4, 0, Type)

#endif
/* clang-format on */
