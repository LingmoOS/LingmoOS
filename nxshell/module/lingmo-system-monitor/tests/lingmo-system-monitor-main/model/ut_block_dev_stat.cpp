// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/block_dev_stat_model.h"
#include "common/time_period.h"
#include "common/sample.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QDebug>
#include <QModelIndex>
using namespace common::core;

static QString m_Sresult;
/***************************************STUB begin*********************************************/
bool stub_data_isValid(){
    m_Sresult = "index is valid";
    return true;
}

int stub_data_row(){

    return 0;
}

int stub_data_count() {
    m_Sresult = "sample count";
    return 1;
}

int stub_data_column1()
{
    m_Sresult = "index column kStatTotalRead";
    return BlockDevStatModel::kStatTotalRead;
}

int stub_data_column2()
{
    m_Sresult = "index column kStatTotalWrite";
    return BlockDevStatModel::kStatTotalWrite;
}

int stub_data_column3()
{
    m_Sresult = "index column kStatReadSpeed";
    return BlockDevStatModel::kStatReadSpeed;
}

int stub_data_column4()
{
    m_Sresult = "index column kStatWriteSpeed";
    return BlockDevStatModel::kStatWriteSpeed;
}

/***************************************STUB end**********************************************/
class UT_BlockDevStatModel: public ::testing::Test
{
public:
    UT_BlockDevStatModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        TimePeriod period(TimePeriod::kNoPeriod, {2, 0});
        m_tester = new BlockDevStatModel(period,nullptr);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDevStatModel *m_tester;
};

TEST_F(UT_BlockDevStatModel, initTest)
{

}

TEST_F(UT_BlockDevStatModel, test_rowCount_001)
{
    int count = m_tester->rowCount();
    EXPECT_EQ(count,0);
}

TEST_F(UT_BlockDevStatModel, test_rowCount_002)
{
    Stub b1;
    b1.set(ADDR(IOSample,count),stub_data_count);
    Stub b2;
    b1.set(ADDR(IOPSSample,count),stub_data_count);
    m_tester->rowCount();
}

TEST_F(UT_BlockDevStatModel, test_columnCount_001)
{
    int count = m_tester->columnCount();
    EXPECT_EQ(count,BlockDevStatModel::kStatPropMax);
}

TEST_F(UT_BlockDevStatModel, test_data_001)
{
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_002)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_003)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column1);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_004)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column2);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_005)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column3);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_006)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);  
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column4);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_007)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column1);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,BlockDevStatModel::kValueRole);
    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_008)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column2);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,BlockDevStatModel::kValueRole);
    delete index;
}


TEST_F(UT_BlockDevStatModel, test_data_009)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column3);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,BlockDevStatModel::kValueRole);

    delete index;
}

TEST_F(UT_BlockDevStatModel, test_data_010)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_data_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_data_row);
    Stub b3;
    b3.set(ADDR(Sample<IOPS>,count),stub_data_count);
    Stub b4;
    b4.set(ADDR(QModelIndex,column),stub_data_column4);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,BlockDevStatModel::kValueRole);

    delete index;
}
