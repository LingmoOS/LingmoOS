// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/cpu_list_sort_filter_proxy_model.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>

/***************************************STUB begin*********************************************/
QModelIndex* stub_sourceModel()
{
    static QModelIndex index;
    return &index;
}
/***************************************STUB end**********************************************/

class UT_CPUListSortFilterProxyModel: public ::testing::Test
{
public:
    UT_CPUListSortFilterProxyModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CPUListSortFilterProxyModel(nullptr);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUListSortFilterProxyModel *m_tester;
};

TEST_F(UT_CPUListSortFilterProxyModel, initTest)
{

}

TEST_F(UT_CPUListSortFilterProxyModel, test_setSortKey_001)
{
    CPUListSortFilterProxyModel::SortKey key = CPUListSortFilterProxyModel::kSortByIndex;
    m_tester->setSortKey(key);
    EXPECT_EQ(m_tester->m_sortKey,key);

}

TEST_F(UT_CPUListSortFilterProxyModel, test_filterAcceptsRow_001)
{
    static QModelIndex index;
    static QSortFilterProxyModel model;
    m_tester->setSourceModel(&model);
    m_tester->filterAcceptsRow(0, index);
}

TEST_F(UT_CPUListSortFilterProxyModel, test_filterAcceptsColumn_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->filterAcceptsColumn(1,*index);
    delete index;

}

TEST_F(UT_CPUListSortFilterProxyModel, test_lessThan_001)
{
    static QModelIndex index1;
    static QModelIndex index2;
    static QSortFilterProxyModel model;
    m_tester->setSourceModel(&model);
    m_tester->m_sortKey = CPUListSortFilterProxyModel::SortKey::kSortByIndex;
    m_tester->lessThan(index1, index2);
}

TEST_F(UT_CPUListSortFilterProxyModel, test_lessThan_002)
{
    static QModelIndex index1;
    static QModelIndex index2;
    static QSortFilterProxyModel model;
    m_tester->setSourceModel(&model);
    m_tester->m_sortKey = CPUListSortFilterProxyModel::SortKey::kSortByUsage;
    m_tester->lessThan(index1, index2);
}

TEST_F(UT_CPUListSortFilterProxyModel, test_lessThan_003)
{
    static QModelIndex index1;
    static QModelIndex index2;
    static QSortFilterProxyModel model;
    m_tester->setSourceModel(&model);
    m_tester->m_sortKey = CPUListSortFilterProxyModel::SortKey::kSortKeyMax;
    m_tester->lessThan(index1, index2);
}




