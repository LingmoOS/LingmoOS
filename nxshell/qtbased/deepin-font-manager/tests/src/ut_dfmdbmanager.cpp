// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dfmdbmanager.h"


#include "dfontinfomanager.h"
#include "dsqliteutil.h"

#include <QDir>

#include <gtest/gtest.h>

#include "../third-party/stub/stub.h"

namespace {
class TestDFMDBManager : public testing::Test
{

protected:
    void SetUp()
    {
        fmd = DFMDBManager::instance();
    }
    void TearDown()
    {
    }
    // Some expensive resource shared by all tests.
    DFMDBManager *fmd;
};

QStringList stub_getInstalledFontsPath()
{
    QStringList list;
    list << "first" << "second" << "third";
    return list;
}

int stub_getMaxFontId(const QString &table_name = "t_fontmanager")
{
    Q_UNUSED(table_name)
    return 10;
}

static QString g_funcname;
void  stub_addFontInfo(const QList<DFontPreviewItemData> &)
{
    g_funcname = __FUNCTION__;
}

void checkIfEmpty_stub()
{
    g_funcname = __FUNCTION__;
}

void stub_deleteFontInfo(const QList<DFontPreviewItemData> &, const QString &)
{
    g_funcname = __FUNCTION__;
}

void stub_updateFontInfo(const QList<DFontPreviewItemData> &, const QString &)
{
    g_funcname = __FUNCTION__;
}
bool stub_transaction()
{
    g_funcname = __FUNCTION__;
    return true;
}
bool stub_commit()
{
    g_funcname = __FUNCTION__;
    return true;
}
bool stub_findRecords(const QList<QString> &, const QMap<QString, QString> &,
                      QList<QMap<QString, QString>> *, const QString &)
{
    g_funcname = __FUNCTION__;
    return true;
}

bool stub_findAllRecords(void *, const QList<QString> &, QList<QMap<QString, QString>> &row, const QString &)
{
    QMap<QString, QString> mapRow;
    mapRow.insert("fontId", "1");
    mapRow.insert("fontName", "1Font");
    row.append(mapRow);
    return true;
}
DFontPreviewItemData stub_parseRecordToItemData(void *, const QMap<QString, QString> &record)
{
    DFontPreviewItemData itemData(record.value("filePath"), record.value("familyName"), record.value("styleName"),
                                  record.value("type"), record.value("version"), record.value("copyright"),
                                  record.value("description"), record.value("sysVersion"), record.value("fullname"),
                                  record.value("psname"), record.value("trademark"), record.value("isInstalled").toInt(),
                                  record.value("isError").toInt(), false,
                                  record.value("isEnabled").toInt(), record.value("isCollected").toInt(),
                                  record.value("isChineseFont").toInt(), record.value("isMonoSpace").toInt(),
                                  record.value("fontName"),
                                  record.value("fontPreview"), record.value("fontId"));

    itemData.fontInfo.filePath = "/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf";
    return itemData;
}
bool stub_exists()
{
    return true;
}
}

TEST_F(TestDFMDBManager, initTest)
{

}

//传入系统字体判断是否为系统字体
TEST_F(TestDFMDBManager, check_SystemFont_IsSystemFont)
{
    Stub stub;
    EXPECT_EQ(true, fmd->isSystemFont("/usr/share/fonts/truetype/liberation/LiberationMono-Italic.ttf"));
}

//传入用户字体判断是否为系统字体
TEST_F(TestDFMDBManager, check_UserFont_IsSystestub_delRecordmFont)
{
    EXPECT_EQ(false, fmd->isSystemFont(QDir::homePath() + "/Desktop/1048字体/Addictype-Regular.otf"));
}

TEST_F(TestDFMDBManager, check_getInstalledFontsPath)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, getInstalledFontsPath), stub_getInstalledFontsPath);
    EXPECT_EQ(3, fmd->getInstalledFontsPath().count());
    EXPECT_EQ(true, fmd->getInstalledFontsPath().contains("first"));
}

TEST_F(TestDFMDBManager, check_getRecordCount)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, getMaxFontId), stub_getMaxFontId);
    EXPECT_EQ(10, fmd->getCurrMaxFontId());
}



TEST_F(TestDFMDBManager, checkGetAllFontInfo)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, findAllRecords), stub_findAllRecords);
    s.set(ADDR(DFMDBManager, parseRecordToItemData), stub_parseRecordToItemData);
    typedef bool (QFileInfo::*fptr)()const;
    fptr A_foo = (fptr)(&QFileInfo::exists);
    s.set(A_foo, stub_exists);

    QList<DFontPreviewItemData> list;
    QList<DFontPreviewItemData> fontItemDataList = fmd->getAllFontInfo(&list);
    EXPECT_FALSE(fontItemDataList.isEmpty());
}

TEST_F(TestDFMDBManager, checkcheckIfEmpty)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, checkIfEmpty), checkIfEmpty_stub);
    fmd->checkIfEmpty();
    EXPECT_TRUE(g_funcname == QLatin1String("checkIfEmpty_stub"));
}


//commitAddFontInfo
TEST_F(TestDFMDBManager, checkCommitAddFontInfo)
{
    Stub s;
    s.set((void(DFMDBManager::*)(const QList<DFontPreviewItemData> &))ADDR(DFMDBManager, addFontInfo), stub_addFontInfo);

    fmd->m_addFontList.clear();
    g_funcname.clear();
    fmd->commitAddFontInfo();
    EXPECT_TRUE(g_funcname.isEmpty());

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";
    fmd->m_addFontList << data;
    fmd->commitAddFontInfo();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_addFontInfo"));
}

//addFontInfo
TEST_F(TestDFMDBManager, checkAddFontInfo)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, addFontInfo), stub_addFontInfo);
    QList<DFontPreviewItemData> list;
    g_funcname.clear();
    fmd->addFontInfo(list);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_addFontInfo"));
}

//deleteFontInfo
TEST_F(TestDFMDBManager, checkDeleteFontInfoFirst)
{
    fmd->m_delFontList.clear();

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    fmd->deleteFontInfo(data);

    EXPECT_TRUE(fmd->m_delFontList.contains(data));

}

//deleteFontInfo
TEST_F(TestDFMDBManager, checkDeleteFontInfoSecond)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, deleteFontInfo), stub_deleteFontInfo);
    QList<DFontPreviewItemData> list;
    fmd->deleteFontInfo(list);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_deleteFontInfo"));
}

//commitUpdateFontInfo
TEST_F(TestDFMDBManager, checkCommitDeleteFontInfo)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, deleteFontInfo), stub_deleteFontInfo);

    fmd->m_delFontList.clear();
    g_funcname.clear();
    fmd->commitDeleteFontInfo();
    EXPECT_TRUE(g_funcname.isEmpty());

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";

    fmd->m_delFontList << data;

    fmd->commitDeleteFontInfo();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_deleteFontInfo"));
}

//updateFontInfo
TEST_F(TestDFMDBManager, checkUpdateFontInfoFirst)
{
    fmd->m_strKey = "";
    DFontPreviewItemData data;

    data.fontInfo.psname = "first";

    fmd->updateFontInfo(data, "psname");

    EXPECT_TRUE(fmd->m_updateFontList.first() == data);
    EXPECT_TRUE(fmd->m_strKey == "psname");
}

//updateFontInfo
TEST_F(TestDFMDBManager, checkUpdateFontInfoSecond)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, updateFontInfo), stub_updateFontInfo);

    QList<DFontPreviewItemData> list;
    QString key = "psname";

    fmd->updateFontInfo(list, key);
    EXPECT_TRUE(g_funcname == QLatin1String("stub_updateFontInfo"));
}

//updateFontInfo
bool updateRecord_stub(QMap<QString, QString> where, QMap<QString, QString> data, const QString &table_name)
{
    return true;
}
TEST_F(TestDFMDBManager, checkUpdateFontInfoThird)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, updateRecord), updateRecord_stub);

    QMap<QString, QString>  whereMap;
    QMap<QString, QString> dataMap;
    EXPECT_EQ(fmd->updateFontInfo(whereMap, dataMap), true);
}

//commitUpdateFontInfo
TEST_F(TestDFMDBManager, checkCommitUpdateFontInfo)
{
    Stub s;
    s.set(ADDR(DSqliteUtil, updateFontInfo), stub_updateFontInfo);

    fmd->m_updateFontList.clear();
    g_funcname.clear();
    fmd->commitUpdateFontInfo();
    EXPECT_TRUE(g_funcname.isEmpty());

    DFontPreviewItemData data;
    data.fontInfo.psname = "first";
    fmd->m_updateFontList << data;
    fmd->commitUpdateFontInfo();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_updateFontInfo"));
}

TEST_F(TestDFMDBManager, checkBeginAndEndTransaction)
{
    Stub s;
    s.set(ADDR(QSqlDatabase, transaction), stub_transaction);
    s.set(ADDR(QSqlDatabase, commit), stub_commit);

    fmd->beginTransaction();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_transaction"));
    fmd->endTransaction();
    EXPECT_TRUE(g_funcname == QLatin1String("stub_commit"));
}


TEST_F(TestDFMDBManager, getSpecifiedFontName)
{
    Stub s;
    s.set((bool(DSqliteUtil::*)(const QList<QString> &, const QMap<QString, QString> &,
                                QList<QMap<QString, QString>> *, const QString &))ADDR(DSqliteUtil, findRecords), stub_findRecords);

    QString filepath = "123";
    EXPECT_TRUE(fmd->getSpecifiedFontName(filepath).isEmpty());
    EXPECT_TRUE(g_funcname == QLatin1String("stub_findRecords"));
}
