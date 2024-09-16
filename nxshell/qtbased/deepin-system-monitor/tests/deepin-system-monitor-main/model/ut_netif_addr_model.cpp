// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/netif_addr_model.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>

static QString m_Sresult;

/***************************************STUB begin*********************************************/
bool stub_net_data_isValid(){
    m_Sresult = "index is valid";
    return true;
}

int stub_net_data_column1()
{
    m_Sresult = "index column kINet4Addr";
    return NetifAddrModel::kINet4Addr;
}

int stub_net_data_column2()
{
    m_Sresult = "index column kINet6Addr";
    return NetifAddrModel::kINet6Addr;
}

int stub_net_data_column3()
{
    m_Sresult = "index column kOtherAddr";
    return NetifAddrModel::kOtherAddr;
}

int stub_net_data_column4()
{

    return 1000;
}

int stub_net_data_row(){

    return 1000;
}
/***************************************STUB end**********************************************/

class UT_NetifAddrModel: public ::testing::Test
{
public:
    UT_NetifAddrModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new NetifAddrModel();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    NetifAddrModel *m_tester;
};

TEST_F(UT_NetifAddrModel, initTest)
{

}

TEST_F(UT_NetifAddrModel, test_rowCount_001)
{
    m_tester->rowCount();
}

TEST_F(UT_NetifAddrModel, test_columnCount_001)
{
    m_tester->columnCount();
}

TEST_F(UT_NetifAddrModel, test_data_001)
{
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_NetifAddrModel, test_data_002)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,column),stub_net_data_column1);
    Stub b2;
    b1.set(ADDR(QModelIndex,isValid),stub_net_data_isValid);
    Stub b3;
    b1.set(ADDR(QModelIndex,row),stub_net_data_row);

    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_NetifAddrModel, test_data_003)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,column),stub_net_data_column2);
    Stub b2;
    b1.set(ADDR(QModelIndex,isValid),stub_net_data_isValid);
    Stub b3;
    b1.set(ADDR(QModelIndex,row),stub_net_data_row);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_NetifAddrModel, test_data_004)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,column),stub_net_data_column3);
    Stub b2;
    b1.set(ADDR(QModelIndex,isValid),stub_net_data_isValid);
    Stub b3;
    b1.set(ADDR(QModelIndex,row),stub_net_data_row);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_NetifAddrModel, test_data_005)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,column),stub_net_data_column4);
    Stub b2;
    b1.set(ADDR(QModelIndex,isValid),stub_net_data_isValid);
    Stub b3;
    b1.set(ADDR(QModelIndex,row),stub_net_data_row);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_NetifAddrModel, test_inet4AddrCount_001)
{
    int expect = m_tester->inet4AddrCount();
    EXPECT_EQ(expect,m_tester->m_addr4DB.size());
}

TEST_F(UT_NetifAddrModel, test_inet6AddrCount_001)
{
    int expect = m_tester->inet4AddrCount();
    EXPECT_EQ(expect,m_tester->m_addr6DB.size());
}

TEST_F(UT_NetifAddrModel, test_otherAddrCount_001)
{
    int expect = m_tester->inet4AddrCount();
    EXPECT_EQ(expect,m_tester->m_addrDB.size());
}
