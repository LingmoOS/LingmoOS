/*
    SPDX-FileCopyrightText: 2009 Stephen Kelly <steveire@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef MODELSELECTOR_H
#define MODELSELECTOR_H

#include <QItemSelectionModel>

//#include <kselectionproxymodel.h>

#include "proxymodeltest.h"

#include "proxymodeltestsuite_export.h"

class ModelSpy;

class OnlySelectedChildrenTest;

class PROXYMODELTESTSUITE_EXPORT ModelSelector : public ProxyModelTestData
{
    Q_OBJECT
public:
    ModelSelector(ProxyModelTest *ProxyModelTest = nullptr);

    void setWatchedModel(QAbstractItemModel *model);

    void setSelectionModel(QItemSelectionModel *selectionModel);

    void setRootModel(DynamicTreeModel *rootModel);

    QItemSelectionModel *selectionModel() const
    {
        return m_selectionModel;
    }
    QAbstractItemModel *watchedModel()
    {
        return m_model;
    }

    void setWatch(bool watch);

    // virtual KSelectionProxyModel::FilterBehavior filterBehaviour() = 0;

public Q_SLOTS:
    void rowsInserted(const QModelIndex &parent, int start, int end);

    void testInsertWhenEmptyData() override
    {
        dummyTestData();
    }

    void testInsertInRootData() override
    {
        dummyTestData();
    }

    void testInsertInTopLevelData() override
    {
        dummyTestData();
    }

    void testInsertInSecondLevelData() override
    {
        dummyTestData();
    }

    void testRemoveFromRootData() override
    {
        dummyTestData();
    }

    void testRemoveFromTopLevelData() override
    {
        dummyTestData();
    }

    void testRemoveFromSecondLevelData() override
    {
        dummyTestData();
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

protected Q_SLOTS:
    void modelDestroyed()
    {
        m_model = nullptr;
        m_selectionModel = nullptr;
    }

protected:
    QAbstractItemModel *m_model;
    QItemSelectionModel *m_selectionModel;
    DynamicTreeModel *m_rootModel;
    ModelSpy *m_modelSpy;

    QList<int> m_selectedRows;
};

#endif
