// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//Self
#include "block_dev_summary_view_widget.h"
#include "system/block_device.h"
#include "common/common.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>
#include "addr_pri.h"

//Qt
#include <QSignalSpy>
#include <QDebug>
#include <QApplication>
#include <DApplication>
#include <DApplicationHelper>
using namespace core::system;
using namespace common::format;
/***************************************STUB begin*********************************************/
bool stub_data_isValid_true()
{
    return true;
}

int stub_data_index_row_0()
{
    return 0;
}

int stub_data_index_row_1()
{
    return 1;
}

int stub_data_index_row_2()
{
    return 2;
}

int stub_data_index_row_3()
{
    return 3;
}

int stub_data_index_row_4()
{
    return 4;
}

int stub_data_index_row_5()
{
    return 5;
}

int stub_data_index_row_6()
{
    return 6;
}

int stub_data_index_column_0()
{
    return 0;
}

int stub_data_index_column_1()
{
    return 1;
}
/***************************************STUB end**********************************************/

class UT_BlockDevSummaryViewWidget : public ::testing::Test
{
public:
    UT_BlockDevSummaryViewWidget() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BlockDevSummaryViewWidget();
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    BlockDevSummaryViewWidget *m_tester;
};

TEST_F(UT_BlockDevSummaryViewWidget, initTest)
{

}


TEST_F(UT_BlockDevSummaryViewWidget, test_fontChanged_01)
{
    QFont font;
    font.setBold(true);
    m_tester->fontChanged(font);

    EXPECT_EQ(m_tester->m_font.bold(), font.bold());
    EXPECT_EQ(m_tester->height(), 260);
}

TEST_F(UT_BlockDevSummaryViewWidget, test_chageSummaryInfo_01)
{
    QString deviceName = "";
    m_tester->chageSummaryInfo(deviceName);
}

TEST_F(UT_BlockDevSummaryViewWidget, test_paintEvent_01)
{
    EXPECT_FALSE(m_tester->grab().isNull());
}

TEST_F(UT_BlockDevSummaryViewWidget, test_rowCount_01)
{
    EXPECT_EQ(m_tester->model()->rowCount(), 7);
}

TEST_F(UT_BlockDevSummaryViewWidget, test_columnCount_01)
{
    EXPECT_EQ(m_tester->model()->columnCount(), 2);
}

TEST_F(UT_BlockDevSummaryViewWidget, test_data_01)
{
    QModelIndex index =  m_tester->model()->index(0, 0);

    m_tester->model()->data(index, Qt::DisplayRole);
}

TEST_F(UT_BlockDevSummaryViewWidget, test_flags_01)
{
    QModelIndex index =  m_tester->model()->index(0, 0);
    EXPECT_EQ(m_tester->model()->flags(index), Qt::NoItemFlags);
}


////////////////////////////////////CLASS 2........////////////////////////////////////////
class DeailTableModelBlock : public QAbstractTableModel
{
public:
    explicit DeailTableModelBlock(QObject *parent = nullptr);
    virtual ~DeailTableModelBlock();
protected:
    int rowCount(const QModelIndex &) const
    {
        return 7;
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const
    {

        if (!index.isValid() || currDeciveName.isEmpty())
            return QVariant();
        int row = index.row();
        int column = index.column();
        if (role == Qt::DisplayRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Model");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Read speed");
                break;
            case 1:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Write speed");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Capacity");
                break;
            case 2:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors read");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Reads/s");
                break;
            case 3:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors read/s");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Reads merged/s");
                break;
            case 4:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors written");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Writes completed ");
                break;
            case 5:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Writes merged");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Writes/s");
                break;
            case 6:
                if (column == 0)
                    return QApplication::translate("DeailTableModelBlock", "Sectors written/s");
                else if (column == 1)
                    return QApplication::translate("DeailTableModelBlock", "Writes merged/s");
                break;

            }
        } else if (role == Qt::UserRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return m_blockInfo.model();
                else if (column == 1)
                    return formatUnit_memory_disk(m_blockInfo.readSpeed(), B, 1);
                break;
            case 1:
                if (column == 0)
                    return formatUnit_memory_disk(m_blockInfo.writeSpeed(), B, 1);
                else if (column == 1)
                    return formatUnit_memory_disk(m_blockInfo.capacity(), B, 1);
                break;
            case  2:
                if (column == 0)
                    return m_blockInfo.blocksRead();
                else if (column == 1)
                    return m_blockInfo.readRequestIssuedPerSecond();
                break;
            case  3:
                if (column == 0)
                    return m_blockInfo.sectorsReadPerSecond();
                else if (column == 1)
                    return m_blockInfo.readRequestMergedPerSecond();
                break;
            case  4:
                if (column == 0)
                    return m_blockInfo.blocksWritten();
                else if (column == 1)
                    return m_blockInfo.writeComplete();
                break;
            case  5:
                if (column == 0)
                    return m_blockInfo.writeMerged();
                else if (column == 1)
                    return m_blockInfo.writeRequestIssuedPerSecond();
                break;
            case  6:
                if (column == 0)
                    return m_blockInfo.sectorsWrittenPerSecond();
                else if (column == 1)
                    return m_blockInfo.writeRequestMergedPerSecond();
                break;
            }
        } else if (role == Qt::TextColorRole) {
            const auto &palette = DApplicationHelper::instance()->applicationPalette();
            return palette.color(DPalette::Text);
        }
        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }

public slots:
    void updateModel();

public:
    void setCurrentName(const QString &str)
    {
        currDeciveName = str;
        if (!str.isEmpty())
            m_blockInfo = m_mapInfo.find(currDeciveName).value();
        updateModel();
    }

private:
    QString currDeciveName;
    BlockDevice m_blockInfo;
    QMap<QString, BlockDevice> m_mapInfo;
};


class UT_DeailTableModelBlock : public ::testing::Test
{
public:
    UT_DeailTableModelBlock() : m_tester2(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester2 = new DeailTableModelBlock();
    }

    virtual void TearDown()
    {
        if (m_tester2) {
            delete m_tester2;
            m_tester2 = nullptr;
        }
    }

protected:
    DeailTableModelBlock *m_tester2;
};

TEST_F(UT_DeailTableModelBlock, initTest)
{

}

TEST_F(UT_DeailTableModelBlock, test_rowCount_01)
{
    QModelIndex index;
    EXPECT_EQ(m_tester2->rowCount(index), 7);
}

TEST_F(UT_DeailTableModelBlock, test_columnCount_01)
{
    QModelIndex index;
    EXPECT_EQ(m_tester2->columnCount(index), 2);
}


TEST_F(UT_DeailTableModelBlock, test_data_01)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}


TEST_F(UT_DeailTableModelBlock, test_data_02)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_03)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}


TEST_F(UT_DeailTableModelBlock, test_data_04)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_05)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_06)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_07)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_08)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_09)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_10)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_11)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_12)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_13)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_14)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::DisplayRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_15)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}


TEST_F(UT_DeailTableModelBlock, test_data_16)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_0);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_17)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}


TEST_F(UT_DeailTableModelBlock, test_data_18)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_1);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_19)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_20)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_2);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_21)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_22)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_3);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_23)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_24)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_4);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_25)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_26)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_5);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_27)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_0);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_28)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::UserRole);
}

TEST_F(UT_DeailTableModelBlock, test_data_29)
{
    Stub stub;
    stub.set(ADDR(QModelIndex, isValid), stub_data_isValid_true);
    stub.set(ADDR(QModelIndex, row), stub_data_index_row_6);
    stub.set(ADDR(QModelIndex, column), stub_data_index_column_1);
    QModelIndex index;
    m_tester2->currDeciveName = "11";
    m_tester2->data(index, Qt::TextColorRole);
}

TEST_F(UT_DeailTableModelBlock, test_flags_01)
{
    QModelIndex index;
    EXPECT_EQ(m_tester2->flags(index), Qt::NoItemFlags);
}
