// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cliproperties.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>

#include <QTextCodec>
#include <QFileInfo>
/*******************************函数打桩************************************/

/*******************************函数打桩************************************/


// 测试CliProperties
class UT_CliProperties : public ::testing::Test
{
public:
    UT_CliProperties(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strPath = _SOURCEDIR;
        strPath += "/3rdparty/libzipplugin/kerfuffle_libzip.json";
        CustomMimeType mimeType;
        mimeType.m_bUnKnown = true;
        mimeType.m_strTypeName = "application/zip";
        KPluginMetaData metadata(strPath);
        m_tester = new CliProperties(nullptr, metadata, mimeType);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CliProperties *m_tester;
};

TEST_F(UT_CliProperties, initTest)
{

}

TEST_F(UT_CliProperties, test_addArgs_001)
{
    m_tester->m_compressionLevelSwitch = "ON";
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->m_compressionMethodSwitch["application/zip"] = "ON";
    QStringList list = QStringList() << "-c" << "tar cf - | 7z a -si 123 ON ON  1.zip";
    EXPECT_EQ(m_tester->addArgs("1.zip", QStringList() << "1.txt", "123", false, 3, "BZip2", "AES256", 0, true, ""), list);
}

TEST_F(UT_CliProperties, test_addArgs_002)
{
    m_tester->m_compressionLevelSwitch = "ON";
    m_tester->m_passwordSwitch = QStringList() << "123";
    m_tester->m_compressionMethodSwitch["application/zip"] = "ON";
    QStringList list = QStringList() << "123" << "ON" << "ON" << "1.zip" << "1.txt";
    EXPECT_EQ(m_tester->addArgs("1.zip", QStringList() << "1.txt", "123", false, 3, "BZip2", "AES256", 0, false, ""), list);
}

TEST_F(UT_CliProperties, test_commentArgs)
{
    m_tester->m_commentSwitch = QStringList() << "ON";
    QStringList list = QStringList() << "ON" << "1.zip";
    EXPECT_EQ(m_tester->commentArgs("1.zip", "123456789"), list);
}

TEST_F(UT_CliProperties, test_deleteArgs)
{
    QList<FileEntry> files;
    FileEntry entry;
    entry.strFullPath = "1/";
    files << entry;
    m_tester->m_passwordSwitch = QStringList() << "123";
    QStringList list = QStringList() << "123" << "1.zip" << "1";
    EXPECT_EQ(m_tester->deleteArgs("1.zip", files, "123456"), list);
}

TEST_F(UT_CliProperties, test_extractArgs_001)
{
    m_tester->m_extractSwitch = QStringList() << "1/";
    m_tester->m_passwordSwitch = QStringList() << "123";
    QStringList list = QStringList() << "1/" << "123" << "1.zip" << "1.txt";
    EXPECT_EQ(m_tester->extractArgs("1.zip", QStringList() << "1.txt", true, "123456"), list);
}

TEST_F(UT_CliProperties, test_extractArgs_002)
{
    m_tester->m_extractSwitch = QStringList() << "1/";
    m_tester->m_passwordSwitch = QStringList() << "123";
    QStringList list = QStringList() << "123" << "1.zip" << "1.txt";
    EXPECT_EQ(m_tester->extractArgs("1.zip", QStringList() << "1.txt", false, "123456"), list);
}

TEST_F(UT_CliProperties, test_listArgs)
{
    QStringList list = QStringList() << "1.zip";
    EXPECT_EQ(m_tester->listArgs("1.zip", "123456"), list);
}

TEST_F(UT_CliProperties, test_moveArgs)
{
    QList<FileEntry> files;
    FileEntry entry;
    entry.strAlias = "2";
    entry.strFullPath = "1";
    files << entry;
    FileEntry srcentry;
    srcentry.strFullPath = "1";
    m_tester->m_passwordSwitch = QStringList() << "123";
    ArchiveData stArchiveData;
    stArchiveData.mapFileEntry.insert("1", srcentry);
    QStringList list = QStringList() << "123" << "1.zip" << "1" << "2";
    EXPECT_EQ(m_tester->moveArgs("1.zip", files, stArchiveData, "123456"), list);
}

TEST_F(UT_CliProperties, test_testArgs)
{
    m_tester->m_testSwitch = QStringList() << "t";
    m_tester->m_passwordSwitch = QStringList() << "123";
    QStringList list = QStringList() << "t" << "123" << "1.zip";
    EXPECT_EQ(m_tester->testArgs("1.zip", "123"), list);
}

TEST_F(UT_CliProperties, test_substituteCommentSwitch)
{
    m_tester->m_commentSwitch = QStringList() << "ON";
    EXPECT_EQ(m_tester->substituteCommentSwitch("123"), QStringList() << "ON");
}
