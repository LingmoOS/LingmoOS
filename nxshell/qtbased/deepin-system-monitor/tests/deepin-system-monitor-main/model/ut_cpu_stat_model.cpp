// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/cpu_stat_model.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt

static QString m_Sresult;
/***************************************STUB begin*********************************************/
bool stub_cpustatdata_isValid(){
    m_Sresult = "index is valid";
    return true;
}

int stub_cpustatdata_row(){

    return 0;
}

int stub_cpustatdata_column1()
{
    m_Sresult = "index column kStatTotalRead";
    return CPUStatModel::kStatUserTime;
}

int stub_cpustatdata_column2()
{
    m_Sresult = "index column kPropCPUUsage";
    return CPUStatModel::kStatNiceTime;
}

int stub_cpustatdata_count(){
    return 1;
}

/***************************************STUB end**********************************************/
class UT_CPUStatModel: public ::testing::Test
{
public:
    UT_CPUStatModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        TimePeriod time {};
        m_tester = new CPUStatModel(time);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUStatModel *m_tester;
};

TEST_F(UT_CPUStatModel, initTest)
{

}

TEST_F(UT_CPUStatModel, test_rowCount_001)
{
    m_tester->rowCount();

}

TEST_F(UT_CPUStatModel, test_columnCount_001)
{
    int expect = m_tester->columnCount();
    EXPECT_EQ(expect,CPUStatModel::kStatPropMax);
}

TEST_F(UT_CPUStatModel, test_data_001)
{
    QModelIndex *index = new QModelIndex;
    int role = Qt::DisplayRole;

    m_tester->data(*index,role);

    delete index;

}

TEST_F(UT_CPUStatModel, test_data_002)
{
    QModelIndex *index = new QModelIndex;
    int role = Qt::DisplayRole;

    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_cpustatdata_isValid);

    m_tester->data(*index,role);
    delete index;

}

TEST_F(UT_CPUStatModel, test_data_003)
{
    QModelIndex *index = new QModelIndex;
    int role = Qt::DisplayRole;

    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_cpustatdata_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_cpustatdata_row);
    Stub b3;
    b3.set(ADDR(Sample<cpu_usage_t>,count),stub_cpustatdata_count);

    m_tester->data(*index,role);
    delete index;

}



TEST_F(UT_CPUStatModel, test_data_004)
{
    QModelIndex *index = new QModelIndex;
    int role = Qt::DisplayRole;

    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_cpustatdata_isValid);
    Stub b2;
    b2.set(ADDR(QModelIndex,row),stub_cpustatdata_row);
    Stub b3;
    b3.set(ADDR(Sample<cpu_usage_t>,count),stub_cpustatdata_count);

    m_tester->data(*index,role);
    delete index;

}

