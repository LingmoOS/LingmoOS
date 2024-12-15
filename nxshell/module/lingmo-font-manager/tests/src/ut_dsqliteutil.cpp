// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dsqliteutil.h"


#include <gtest/gtest.h>
#include "../third-party/stub/stub.h"
#include "commonheaderfile.h"

#include <QDir>

#include <DLog>

namespace {
class TestDSqliteUtil : public testing::Test
{
protected:
    void SetUp()
    {
        fs = new DSqliteUtil();
    }
    void TearDown()
    {
        delete  fs;
    }
    // Some expensive resource shared by all tests.
    DSqliteUtil *fs;
    QString tbname = "t_fontmanager";
    QMap<QString, QString> data;
};

}

TEST_F(TestDSqliteUtil, checkEscapeString)
{
    EXPECT_TRUE(fs->escapeString("") == "");
    EXPECT_TRUE(fs->escapeString(QString()) == "");
    EXPECT_EQ("first", fs->escapeString("first"));
}

TEST_F(TestDSqliteUtil, checkAddRecord)
{
    fs->delAllRecords();

    QMap<QString, QString> data;
    data.insert("isCollected", "1");
    EXPECT_TRUE(fs->addRecord(data));

    QMap<QString, QString> data1;
    data1.insert("isCollected", "2");

    EXPECT_TRUE(fs->updateRecord(data, data1));

    QList<QString> list;
    QList<QMap<QString, QString>> datalist;
    QMap<QString, QString> data2;
    list << "isCollected";

    EXPECT_TRUE(fs->findRecords(list, &datalist));


    EXPECT_TRUE(fs->delRecord(data1));


    EXPECT_FALSE(fs->addRecord(data1, ""));
}

TEST_F(TestDSqliteUtil, checkAddFontinfo)
{
    QList<DFontPreviewItemData> fontList;
    fs->addFontInfo(fontList);
    fs->delAllRecords();
    DFontPreviewItemData data;
    fontList << data;
    fs->addFontInfo(fontList);
    EXPECT_TRUE(fs->getRecordCount() == 1);
    EXPECT_TRUE(fs->getMaxFontId());
    fs->delAllRecords();
    fs->finish();

    fs->addFontInfo(fontList, "");
}

TEST_F(TestDSqliteUtil, checkUpdateFontInfo)
{
    QList<DFontPreviewItemData> fontList;

    DFontPreviewItemData data;

    fs->delAllRecords();

    fontList << data;
    fs->addFontInfo(fontList);

    data.fontData.setChinese(true);
    fontList.clear();
    fontList << data;
    fs->updateFontInfo(fontList, "isCollected");

    fs->updateFontInfo(fontList, "isMonoSpace");
    fs->updateFontInfo(fontList, "isMonoSpace", "");
    fs->delAllRecords();
}

TEST_F(TestDSqliteUtil, checkGetInstalledFontsPath)
{
    fs->delAllRecords();
    QList<DFontPreviewItemData> fontList;

    DFontPreviewItemData data;
    data.fontInfo.isInstalled = true;
    data.fontInfo.psname = "first";
    fontList << data;
    fs->addFontInfo(fontList);

    QStringList list = fs->getInstalledFontsPath();
    EXPECT_TRUE(list.count() == 1);
}


//删除记录函数可能出错，删除一条记录后数目不对
TEST_F(TestDSqliteUtil, checkDeleteFontInfo)
{

    QList<DFontPreviewItemData> fontList;
    DFontPreviewItemData data;
    data.fontInfo.filePath = "first";
    fontList << data;
    fs->delAllRecords();

    fs->addFontInfo(fontList);
    EXPECT_TRUE(fs->getRecordCount() == 1) << fs->getRecordCount() ;
    qDebug() << fs->getRecordCount();
    fs->deleteFontInfo(fontList);
    qDebug() << fs->getRecordCount();
    EXPECT_TRUE(fs->getRecordCount() == 0) << fs->getRecordCount() ;
    qDebug() << fs->getRecordCount();
}

TEST_F(TestDSqliteUtil, checkFindAllRecords)
{
    QList<DFontPreviewItemData> fontList;
    DFontPreviewItemData data;
    data.fontInfo.psname = "first";
    fontList << data;
    fs->delAllRecords();

    fs->addFontInfo(fontList);

    QList<QMap<QString, QString>> recordList;
    QList<QString> keyList;
    keyList << "psname";
    fs->findAllRecords(keyList, recordList);

    EXPECT_TRUE(recordList.count() == 1);

    EXPECT_FALSE(fs->findAllRecords(keyList, recordList, ""));
}

TEST_F(TestDSqliteUtil, checkDelAllRecords)
{
    fs->delAllRecords();
    EXPECT_TRUE(fs->getRecordCount() == 0);
}


TEST_F(TestDSqliteUtil, checkDelAllRecordsErr)
{

    EXPECT_FALSE(fs->delAllRecords(""));
}


TEST_F(TestDSqliteUtil, checkFinish)
{
    SAFE_DELETE_ELE(fs->m_query)
    fs->m_query = nullptr;
    fs->finish();
}





