// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/cpu_list_model.h"
#include "model/cpu_stat_model.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QMap>

static QString m_Sresult;
using myMap = QMap<QByteArray, std::shared_ptr<CPUStatModel>> ;
/***************************************STUB begin*********************************************/
bool stub_cpudata_isValid(){
    m_Sresult = "index is valid";
    return true;
}

int stub_cpudata_row(){

    return 1;
}

int stub_cpudata_column1()
{
    m_Sresult = "index column kStatTotalRead";
    return CPUListModel::kPropCPUIndex;
}

int stub_cpudata_column2()
{
    m_Sresult = "index column kPropCPUUsage";
    return CPUListModel::kPropCPUUsage;
}

int stub_cpudata_count(){
    return 1;
}
/***************************************STUB end**********************************************/
class UT_CPUListModel: public ::testing::Test
{
public:
    UT_CPUListModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CPUListModel(nullptr);
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUListModel *m_tester;
};

TEST_F(UT_CPUListModel, initTest)
{

}

TEST_F(UT_CPUListModel, test_rowCount_001)
{
    m_tester->rowCount();

}

TEST_F(UT_CPUListModel, test_columnCount_001)
{
    int count = m_tester->columnCount();
    EXPECT_EQ(count,CPUListModel::kPropCount);

}

TEST_F(UT_CPUListModel, test_data_001)
{
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}


TEST_F(UT_CPUListModel, test_data_002)
{
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_cpudata_isValid);
    QModelIndex *index = new QModelIndex();
    m_tester->data(*index,Qt::DisplayRole);
    delete index;
}

TEST_F(UT_CPUListModel, test_fetchMore_001)
{
    QModelIndex *index = new QModelIndex();
    m_tester->fetchMore(*index);
    delete index;
}

TEST_F(UT_CPUListModel, test_fetchMore_002)
{
    QModelIndex *index = new QModelIndex();
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_cpudata_isValid);
    m_tester->fetchMore(*index);
    delete index;
}

TEST_F(UT_CPUListModel, test_canFetchMore_001)
{
    QModelIndex *index = new QModelIndex();
    m_tester->canFetchMore(*index);
    delete index;
}

TEST_F(UT_CPUListModel, test_canFetchMore_002)
{
    QModelIndex *index = new QModelIndex();
    Stub b1;
    b1.set(ADDR(QModelIndex,isValid),stub_cpudata_isValid);
    m_tester->canFetchMore(*index);
    delete index;
}

TEST_F(UT_CPUListModel, test_statModel_001)
{
    QByteArray cpu="cpu0";
    m_tester->statModel(cpu);

}
