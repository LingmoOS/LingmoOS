// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "model/system_service_table_model.h"
#include "service/service_manager.h"
#include "common/common.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <DApplication>

DWIDGET_USE_NAMESPACE

static QString m_Sresult;
/***************************************STUB begin*********************************************/
int stub_svc_data_row(){
    return 0;
}

bool stub_svc_data_isValid(){
    return true;
}

int stub_svc_data_column1(){
    m_Sresult = "kSystemServiceNameColumn";
    return SystemServiceTableModel::kSystemServiceNameColumn;
}

int stub_svc_data_column2(){
    m_Sresult = "kSystemServiceLoadStateColumn";
    return SystemServiceTableModel::kSystemServiceLoadStateColumn;
}
int stub_svc_data_column3(){
    m_Sresult = "kSystemServiceActiveStateColumn";
    return SystemServiceTableModel::kSystemServiceActiveStateColumn;
}

int stub_svc_data_column4(){
    m_Sresult = "kSystemServiceSubStateColumn";
    return SystemServiceTableModel::kSystemServiceSubStateColumn;
}

int stub_svc_data_column5(){
    m_Sresult = "kSystemServiceStateColumn";
    return SystemServiceTableModel::kSystemServiceStateColumn;
}

int stub_svc_data_column6(){
    m_Sresult = "kSystemServiceStartupModeColumn";
    return SystemServiceTableModel::kSystemServiceStartupModeColumn;
}

int stub_svc_data_column7(){
    m_Sresult = "kSystemServiceDescriptionColumn";
    return SystemServiceTableModel::kSystemServiceDescriptionColumn;
}

int stub_svc_data_column8(){
    m_Sresult = "kSystemServicePIDColumn";
    return SystemServiceTableModel::kSystemServicePIDColumn;
}
/***************************************STUB end**********************************************/
class UT_SystemServiceTableModel: public ::testing::Test
{
public:
    UT_SystemServiceTableModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new SystemServiceTableModel();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    SystemServiceTableModel *m_tester;
};

TEST_F(UT_SystemServiceTableModel, initTest)
{

}

TEST_F(UT_SystemServiceTableModel, test_getUnitFileName_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->getUnitFileName(*index);
    delete index;

}

TEST_F(UT_SystemServiceTableModel, test_getUnitActiveState_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->getUnitActiveState(*index);
    delete index;

}

TEST_F(UT_SystemServiceTableModel, test_getUnitFileState_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->getUnitFileState(*index);
    delete index;

}

TEST_F(UT_SystemServiceTableModel, test_updateServiceEntry_001)
{
    SystemServiceEntry *entry = new SystemServiceEntry();
    entry->setSName("aaa");
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->updateServiceEntry(*entry);
    delete entry;
}

TEST_F(UT_SystemServiceTableModel, test_updateServiceEntry_002)
{
    SystemServiceEntry *entry = new SystemServiceEntry();
    entry->setSName("aaa");
    m_tester->updateServiceEntry(*entry);
    delete entry;
}

TEST_F(UT_SystemServiceTableModel, test_updateServiceEntry_003)
{
    SystemServiceEntry *entry = new SystemServiceEntry();
    m_tester->updateServiceEntry(*entry);
    delete entry;
}

TEST_F(UT_SystemServiceTableModel, test_data_001)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);

    m_tester->data(*index,role);
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_002)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column1);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_003)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column2);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_004)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column3);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_005)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column4);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_006)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column5);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_007)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column6);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_008)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column7);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_009)
{
    int role = Qt::DisplayRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column8);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_data_010)
{
    int role = Qt::TextAlignmentRole;
    QModelIndex *index = new QModelIndex;
    SystemServiceEntry *entry = new SystemServiceEntry();

    Stub b1;
    b1.set(ADDR(QModelIndex,row),stub_svc_data_row);
    Stub b2;
    b2.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    Stub b3;
    b3.set(ADDR(QModelIndex,column),stub_svc_data_column8);
    m_tester->m_svcList << "aaa";
    m_tester->m_svcMap.insert("aaa",*entry);
    QVariant expect = m_tester->data(*index,role);


    delete entry;
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_rowCount_001)
{
    QModelIndex *index = new QModelIndex;
    m_tester->rowCount(*index);
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_rowCount_002)
{
    QModelIndex *index = new QModelIndex;
    Stub b;
    b.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    int expect = m_tester->rowCount(*index);

    EXPECT_EQ(expect,0);
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_columnCount_001)
{
    QModelIndex *index = new QModelIndex;
    int expect = m_tester->columnCount(*index);
    EXPECT_EQ(expect,SystemServiceTableModel::kSystemServiceTableColumnCount);
    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_columnCount_002)
{
    QModelIndex *index = new QModelIndex;
    Stub b;
    b.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
    int expect = m_tester->columnCount(*index);

    delete index;
}

TEST_F(UT_SystemServiceTableModel, test_headerData_001)
{
    int section = SystemServiceTableModel::kSystemServiceNameColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceName));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_002)
{
    int section = SystemServiceTableModel::kSystemServiceLoadStateColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceLoadState));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_003)
{
    int section = SystemServiceTableModel::kSystemServiceActiveStateColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceActiveState));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_004)
{
    int section = SystemServiceTableModel::kSystemServiceSubStateColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceSubState));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_005)
{
    int section = SystemServiceTableModel::kSystemServiceStateColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceState));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_006)
{
    int section = SystemServiceTableModel::kSystemServiceDescriptionColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceDescription));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_007)
{
    int section = SystemServiceTableModel::kSystemServicePIDColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServicePID));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_008)
{
    int section = SystemServiceTableModel::kSystemServiceStartupModeColumn;
    int role = Qt::DisplayRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,DApplication::translate("Service.Table.Header", kSystemServiceStartupMode));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_009)
{
    int section = SystemServiceTableModel::kSystemServiceStartupModeColumn;
    int role = Qt::TextAlignmentRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,QVariant(Qt::AlignLeft | Qt::AlignVCenter));
}

TEST_F(UT_SystemServiceTableModel, test_headerData_010)
{
    int section = SystemServiceTableModel::kSystemServiceStartupModeColumn;
    int role = Qt::InitialSortOrderRole;
    Qt::Orientation orientation = Qt::Horizontal;
    QVariant expect = m_tester->headerData(section,orientation,role);
    EXPECT_EQ(expect,QVariant::fromValue(Qt::DescendingOrder));
}

TEST_F(UT_SystemServiceTableModel, test_flags_001)
{
     Stub b1;
     b1.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
     QModelIndex *index = new QModelIndex();
     m_tester->flags(*index);

     delete index;
}

TEST_F(UT_SystemServiceTableModel, test_flags_002)
{

     QModelIndex *index = new QModelIndex();
     m_tester->flags(*index);

     delete index;
}

TEST_F(UT_SystemServiceTableModel, test_fetchMore_001)
{
     Stub b1;
     b1.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
     QModelIndex *index = new QModelIndex();
     m_tester->fetchMore(*index);
     delete index;
}

TEST_F(UT_SystemServiceTableModel, test_fetchMore_002)
{

     QModelIndex *index = new QModelIndex();
     m_tester->fetchMore(*index);

     delete index;
}

TEST_F(UT_SystemServiceTableModel, test_canFetchMore_001)
{
     Stub b1;
     b1.set(ADDR(QModelIndex,isValid),stub_svc_data_isValid);
     QModelIndex *index = new QModelIndex();
     m_tester->canFetchMore(*index);
     delete index;
}

TEST_F(UT_SystemServiceTableModel, test_canFetchMore_002)
{

     QModelIndex *index = new QModelIndex();
     m_tester->canFetchMore(*index);

     delete index;
}

TEST_F(UT_SystemServiceTableModel, test_updateServiceList_001)
{
    QList<SystemServiceEntry> List {};
    m_tester->updateServiceList(List);
}
