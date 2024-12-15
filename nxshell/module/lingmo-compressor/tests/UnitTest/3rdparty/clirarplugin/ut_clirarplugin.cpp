// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "clirarplugin.h"
#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)

PluginFinishType g_cliInterface_handlePassword_result = PFT_Nomral;
/*******************************函数打桩************************************/
int kill_stub(__pid_t, int)
{
    return 0;
}

qint64 kPtyProcess_processId_stub()
{
    return 123456;
}

PluginFinishType cliInterface_handlePassword_stub()
{
    return g_cliInterface_handlePassword_result;
}

void cliInterface_writeToProcess_stub()
{
    return ;
}

bool readOnlyArchiveInterface_isInsufficientDiskSpace_stub()
{
    return true;
}
/*******************************函数打桩************************************/

class UT_CliRarPluginFactory : public QObject, public ::testing::Test
{
public:
    UT_CliRarPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new CliRarPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CliRarPluginFactory *m_tester;
};

class UT_CliRarPlugin : public QObject, public ::testing::Test
{
public:
    UT_CliRarPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("test.rar").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new CliRarPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CliRarPlugin *m_tester;
};


TEST_F(UT_CliRarPluginFactory, initTest)
{

}

TEST_F(UT_CliRarPlugin, initTest)
{

}

TEST_F(UT_CliRarPlugin, test_setupCliProperties)
{
    m_tester->setupCliProperties();

    EXPECT_EQ(m_tester->m_cliProps->property("captureProgress"), true);

    EXPECT_EQ(m_tester->m_cliProps->property("addProgram"), QStringLiteral("rar"));
    EXPECT_EQ(m_tester->m_cliProps->property("addSwitch"), QStringList({QStringLiteral("a")}));

    EXPECT_EQ(m_tester->m_cliProps->property("deleteProgram"), QStringLiteral("rar"));
    EXPECT_EQ(m_tester->m_cliProps->property("deleteSwitch"), QStringLiteral("d"));

    EXPECT_EQ(m_tester->m_cliProps->property("extractProgram"), QStringLiteral("unrar"));
    EXPECT_EQ(m_tester->m_cliProps->property("extractSwitch"), QStringList({QStringLiteral("x"), QStringLiteral("-kb")}));
    EXPECT_EQ(m_tester->m_cliProps->property("extractSwitchNoPreserve"), QStringList({QStringLiteral("e"), QStringLiteral("-kb")}));

    EXPECT_EQ(m_tester->m_cliProps->property("listProgram"), QStringLiteral("unrar"));
    EXPECT_EQ(m_tester->m_cliProps->property("listSwitch"), QStringList({QStringLiteral("vt"), QStringLiteral("-v")}));

    EXPECT_EQ(m_tester->m_cliProps->property("moveProgram"), QStringLiteral("rar"));
    EXPECT_EQ(m_tester->m_cliProps->property("moveSwitch"), QStringLiteral("rn"));

    EXPECT_EQ(m_tester->m_cliProps->property("testProgram"), QStringLiteral("unrar"));
    EXPECT_EQ(m_tester->m_cliProps->property("testSwitch"), QStringLiteral("t"));

    EXPECT_EQ(m_tester->m_cliProps->property("commentSwitch"), QStringList({QStringLiteral("c"), QStringLiteral("-z$CommentFile")}));

    EXPECT_EQ(m_tester->m_cliProps->property("passwordSwitch"), QStringList{QStringLiteral("-p$Password")});
    EXPECT_EQ(m_tester->m_cliProps->property("passwordSwitchHeaderEnc"), QStringList{QStringLiteral("-hp$Password")});

    EXPECT_EQ(m_tester->m_cliProps->property("compressionLevelSwitch"), QStringLiteral("-m$CompressionLevel"));

    EXPECT_EQ(m_tester->m_cliProps->property("multiVolumeSwitch"), QStringLiteral("-v$VolumeSizek"));

    EXPECT_EQ(m_tester->m_cliProps->property("testPassedPatterns"), QStringList{QStringLiteral("^All OK$")});
    EXPECT_EQ(m_tester->m_cliProps->property("fileExistsFileNameRegExp"), QStringList({QStringLiteral("^(.+) already exists. Overwrite it"),
                                                                                       QStringLiteral("^Would you like to replace the existing file (.+)$")}));
    EXPECT_EQ(m_tester->m_cliProps->property("fileExistsInput"), QStringList({QStringLiteral("Y"),   //Overwrite
                                                                              QStringLiteral("N"),   //Skip
                                                                              QStringLiteral("A"),   //Overwrite all
                                                                              QStringLiteral("E"),   //Autoskip
                                                                              QStringLiteral("Q")}));
}

TEST_F(UT_CliRarPlugin, test_isPasswordPrompt)
{
    EXPECT_EQ(m_tester->isPasswordPrompt("Enter password (will not be echoed) for : "), true);
}
TEST_F(UT_CliRarPlugin, test_isWrongPasswordMsg)
{
    EXPECT_EQ(m_tester->isWrongPasswordMsg("The specified password is incorrect"), true);
}

TEST_F(UT_CliRarPlugin, test_isCorruptArchiveMsg)
{
    EXPECT_EQ(m_tester->isCorruptArchiveMsg("Unexpected end of archive"), true);
}

TEST_F(UT_CliRarPlugin, test_isDiskFullMsg)
{
    EXPECT_EQ(m_tester->isDiskFullMsg("No space left on device"), true);
}

TEST_F(UT_CliRarPlugin, test_isFileExistsMsg)
{
    EXPECT_EQ(m_tester->isFileExistsMsg("[Y]es, [N]o, [A]ll, n[E]ver, [R]ename, [Q]uit "), true);
}

TEST_F(UT_CliRarPlugin, test_isFileExistsFileName)
{
    EXPECT_EQ(m_tester->isFileExistsFileName("Would you like to replace the existing file "), true);
}

TEST_F(UT_CliRarPlugin, test_isMultiPasswordPrompt)
{
    EXPECT_EQ(m_tester->isMultiPasswordPrompt("use current password ? [Y]es, [N]o, [A]ll"), true);
}

TEST_F(UT_CliRarPlugin, test_isOpenFileFailed)
{
    EXPECT_EQ(m_tester->isOpenFileFailed("Cannot create "), true);
}

TEST_F(UT_CliRarPlugin, test_killProcess_001)
{
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KPtyProcess, processId), kPtyProcess_processId_stub);

    m_tester->m_isProcessKilled = false;
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_isProcessKilled, false);
}

TEST_F(UT_CliRarPlugin, test_killProcess_002)
{
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KPtyProcess, processId), kPtyProcess_processId_stub);

    m_tester->m_isProcessKilled = false;
    m_tester->m_process = new KPtyProcess;
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_isProcessKilled, true);
}

TEST_F(UT_CliRarPlugin, test_readListLine_001)
{
    m_tester->m_parseState = ParseStateTitle;
    EXPECT_EQ(m_tester->readListLine("UNRAR 5.61 beta 1 freeware      Copyright (c) 1993-2018 Alexander Roshal"), true);
    EXPECT_EQ(m_tester->m_parseState, ParseStateArchiveInformation);
}

TEST_F(UT_CliRarPlugin, test_readListLine_002)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    m_tester->m_comment = "123456";
    EXPECT_EQ(m_tester->readListLine("Archive:"), true);
    EXPECT_EQ(m_tester->m_parseState, ParseStateArchiveInformation);
    EXPECT_EQ(m_tester->m_comment, "123456");
}

TEST_F(UT_CliRarPlugin, test_readListLine_003)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Details:"), true);
    EXPECT_EQ(m_tester->m_parseState, ParseStateEntryInformation);
}

TEST_F(UT_CliRarPlugin, test_readListLine_004)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("sfasf:"), true);
    EXPECT_EQ(m_tester->m_comment, "sfasf:\n");
}

TEST_F(UT_CliRarPlugin, test_readListLine_005)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Name: 1.txt"), true);
    EXPECT_EQ(m_tester->m_fileEntry.strFullPath, "1.txt");
    EXPECT_EQ(m_tester->m_fileEntry.strFileName, "1.txt");
}

TEST_F(UT_CliRarPlugin, test_readListLine_006)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("        Type: File"), true);
    EXPECT_EQ(m_tester->m_fileEntry.isDirectory, false);
}

TEST_F(UT_CliRarPlugin, test_readListLine_007)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("        Type: Directory"), true);
    EXPECT_EQ(m_tester->m_fileEntry.isDirectory, true);
}

TEST_F(UT_CliRarPlugin, test_readListLine_008)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("        Size: 2066668493"), true);
    EXPECT_EQ(m_tester->m_fileEntry.qSize, 2066668493);
}

TEST_F(UT_CliRarPlugin, test_readListLine_009)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("       mtime: 2020-09-23 10:11:43,000000000"), true);
    EXPECT_EQ(m_tester->m_fileEntry.uLastModifiedTime, 0);
}

TEST_F(UT_CliRarPlugin, test_handleLine_001)
{
    Stub stub;
    stub.set(ADDR(CliInterface, handlePassword), cliInterface_handlePassword_stub);
    g_cliInterface_handlePassword_result = PFT_Cancel;
    EXPECT_EQ(m_tester->handleLine("Enter password (will not be echoed) for : ", WT_List), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Cancel);
}

TEST_F(UT_CliRarPlugin, test_handleLine_002)
{
    Stub stub;
    stub.set(ADDR(CliInterface, handlePassword), cliInterface_handlePassword_stub);
    g_cliInterface_handlePassword_result = PFT_Nomral;
    EXPECT_EQ(m_tester->handleLine("Enter password (will not be echoed) for : ", WT_Extract), true);
}

TEST_F(UT_CliRarPlugin, test_handleLine_003)
{
    EXPECT_EQ(m_tester->handleLine("The specified password is incorrect", WT_Extract), true);
    EXPECT_EQ(m_tester->m_eErrorType, ET_WrongPassword);
}

TEST_F(UT_CliRarPlugin, test_handleLine_004)
{
    EXPECT_EQ(m_tester->handleLine("Checksum error in the encrypted file", WT_Extract), false);
    EXPECT_EQ(m_tester->m_eErrorType, ET_WrongPassword);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
}

TEST_F(UT_CliRarPlugin, test_handleLine_005)
{
    EXPECT_EQ(m_tester->handleLine("Name: 电影歌曲.zip", WT_List), true);
}

TEST_F(UT_CliRarPlugin, test_handleLine_006)
{
    Stub stub;
    stub.set(ADDR(CliInterface, writeToProcess), cliInterface_writeToProcess_stub);
    EXPECT_EQ(m_tester->handleLine("use current password ? [Y]es, [N]o, [A]ll", WT_Extract), true);
}

TEST_F(UT_CliRarPlugin, test_handleLine_007)
{
    EXPECT_EQ(m_tester->handleLine("Cannot create 1/2/3/sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss.txt", WT_Extract), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
    EXPECT_EQ(m_tester->m_eErrorType, ET_LongNameError);
}

TEST_F(UT_CliRarPlugin, test_handleLine_008)
{
    Stub stub;
    stub.set(ADDR(CliInterface, writeToProcess), cliInterface_writeToProcess_stub);
    stub.set(ADDR(ReadOnlyArchiveInterface, isInsufficientDiskSpace), readOnlyArchiveInterface_isInsufficientDiskSpace_stub);
    EXPECT_EQ(m_tester->handleLine("Write error in the file 1.txt [R]etry, [A]bort ", WT_Extract), false);
    EXPECT_EQ(m_tester->m_eErrorType, ET_InsufficientDiskSpace);
}
