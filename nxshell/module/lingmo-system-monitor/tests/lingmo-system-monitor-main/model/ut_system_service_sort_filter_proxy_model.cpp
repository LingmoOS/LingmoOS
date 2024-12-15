// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/system_service_sort_filter_proxy_model.h"
#include "model/system_service_table_model.h"
#include "common/common.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
/***************************************STUB begin*********************************************/
int stub_lessThan_sortColumn1(){
    return SystemServiceTableModel::kSystemServicePIDColumn;
}

int stub_lessThan_sortColumn2(){
    return SystemServiceTableModel::kSystemServiceNameColumn;
}

int stub_lessThan_sortColumn3(){
    return SystemServiceTableModel::kSystemServiceDescriptionColumn;
}
/***************************************STUB end**********************************************/
class UT_SystemServiceSortFilterProxyModel: public ::testing::Test
{
public:
    UT_SystemServiceSortFilterProxyModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SystemServiceSortFilterProxyModel();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemServiceSortFilterProxyModel *m_tester;
};

TEST_F(UT_SystemServiceSortFilterProxyModel, initTest)
{

}

TEST_F(UT_SystemServiceSortFilterProxyModel, test_canFetchMore_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->canFetchMore(*index);
    delete index;
}

TEST_F(UT_SystemServiceSortFilterProxyModel, test_fetchMore_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->fetchMore(*index);
    delete index;
}

TEST_F(UT_SystemServiceSortFilterProxyModel, test_filterAcceptsColumn_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->filterAcceptsColumn(SystemServiceTableModel::kSystemServiceNameColumn,*index);
    delete index;
}

TEST_F(UT_SystemServiceSortFilterProxyModel, test_lessThan_001)
{
    Stub b;
    b.set(ADDR(QSortFilterProxyModel,sortColumn),stub_lessThan_sortColumn1);
    QModelIndex *lindex = new QModelIndex;
    QModelIndex *rindex = new QModelIndex;

    m_tester->lessThan(*lindex,*rindex);

    delete rindex;
    delete lindex;
}

TEST_F(UT_SystemServiceSortFilterProxyModel, test_lessThan_002)
{
    Stub b;
    b.set(ADDR(QSortFilterProxyModel,sortColumn),stub_lessThan_sortColumn2);
    QModelIndex *lindex = new QModelIndex;
    QModelIndex *rindex = new QModelIndex;

    m_tester->lessThan(*lindex,*rindex);

    delete rindex;
    delete lindex;
}

TEST_F(UT_SystemServiceSortFilterProxyModel, test_lessThan_003)
{
    Stub b;
    b.set(ADDR(QSortFilterProxyModel,sortColumn),stub_lessThan_sortColumn3);
    QModelIndex *lindex = new QModelIndex;
    QModelIndex *rindex = new QModelIndex;

    m_tester->lessThan(*lindex,*rindex);

    delete rindex;
    delete lindex;
}
