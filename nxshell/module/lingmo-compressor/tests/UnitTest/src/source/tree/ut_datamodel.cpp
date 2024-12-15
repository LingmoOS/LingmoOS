// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "datamodel.h"
#include "uistruct.h"
#include "mimetypes.h"
#include "mimetypedisplaymanager.h"
#include "uitools.h"

#include "gtest/src/stub.h"

#include <QItemSelection>

#include <gtest/gtest.h>

/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


class UT_DataModel : public ::testing::Test
{
public:
    UT_DataModel(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new DataModel;
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    DataModel *m_tester;
};

TEST_F(UT_DataModel, initTest)
{

}

TEST_F(UT_DataModel, test_headerData_001)
{
    EXPECT_EQ(m_tester->headerData(0, Qt::Horizontal, Qt::DisplayRole), QObject::tr("Name"));
}

TEST_F(UT_DataModel, test_headerData_002)
{
    EXPECT_EQ(m_tester->headerData(0, Qt::Horizontal, Qt::TextAlignmentRole), QVariant(Qt::AlignLeft | Qt::AlignVCenter));
}

TEST_F(UT_DataModel, test_headerData_003)
{
    EXPECT_EQ(m_tester->headerData(0, Qt::Horizontal, Qt::UserRole), QVariant());
}

TEST_F(UT_DataModel, test_data_001)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Name);
    EXPECT_EQ(m_tester->data(index, Qt::DisplayRole), m_tester->m_listEntry[0].strFileName);
}

TEST_F(UT_DataModel, test_data_002)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Time);
    EXPECT_EQ(m_tester->data(index, Qt::DisplayRole), QDateTime::fromTime_t(m_tester->m_listEntry[0].uLastModifiedTime).toString("yyyy/MM/dd hh:mm:ss"));
}

TEST_F(UT_DataModel, test_data_003)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Type);
    CustomMimeType mimetype = determineMimeType(m_tester->m_listEntry[0].strFullPath);  // 根据全路径获取类型
    MimeTypeDisplayManager *pMimetype = new MimeTypeDisplayManager(m_tester);
    EXPECT_EQ(m_tester->data(index, Qt::DisplayRole), pMimetype->displayName(mimetype.name()));
}

TEST_F(UT_DataModel, test_data_004)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Size);
    EXPECT_EQ(m_tester->data(index, Qt::DisplayRole), UiTools::humanReadableSize(m_tester->m_listEntry[0].qSize, 1));
}

TEST_F(UT_DataModel, test_data_005)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(1, DC_Size);
    QString str = m_tester->data(index, Qt::DisplayRole).toString();
    EXPECT_EQ(m_tester->data(index, Qt::DisplayRole), QString::number(m_tester->m_listEntry[1].qSize) + " " + QObject::tr("item(s)"));
}

TEST_F(UT_DataModel, test_data_006)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Name);
    ASSERT_EQ(m_tester->data(index, Qt::UserRole).value<FileEntry>().strFileName, "1.txt");
}

TEST_F(UT_DataModel, test_data_007)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Name);
    EXPECT_EQ(m_tester->data(index, Qt::DecorationRole).isValid(), true);
}

TEST_F(UT_DataModel, test_data_008)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Time);
    EXPECT_EQ(m_tester->data(index, Qt::DecorationRole), QVariant());
}

TEST_F(UT_DataModel, test_data_009)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    m_tester->m_listEntry << entry;
    entry.strFileName = "2";
    entry.strFullPath = "2";
    entry.isDirectory = true;
    entry.uLastModifiedTime = 123456;
    entry.qSize = 0;
    m_tester->m_listEntry << entry;

    QModelIndex index = m_tester->index(0, DC_Time);
    EXPECT_EQ(m_tester->data(index, Qt::StatusTipRole), QVariant());
}

TEST_F(UT_DataModel, test_rowCount)
{
    EXPECT_EQ(m_tester->rowCount(QModelIndex()), m_tester->m_listEntry.count());
}

TEST_F(UT_DataModel, test_columnCount)
{
    EXPECT_EQ(m_tester->columnCount(QModelIndex()), m_tester->m_listColumn.count());
}

TEST_F(UT_DataModel, test_refreshFileEntry)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    EXPECT_EQ(m_tester->m_listEntry, listEntry);
}

TEST_F(UT_DataModel, test_getSelectItem)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    QItemSelection sel = m_tester->getSelectItem(QStringList() << "1.txt");

    EXPECT_EQ(sel.count() != 0, true);
}

TEST_F(UT_DataModel, test_getListEntryIndex_001)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    EXPECT_EQ(m_tester->getListEntryIndex("1.txt").row(), 0);
}

TEST_F(UT_DataModel, test_getListEntryIndex_002)
{
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;

    QList<FileEntry> listEntry;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    EXPECT_EQ(m_tester->getListEntryIndex("2.txt").row(), -1);
}

TEST_F(UT_DataModel, test_sort_001)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);

    m_tester->sort(5, Qt::DescendingOrder);
}

TEST_F(UT_DataModel, test_sort_002)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(0, Qt::DescendingOrder);
}

TEST_F(UT_DataModel, test_sort_003)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(0, Qt::AscendingOrder);
}

TEST_F(UT_DataModel, test_sort_004)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(1, Qt::DescendingOrder);
}

TEST_F(UT_DataModel, test_sort_005)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(1, Qt::AscendingOrder);
}

TEST_F(UT_DataModel, test_sort_006)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(2, Qt::DescendingOrder);
}

TEST_F(UT_DataModel, test_sort_007)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(2, Qt::AscendingOrder);
}

TEST_F(UT_DataModel, test_sort_008)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(3, Qt::DescendingOrder);
}

TEST_F(UT_DataModel, test_sort_009)
{
    QList<FileEntry> listEntry;
    FileEntry entry;
    entry.strFileName = "1.txt";
    entry.strFullPath = "1.txt";
    entry.uLastModifiedTime = 123456;
    entry.qSize = 10;
    listEntry << entry;
    entry.strFileName = "哈哈.txt";
    entry.strFullPath = "哈哈.txt";
    entry.uLastModifiedTime = 45678;
    entry.qSize = 20;
    listEntry << entry;
    entry.strFileName = "呵呵.txt";
    entry.strFullPath = "呵呵.txt";
    entry.uLastModifiedTime = 8794;
    entry.qSize = 30;
    listEntry << entry;

    m_tester->refreshFileEntry(listEntry);
    m_tester->sort(3, Qt::AscendingOrder);
}
