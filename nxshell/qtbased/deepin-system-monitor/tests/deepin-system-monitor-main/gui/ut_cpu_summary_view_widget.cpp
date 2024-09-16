// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "cpu_summary_view_widget.h"
#include "model/cpu_info_model.h"
#include "system/cpu_set.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QApplication>
#include <DApplicationHelper>

/***************************************STUB begin*********************************************/
bool stub_CPUSummaryTableModel_data_isValid_true()
{
    return true;
}

int stub_CPUSummaryTableModel_data_index_row_0()
{
    return 0;
}

int stub_CPUSummaryTableModel_data_index_row_1()
{
    return 1;
}

int stub_CPUSummaryTableModel_data_index_row_2()
{
    return 2;
}

int stub_CPUSummaryTableModel_data_index_row_3()
{
    return 3;
}

int stub_CPUSummaryTableModel_data_index_row_4()
{
    return 4;
}

int stub_CPUSummaryTableModel_data_index_row_5()
{
    return 5;
}

int stub_CPUSummaryTableModel_data_index_row_6()
{
    return 6;
}

int stub_CPUSummaryTableModel_data_index_row_7()
{
    return 7;
}

int stub_CPUSummaryTableModel_data_index_row_8()
{
    return 8;
}

int stub_CPUSummaryTableModel_data_index_column_0()
{
    return 0;
}

int stub_CPUSummaryTableModel_data_index_column_1()
{
    return 1;
}
/***************************************STUB end**********************************************/

class UT_CPUDetailSummaryTable : public ::testing::Test
{
public:
    UT_CPUDetailSummaryTable() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static CPUInfoModel model;
        m_tester = new CPUDetailSummaryTable(&model, nullptr);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUDetailSummaryTable *m_tester;
};

TEST_F(UT_CPUDetailSummaryTable, initTest)
{

}

TEST_F(UT_CPUDetailSummaryTable, test_fontChanged_01)
{
    QFont font;
    font.setBold(true);
    m_tester->fontChanged(font);

    EXPECT_EQ(m_tester->m_font.bold(), font.bold());
}

TEST_F(UT_CPUDetailSummaryTable, test_onModelUpdate_01)
{
    m_tester->onModelUpdate();
}

TEST_F(UT_CPUDetailSummaryTable, test_paintEvent_01)
{
    EXPECT_FALSE(m_tester->grab().isNull());
}

class UT_CPUSummaryTableModel : public ::testing::Test
{
public:
    UT_CPUSummaryTableModel() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        static CPUInfoModel model;
        m_tester = new CPUSummaryTableModel(&model);
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    CPUSummaryTableModel *m_tester;
};

TEST_F(UT_CPUSummaryTableModel, initTest)
{

}


TEST_F(UT_CPUSummaryTableModel, test_data_01)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}


TEST_F(UT_CPUSummaryTableModel, test_data_02)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_03)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}


TEST_F(UT_CPUSummaryTableModel, test_data_04)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_05)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_06)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_07)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_08)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_09)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_10)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_11)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_12)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_13)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_14)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::DisplayRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_15)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}


TEST_F(UT_CPUSummaryTableModel, test_data_16)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_17)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}


TEST_F(UT_CPUSummaryTableModel, test_data_18)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_19)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_20)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_21)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_22)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_23)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_24)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_25)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_26)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_27)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_28)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_29)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::TextColorRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_30)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_7);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::TextColorRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_31)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_7);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::TextColorRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_32)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_8);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::TextColorRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_33)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_8);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::TextColorRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_34)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_7);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_35)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_7);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_36)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_8);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_1);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}

TEST_F(UT_CPUSummaryTableModel, test_data_37)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_CPUSummaryTableModel_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_CPUSummaryTableModel_data_index_row_8);
    stub.set(ADDR(QModelIndex, column), stub_CPUSummaryTableModel_data_index_column_0);
    QModelIndex index;
    m_tester->data(index, Qt::UserRole);
}
