// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cli7zplugin.h"
#include "commonstruct.h"
#include "datamanager.h"
#include "queries.h"
#include "gtest/src/stub.h"
#include <gtest/gtest.h>

#include <QFileInfo>
#include <QDir>
#include <QMimeDatabase>
#include <QStandardPaths>

Q_DECLARE_METATYPE(KPluginMetaData)

PluginFinishType g_cliInterface_handlePassword_result = PFT_Nomral;
bool g_cliInterface_handleFileExists_result = false;
bool g_QProcess_waitForStarted_result = false;
bool g_PasswordNeededQuery_responseCancelled_result = false;
bool g_OverwriteQuery_responseCancelled_result = false;
bool g_OverwriteQuery_responseSkip_result = false;
bool g_OverwriteQuery_responseSkipAll_result = false;
bool g_OverwriteQuery_responseOverwrite_result = false;
bool g_OverwriteQuery_responseOverwriteAll_result = false;
bool g_LoadCorruptQuery_responseYes_result = false;
/*******************************函数打桩************************************/
PluginFinishType cliInterface_handlePassword_stub()
{
    return g_cliInterface_handlePassword_result;
}

bool cliInterface_handleFileExists_stub(const QString &)
{
    return g_cliInterface_handleFileExists_result;
}

bool cliInterface_runProcess_stub(const QString &, const QStringList &)
{
    return true;
}

int kill_stub(__pid_t, int)
{
    return 0;
}

void kProcess_start_stub()
{
    return ;
}

qint64 kProcess_processId_stub()
{
    return 123456;
}

bool qProcess_waitForStarted_stub(int)
{
    return g_QProcess_waitForStarted_result;
}

QString qStandardPaths_findExecutable_stub(const QString &executableName, const QStringList &paths)
{
    if (executableName == "7z") {
        return "/usr/bin/7z";
    }
    return"";
}

void query_waitForResponse_stub()
{
    return ;
}

bool passwordNeededQuery_responseCancelled_stub()
{
    return g_PasswordNeededQuery_responseCancelled_result;
}

bool overwriteQuery_responseCancelled_stub()
{
    return g_OverwriteQuery_responseCancelled_result;
}

bool overwriteQuery_responseSkip_stub()
{
    return g_OverwriteQuery_responseSkip_result;
}

bool overwriteQuery_responseSkipAll_stub()
{
    return g_OverwriteQuery_responseSkipAll_result;
}

bool overwriteQuery_responseOverwrite_stub()
{
    return g_OverwriteQuery_responseOverwrite_result;
}

bool overwriteQuery_responseOverwriteAll_stub()
{
    return g_OverwriteQuery_responseOverwriteAll_result;
}

bool loadCorruptQuery_responseYes_stub()
{
    return g_LoadCorruptQuery_responseYes_result;
}

void writeToProcess_stub()
{
    return;
}

bool moveExtractTempFilesToDest_stub()
{
    return false;
}

bool isInsufficientDiskSpace_stub()
{
    return true;
}
/*******************************函数打桩************************************/

class UT_Cli7zPluginFactory : public QObject, public ::testing::Test
{
public:
    UT_Cli7zPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new Cli7zPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Cli7zPluginFactory *m_tester;
};

class UT_Cli7zPlugin : public QObject, public ::testing::Test
{
public:
    UT_Cli7zPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile = QFileInfo("test.7z").absoluteFilePath();
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new Cli7zPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    Cli7zPlugin *m_tester;
};


TEST_F(UT_Cli7zPluginFactory, initTest)
{

}

TEST_F(UT_Cli7zPlugin, initTest)
{

}

TEST_F(UT_Cli7zPlugin, test_isPasswordPrompt)
{
    EXPECT_EQ(m_tester->isPasswordPrompt("Enter password (will not be echoed):"), true);
}

TEST_F(UT_Cli7zPlugin, test_isWrongPasswordMsg)
{
    EXPECT_EQ(m_tester->isWrongPasswordMsg("Wrong password"), true);
}

TEST_F(UT_Cli7zPlugin, test_isCorruptArchiveMsg)
{
    EXPECT_EQ(m_tester->isCorruptArchiveMsg("Unexpected end of archive"), true);
}

TEST_F(UT_Cli7zPlugin, test_isisDiskFullMsg)
{
    EXPECT_EQ(m_tester->isDiskFullMsg("No space left on device"), true);
}

TEST_F(UT_Cli7zPlugin, testisFileExistsMsg)
{
    EXPECT_EQ(m_tester->isFileExistsMsg("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(UT_Cli7zPlugin, test_isFileExistsFileName)
{
    EXPECT_EQ(m_tester->isFileExistsFileName("file ./"), true);
}

TEST_F(UT_Cli7zPlugin, test_isMultiPasswordPrompt)
{
    EXPECT_EQ(m_tester->isMultiPasswordPrompt("ssssss"), false);
}

TEST_F(UT_Cli7zPlugin, test_isOpenFileFailed)
{
    EXPECT_EQ(m_tester->isOpenFileFailed("ERROR: Can not open output file :"), true);
}

TEST_F(UT_Cli7zPlugin, test_setupCliProperties)
{
    m_tester->setupCliProperties();

    EXPECT_EQ(m_tester->m_cliProps->property("captureProgress"), false);
    EXPECT_EQ(m_tester->m_cliProps->property("addProgram"), QStringLiteral("7z"));

    EXPECT_EQ(m_tester->m_cliProps->property("addSwitch"), QStringList({QStringLiteral("a"), QStringLiteral("-l")}));
    EXPECT_EQ(m_tester->m_cliProps->property("deleteProgram"), QStringLiteral("7z"));
    EXPECT_EQ(m_tester->m_cliProps->property("deleteSwitch"), QStringLiteral("d"));

    EXPECT_EQ(m_tester->m_cliProps->property("extractProgram"), QStringLiteral("7z"));
    EXPECT_EQ(m_tester->m_cliProps->property("extractSwitch"), QStringLiteral("x"));
    EXPECT_EQ(m_tester->m_cliProps->property("progressarg"), QStringLiteral("-bsp1"));
    EXPECT_EQ(m_tester->m_cliProps->property("extractSwitchNoPreserve"), QStringList{QStringLiteral("e")});

    EXPECT_EQ(m_tester->m_cliProps->property("listProgram"), QStringLiteral("7z"));
    EXPECT_EQ(m_tester->m_cliProps->property("listSwitch"), QStringList({QStringLiteral("l"), QStringLiteral("-slt")}));

    EXPECT_EQ(m_tester->m_cliProps->property("moveProgram"), QStringLiteral("7z"));
    EXPECT_EQ(m_tester->m_cliProps->property("moveSwitch"), QStringLiteral("rn"));

    EXPECT_EQ(m_tester->m_cliProps->property("testProgram"), QStringLiteral("7z")) ;
    EXPECT_EQ(m_tester->m_cliProps->property("testSwitch"), QStringLiteral("t")) ;

    EXPECT_EQ(m_tester->m_cliProps->property("passwordSwitch"), QStringLiteral("-p$Password")) ;
    EXPECT_EQ(m_tester->m_cliProps->property("passwordSwitchHeaderEnc"), QStringList({QStringLiteral("-p$Password"), QStringLiteral("-mhe=on")})) ;

    EXPECT_EQ(m_tester->m_cliProps->property("compressionLevelSwitch"), QStringLiteral("-mx=$CompressionLevel")) ;

    EXPECT_EQ(m_tester->m_cliProps->property("multiVolumeSwitch"), QStringLiteral("-v$VolumeSizek"));
    EXPECT_EQ(m_tester->m_cliProps->property("testPassedPatterns"), QStringList({QStringLiteral("^Everything is Ok$")})) ;
    EXPECT_EQ(m_tester->m_cliProps->property("fileExistsFileNameRegExp"), QStringList({QStringLiteral("^file \\./(.*)$"), QStringLiteral("^  Path:     \\./(.*)$")})) ;
    EXPECT_EQ(m_tester->m_cliProps->property("fileExistsInput"), QStringList({QStringLiteral("Y"), QStringLiteral("N"), QStringLiteral("A"), QStringLiteral("S"), QStringLiteral("Q")})) ;
    EXPECT_EQ(m_tester->m_cliProps->property("multiVolumeSuffix"), QStringList{QStringLiteral("$Suffix.001")});
}

TEST_F(UT_Cli7zPlugin, test_killProcess_001)
{
    m_tester->m_process = nullptr;
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_process, nullptr);
}

TEST_F(UT_Cli7zPlugin, test_killProcess_002)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->m_bWaitingPassword = true;
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KProcess, processId), kProcess_processId_stub);
    m_tester->m_bWaitingPassword = true;
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_isProcessKilled, true);
}

TEST_F(UT_Cli7zPlugin, test_killProcess_003)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->m_bWaitingPassword = true;
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KProcess, processId), kProcess_processId_stub);
    m_tester->m_bWaitingPassword = false;
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_isProcessKilled, true);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_001)
{
    EXPECT_EQ(m_tester->readListLine("Open ERROR: Can not open the file as [7z] archive"), false);
    EXPECT_EQ(m_tester->m_eErrorType, ET_ArchiveDamaged);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_002)
{
    m_tester->m_parseState = ParseStateTitle;
    EXPECT_EQ(m_tester->readListLine("p7zip Version 16.02 (locale=zh_CN.UTF-8,Utf16=on,HugeFiles=on,64 bits,16 CPUs Intel(R) Core(TM) i7-10700 CPU @ 2.90GHz (A0655),ASM,AES-NI)"), true);
    EXPECT_EQ(m_tester->m_parseState, ParseStateHeader);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_003)
{
    m_tester->m_parseState = ParseStateHeader;
    EXPECT_EQ(m_tester->readListLine("--"), true);
    EXPECT_EQ(m_tester->m_parseState, ParseStateArchiveInformation);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_004)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("----------"), true);
    EXPECT_EQ(m_tester->m_parseState, ParseStateEntryInformation);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_005)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = 7z"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveType7z);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_006)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = bzip2"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeBZip2);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_007)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = gzip"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeGZip);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_008)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = xz"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeXz);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_009)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = tar"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeTar);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_010)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = zip"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeZip);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_011)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = Rar"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeRar);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_012)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = Split"), true);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_013)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = Udf"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeUdf);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_014)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = Iso"), true);
    EXPECT_EQ(m_tester->m_archiveType, Cli7zPlugin::ArchiveType::ArchiveTypeIso);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_015)
{
    m_tester->m_parseState = ParseStateArchiveInformation;
    EXPECT_EQ(m_tester->readListLine("Type = crx"), false);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_016)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Path = /home"), true);
    EXPECT_EQ(m_tester->m_fileEntry.strFullPath, "/home");
    EXPECT_EQ(m_tester->m_fileEntry.strFileName, "home");
}

TEST_F(UT_Cli7zPlugin, test_readListLine_017)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Size = 8172"), true);
    EXPECT_EQ(m_tester->m_fileEntry.qSize, 8172);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_018)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    m_tester->m_archiveType = Cli7zPlugin::ArchiveType::ArchiveType7z;
    EXPECT_EQ(m_tester->readListLine("Modified = 2021-06-18 15:27:01"), true);
    EXPECT_EQ(m_tester->m_fileEntry.uLastModifiedTime, QDateTime::fromString("2021-06-18 15:27:01", "yyyy-MM-dd hh:mm:ss").toTime_t());
}

TEST_F(UT_Cli7zPlugin, test_readListLine_019)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    m_tester->m_archiveType = Cli7zPlugin::ArchiveType::ArchiveTypeIso;
    EXPECT_EQ(m_tester->readListLine("Modified = 2021-06-18 15:27:01"), true);
    EXPECT_EQ(m_tester->m_fileEntry.uLastModifiedTime, 0);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_020)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Attributes = D...."), true);
    EXPECT_EQ(m_tester->m_fileEntry.isDirectory, true);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_021)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Attributes = ....."), true);
    EXPECT_EQ(m_tester->m_fileEntry.isDirectory, false);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_022)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Block = "), true);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_023)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Folder = +"), true);
    EXPECT_EQ(m_tester->m_fileEntry.isDirectory, true);
}

TEST_F(UT_Cli7zPlugin, test_readListLine_024)
{
    m_tester->m_parseState = ParseStateEntryInformation;
    EXPECT_EQ(m_tester->readListLine("Folder = -"), true);
    EXPECT_EQ(m_tester->m_fileEntry.isDirectory, false);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_001)
{
    Stub stub;
    stub.set(ADDR(CliInterface, handlePassword), cliInterface_handlePassword_stub);
    g_cliInterface_handlePassword_result = PFT_Cancel;
    EXPECT_EQ(m_tester->handleLine("Enter password (will not be echoed):", WT_List), false);
    EXPECT_EQ(m_tester->m_eErrorType, ET_NeedPassword);
    EXPECT_EQ(m_tester->m_finishType, PFT_Cancel);
    EXPECT_EQ(m_tester->m_bWaitingPassword, true);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_002)
{
    Stub stub;
    stub.set(ADDR(CliInterface, handlePassword), cliInterface_handlePassword_stub);
    g_cliInterface_handlePassword_result = PFT_Nomral;
    EXPECT_EQ(m_tester->handleLine("Enter password (will not be echoed):", WT_List), true);
    EXPECT_EQ(m_tester->m_eErrorType, ET_NeedPassword);
    EXPECT_EQ(m_tester->m_bWaitingPassword, false);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_003)
{
    EXPECT_EQ(m_tester->handleLine("Wrong password", WT_List), false);
    EXPECT_EQ(m_tester->m_eErrorType, ET_WrongPassword);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_004)
{
    EXPECT_EQ(m_tester->handleLine("No space left on device", WT_List), false);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_005)
{
    EXPECT_EQ(m_tester->handleLine("Unexpected end of archive", WT_List), true);
    EXPECT_EQ(m_tester->m_isCorruptArchive, true);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_006)
{
    EXPECT_EQ(m_tester->handleLine("Unexpected end of archive", WT_Extract), true);
    EXPECT_EQ(m_tester->m_eErrorType, ET_MissingVolume);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_007)
{
    EXPECT_EQ(m_tester->handleLine("No files to process", WT_Extract), true);
    EXPECT_EQ(m_tester->m_isEmptyArchive, true);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_008)
{
    EXPECT_EQ(m_tester->handleLine("ERROR: Can not open output file : sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss"
                                   "sssssssssssssssssssssssssssssssssssssssssssssssss", WT_Extract), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
    EXPECT_EQ(m_tester->m_eErrorType, ET_LongNameError);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_009)
{
    EXPECT_EQ(m_tester->handleLine("System ERROR:28", WT_Add), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
    EXPECT_EQ(m_tester->m_eErrorType, ET_InsufficientDiskSpace);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_010)
{
    Stub stub;
    stub.set(ADDR(ReadOnlyArchiveInterface, isInsufficientDiskSpace), isInsufficientDiskSpace_stub);
    EXPECT_EQ(m_tester->handleLine("ERROR: E_FAIL", WT_Extract), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
    EXPECT_EQ(m_tester->m_eErrorType, ET_InsufficientDiskSpace);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_011)
{
    m_tester->m_eErrorType = ET_WrongPassword;
    EXPECT_EQ(m_tester->handleLine("E_FAIL", WT_Delete), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
}

TEST_F(UT_Cli7zPlugin, test_handleLine_012)
{
    EXPECT_EQ(m_tester->handleLine("MAX_PATHNAME_LEN", WT_Add), false);
    EXPECT_EQ(m_tester->m_finishType, PFT_Error);
    EXPECT_EQ(m_tester->m_eErrorType, ET_LongNameError);
}

TEST_F(UT_Cli7zPlugin, test_isNoFilesArchive)
{
    EXPECT_EQ(m_tester->isNoFilesArchive("No files to process"), true);
}

TEST_F(UT_Cli7zPlugin, test_list)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);
    EXPECT_EQ(m_tester->list(), PFT_Nomral);
    EXPECT_EQ(m_tester->m_strPassword.isEmpty(), true);
    EXPECT_EQ(m_tester->m_setHasRootDirs.isEmpty(), true);
    EXPECT_EQ(m_tester->m_setHasHandlesDirs.isEmpty(), true);
    EXPECT_EQ(m_tester->m_workStatus, WT_List);
}

TEST_F(UT_Cli7zPlugin, test_testArchive)
{
    EXPECT_EQ(m_tester->testArchive(), PFT_Nomral);
    EXPECT_EQ(m_tester->m_workStatus, WT_Add);
}

TEST_F(UT_Cli7zPlugin, test_extractFiles_001)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    ExtractionOptions options;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/7z/extract";
    options.bAllExtract = true;
    EXPECT_EQ(m_tester->extractFiles(QList<FileEntry>(), options), PFT_Nomral);
    EXPECT_EQ(m_tester->m_strPassword.isEmpty(), true);
    EXPECT_EQ(m_tester->m_workStatus, WT_Extract);
}

TEST_F(UT_Cli7zPlugin, test_extractFiles_002)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    FileEntry entry;
    entry.strFullPath = "1.txt";
    entry.strFileName = "1.txt";
    ExtractionOptions options;
    options.strTargetPath = _UTSOURCEDIR;
    options.strTargetPath += "/test_sources/7z/extract";
    options.bAllExtract = false;
    EXPECT_EQ(m_tester->extractFiles(QList<FileEntry>() << entry, options), PFT_Nomral);
    EXPECT_EQ(m_tester->m_strPassword.isEmpty(), true);
    EXPECT_EQ(m_tester->m_workStatus, WT_Extract);
}

TEST_F(UT_Cli7zPlugin, test_pauseOperation)
{
    Stub stub;
    stub.set(kill, kill_stub);
    m_tester->m_childProcessId << 123456 << 234567;
    m_tester->m_processId = 012345;
    m_tester->pauseOperation();
}

TEST_F(UT_Cli7zPlugin, test_continueOperation)
{
    Stub stub;
    stub.set(kill, kill_stub);
    m_tester->m_childProcessId << 123456 << 234567;
    m_tester->m_processId = 012345;
    m_tester->continueOperation();
}

TEST_F(UT_Cli7zPlugin, test_doKill_001)
{
    m_tester->m_process = new KPtyProcess;
    EXPECT_EQ(m_tester->doKill(), true);
}

TEST_F(UT_Cli7zPlugin, test_doKill_002)
{
    m_tester->m_process = nullptr;
    EXPECT_EQ(m_tester->doKill(), false);
}

TEST_F(UT_Cli7zPlugin, test_addFiles_001)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    QList<FileEntry> files;
    CompressOptions options;
    options.bTar_7z = true;

    m_tester->m_filesSize = 10;

    options.strDestination = _UTSOURCEDIR;
    options.strDestination += "/test_sources/7z/compress";
    EXPECT_EQ(m_tester->addFiles(files, options), PFT_Nomral);
    EXPECT_EQ(m_tester->m_workStatus, WT_Add);
}

TEST_F(UT_Cli7zPlugin, test_addFiles_002)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    QList<FileEntry> files;
    CompressOptions options;
    options.strDestination = _UTSOURCEDIR;
    options.strDestination += "/test_sources/7z/compress";
    options.bTar_7z = true;
    options.strDestination.clear();
    EXPECT_EQ(m_tester->addFiles(files, options), PFT_Nomral);
    EXPECT_EQ(m_tester->m_workStatus, WT_Add);
    EXPECT_EQ(m_tester->m_isTar7z, true);
}

TEST_F(UT_Cli7zPlugin, test_addFiles_003)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    QList<FileEntry> files;
    CompressOptions options;
    options.strDestination = _UTSOURCEDIR;
    options.strDestination += "/test_sources/7z/compress";
    options.bTar_7z = false;
    EXPECT_EQ(m_tester->addFiles(files, options), PFT_Nomral);
    EXPECT_EQ(m_tester->m_workStatus, WT_Add);
    EXPECT_EQ(m_tester->m_isTar7z, false);

}

TEST_F(UT_Cli7zPlugin, test_moveFiles)
{
    EXPECT_EQ(m_tester->moveFiles(QList<FileEntry>(), CompressOptions()), PFT_Nomral);
}

TEST_F(UT_Cli7zPlugin, test_copyFiles)
{
    EXPECT_EQ(m_tester->copyFiles(QList<FileEntry>(), CompressOptions()), PFT_Nomral);
}

TEST_F(UT_Cli7zPlugin, test_deleteFiles)
{
    Stub stub;
    stub.set(ADDR(CliInterface, runProcess), cliInterface_runProcess_stub);

    EXPECT_EQ(m_tester->deleteFiles(QList<FileEntry>()), PFT_Nomral);
    EXPECT_EQ(m_tester->m_workStatus, WT_Delete);
}

TEST_F(UT_Cli7zPlugin, test_addComment)
{
    EXPECT_EQ(m_tester->addComment(""), PFT_Nomral);
}

TEST_F(UT_Cli7zPlugin, test_updateArchiveData_001)
{
    UpdateOptions options;

    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    FileEntry entry;
    entry.strFullPath = "1/";
    entry.strFileName = "1";
    entry.isDirectory = true;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    options.listEntry << entry;
    entry.strFullPath = "2/";
    entry.strFileName = "2";
    entry.isDirectory = true;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    entry.strFullPath = "1.txt";
    entry.strFileName = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    options.listEntry << entry;
    entry.strFullPath = "1/1.txt";
    entry.strFileName = "1.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;

    options.eType = UpdateOptions::Delete;
    EXPECT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);
}

TEST_F(UT_Cli7zPlugin, test_updateArchiveData_002)
{
    UpdateOptions options;
    FileEntry entry;
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    entry.strFullPath = "1/2.txt";
    entry.strFileName = "2.txt";
    entry.isDirectory = false;
    entry.qSize = 10;
    stArchiveData.listRootEntry << entry;
    stArchiveData.mapFileEntry[entry.strFullPath] = entry;
    options.eType = UpdateOptions::Add;
    EXPECT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);
}

TEST_F(UT_Cli7zPlugin, test_setListEmptyLines)
{
    m_tester->setListEmptyLines(true);
    EXPECT_EQ(m_tester->m_listEmptyLines, true);
}

TEST_F(UT_Cli7zPlugin, test_runProcess_001)
{
    Stub stub;
    stub.set(ADDR(KProcess, start), kProcess_start_stub);
    stub.set(ADDR(QStandardPaths, findExecutable), qStandardPaths_findExecutable_stub);
    EXPECT_EQ(m_tester->runProcess("asdasd", QStringList()), false);
}

TEST_F(UT_Cli7zPlugin, test_runProcess_002)
{
//    Stub stub;
//    stub.set(ADDR(QStandardPaths, findExecutable), qStandardPaths_findExecutable_stub);
//    stub.set(ADDR(QProcess, waitForStarted), qProcess_waitForStarted_stub);
//    g_QProcess_waitForStarted_result = true;
//    m_tester->m_workStatus = WT_Extract;
//    m_tester->m_isTar7z = true;
//    EXPECT_EQ(m_tester->runProcess("7z", QStringList()), true);
//    EXPECT_EQ(m_tester->m_stdOutData.isEmpty(), true);
//    EXPECT_EQ(m_tester->m_isProcessKilled, false);
}

TEST_F(UT_Cli7zPlugin, test_runProcess_003)
{
    Stub stub;
    stub.set(ADDR(QStandardPaths, findExecutable), qStandardPaths_findExecutable_stub);
    stub.set(ADDR(QProcess, waitForStarted), qProcess_waitForStarted_stub);
    g_QProcess_waitForStarted_result = false;
    m_tester->m_workStatus = WT_Add;
    m_tester->m_isTar7z = false;
    EXPECT_EQ(m_tester->runProcess("7z", QStringList()), true);
    EXPECT_EQ(m_tester->m_stdOutData.isEmpty(), true);
    EXPECT_EQ(m_tester->m_isProcessKilled, false);
}

TEST_F(UT_Cli7zPlugin, test_deleteProcess)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->deleteProcess();
    EXPECT_EQ(m_tester->m_process, nullptr);
}

TEST_F(UT_Cli7zPlugin, test_handleProgress_001)
{
    m_tester->m_filesSize = 10;
    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("7z");
    m_tester->m_workStatus = WT_Extract;
    m_tester->handleProgress("22% \b\b\b\b 73593 - 1/2/3.js");
    EXPECT_EQ(m_tester->m_indexOfListRootEntry, 0);
}

TEST_F(UT_Cli7zPlugin, test_handleProgress_002)
{
    m_tester->m_filesSize = 10;
    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("7z");
    m_tester->m_workStatus = WT_Delete;
    m_tester->handleProgress("% = 1/2/3.js");
    EXPECT_NE(m_tester->m_process, nullptr);
}

TEST_F(UT_Cli7zPlugin, test_handleProgress_003)
{
    m_tester->m_filesSize = 10;
    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("7z");
    m_tester->m_workStatus = WT_Extract;
    m_tester->m_extractOptions.bExistList = false;
    m_tester->m_indexOfListRootEntry = 0;
    DataManager::get_instance().resetArchiveData();
    m_tester->handleProgress("12% \b\b\b\b 73593 - 1/2/3.js");
    EXPECT_EQ(m_tester->m_indexOfListRootEntry, 1);
    EXPECT_EQ(DataManager::get_instance().archiveData().listRootEntry.count(), 1);
}

TEST_F(UT_Cli7zPlugin, test_handleProgress_004)
{
    m_tester->m_filesSize = 10;
    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("unrar");
    m_tester->m_extractOptions.bExistList = false;
    m_tester->m_indexOfListRootEntry = 0;
    DataManager::get_instance().resetArchiveData();
    m_tester->handleProgress("Extracting  安装包/新建文件夹 - 副本 (2)/TabTip.exe.mui                         OK");
    EXPECT_EQ(m_tester->m_indexOfListRootEntry, 1);
    EXPECT_EQ(DataManager::get_instance().archiveData().listRootEntry.count(), 1);
}

TEST_F(UT_Cli7zPlugin, test_handleProgress_005)
{
    m_tester->m_filesSize = 10;
    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("bash");
    m_tester->handleProgress("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b                \b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b  7M + [Content]");
    EXPECT_NE(m_tester->m_process, nullptr);
}

TEST_F(UT_Cli7zPlugin, test_handlePassword_001)
{
    Stub stub;
    stub.set(ADDR(PasswordNeededQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(PasswordNeededQuery, responseCancelled), passwordNeededQuery_responseCancelled_stub);

    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("7z");

    g_PasswordNeededQuery_responseCancelled_result = true;
    EXPECT_EQ(m_tester->handlePassword(), PFT_Cancel);
    EXPECT_EQ(m_tester->m_eErrorType, ET_NoError);
    EXPECT_EQ(m_tester->m_strPassword.isEmpty(), true);
}

TEST_F(UT_Cli7zPlugin, test_handlePassword_002)
{
    Stub stub;
    stub.set(ADDR(PasswordNeededQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(PasswordNeededQuery, responseCancelled), passwordNeededQuery_responseCancelled_stub);

    g_PasswordNeededQuery_responseCancelled_result = false;

    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("7z");

    EXPECT_EQ(m_tester->handlePassword(), PFT_Nomral);
    EXPECT_EQ(m_tester->m_eErrorType, ET_NoError);
    EXPECT_EQ(m_tester->m_strPassword.isEmpty(), true);
}

TEST_F(UT_Cli7zPlugin, test_handlePassword_003)
{
    Stub stub;
    stub.set(ADDR(PasswordNeededQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(PasswordNeededQuery, responseCancelled), passwordNeededQuery_responseCancelled_stub);

    m_tester->m_process = new KPtyProcess;
    m_tester->m_process->setProgram("unrar");

    EXPECT_EQ(m_tester->handlePassword(), PFT_Nomral);
    EXPECT_EQ(m_tester->m_eErrorType, ET_NoError);
    EXPECT_EQ(m_tester->m_strPassword.isEmpty(), true);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_001)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);

    EXPECT_EQ(m_tester->handleFileExists("file ./"), false);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_002)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), overwriteQuery_responseCancelled_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), overwriteQuery_responseSkip_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), overwriteQuery_responseSkipAll_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwrite), overwriteQuery_responseOverwrite_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), overwriteQuery_responseOverwriteAll_stub);

    g_OverwriteQuery_responseCancelled_result = true;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    EXPECT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_003)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), overwriteQuery_responseCancelled_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), overwriteQuery_responseSkip_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), overwriteQuery_responseSkipAll_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwrite), overwriteQuery_responseOverwrite_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), overwriteQuery_responseOverwriteAll_stub);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = true;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    EXPECT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_004)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), overwriteQuery_responseCancelled_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), overwriteQuery_responseSkip_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), overwriteQuery_responseSkipAll_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwrite), overwriteQuery_responseOverwrite_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), overwriteQuery_responseOverwriteAll_stub);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = true;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    EXPECT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_005)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), overwriteQuery_responseCancelled_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), overwriteQuery_responseSkip_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), overwriteQuery_responseSkipAll_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwrite), overwriteQuery_responseOverwrite_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), overwriteQuery_responseOverwriteAll_stub);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = true;
    g_OverwriteQuery_responseOverwriteAll_result = false;
    EXPECT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_006)
{
    Stub stub;
    stub.set(ADDR(OverwriteQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(CliInterface, writeToProcess), writeToProcess_stub);
    stub.set(ADDR(OverwriteQuery, responseCancelled), overwriteQuery_responseCancelled_stub);
    stub.set(ADDR(OverwriteQuery, responseSkip), overwriteQuery_responseSkip_stub);
    stub.set(ADDR(OverwriteQuery, responseSkipAll), overwriteQuery_responseSkipAll_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwrite), overwriteQuery_responseOverwrite_stub);
    stub.set(ADDR(OverwriteQuery, responseOverwriteAll), overwriteQuery_responseOverwriteAll_stub);

    g_OverwriteQuery_responseCancelled_result = false;
    g_OverwriteQuery_responseSkip_result = false;
    g_OverwriteQuery_responseSkipAll_result = false;
    g_OverwriteQuery_responseOverwrite_result = false;
    g_OverwriteQuery_responseOverwriteAll_result = true;
    EXPECT_EQ(m_tester->handleFileExists("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "), true);
}

TEST_F(UT_Cli7zPlugin, test_handleFileExists_007)
{
    EXPECT_EQ(m_tester->handleFileExists("sssssssss"), false);
}

TEST_F(UT_Cli7zPlugin, test_handleCorrupt_001)
{
    Stub stub;
    stub.set(ADDR(LoadCorruptQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(LoadCorruptQuery, responseYes), loadCorruptQuery_responseYes_stub);

    g_LoadCorruptQuery_responseYes_result = false;
    EXPECT_EQ(m_tester->handleCorrupt(), PFT_Error);
}

TEST_F(UT_Cli7zPlugin, test_handleCorrupt_002)
{
    Stub stub;
    stub.set(ADDR(LoadCorruptQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(LoadCorruptQuery, responseYes), loadCorruptQuery_responseYes_stub);

    g_LoadCorruptQuery_responseYes_result = true;
    EXPECT_EQ(m_tester->handleCorrupt(), PFT_Nomral);
}

TEST_F(UT_Cli7zPlugin, test_writeToProcess)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->writeToProcess("");
    EXPECT_NE(m_tester->m_process->pty(), nullptr);
}

TEST_F(UT_Cli7zPlugin, test_moveExtractTempFilesToDest)
{

}

TEST_F(UT_Cli7zPlugin, test_readStdout)
{

}

TEST_F(UT_Cli7zPlugin, test_processFinished)
{
    Stub stub;
    stub.set(ADDR(LoadCorruptQuery, waitForResponse), query_waitForResponse_stub);
    stub.set(ADDR(LoadCorruptQuery, responseYes), loadCorruptQuery_responseYes_stub);

    g_LoadCorruptQuery_responseYes_result = false;

    m_tester->m_process = new KPtyProcess;
    m_tester->m_isCorruptArchive = true;
    m_tester->m_workStatus = WT_List;
    m_tester->processFinished(0, QProcess::NormalExit);
    EXPECT_EQ(m_tester->m_eErrorType, ET_MissingVolume);
    EXPECT_EQ(m_tester->m_finishType, PFT_Nomral);
    EXPECT_EQ(m_tester->m_isCorruptArchive, false);
}

TEST_F(UT_Cli7zPlugin, test_extractProcessFinished_001)
{
    Stub stub;
    stub.set(ADDR(CliInterface, moveExtractTempFilesToDest), moveExtractTempFilesToDest_stub);

    m_tester->m_process = new KPtyProcess;
    m_tester->m_extractOptions.bAllExtract = false;
    m_tester->m_extractOptions.strTargetPath = "/home";
    m_tester->extractProcessFinished(0, QProcess::NormalExit);
    EXPECT_EQ(m_tester->m_finishType, PFT_Nomral);
    EXPECT_EQ(m_tester->m_indexOfListRootEntry, 0);
    EXPECT_EQ(m_tester->m_isEmptyArchive, false);
    EXPECT_EQ(m_tester->m_rootNode.isEmpty(), true);
}

TEST_F(UT_Cli7zPlugin, test_extractProcessFinished_002)
{
    Stub stub;
    stub.set(ADDR(CliInterface, moveExtractTempFilesToDest), moveExtractTempFilesToDest_stub);

    m_tester->m_process = new KPtyProcess;
    m_tester->m_extractOptions.bAllExtract = false;
    m_tester->m_extractOptions.strTargetPath = "/home";
    m_tester->extractProcessFinished(0, QProcess::CrashExit);
    EXPECT_EQ(m_tester->m_finishType, PFT_Nomral);
    EXPECT_EQ(m_tester->m_indexOfListRootEntry, 0);
    EXPECT_EQ(m_tester->m_isEmptyArchive, false);
    EXPECT_EQ(m_tester->m_rootNode.isEmpty(), true);
}

TEST_F(UT_Cli7zPlugin, test_getChildProcessId)
{

}

TEST_F(UT_Cli7zPlugin, test_getTargetPath)
{
    m_tester->m_extractOptions.strTargetPath = "a/b";
    EXPECT_EQ(m_tester->getTargetPath(), "a/b");
}

TEST_F(UT_Cli7zPlugin, test_waitForFinished)
{
    m_tester->m_bWaitForFinished = true;
    EXPECT_EQ(m_tester->waitForFinished(), true);
}

TEST_F(UT_Cli7zPlugin, test_setPassword)
{
    m_tester->setPassword("123456");
    EXPECT_EQ(m_tester->m_strPassword, "123456");
}

TEST_F(UT_Cli7zPlugin, test_getPassword)
{
    m_tester->m_strPassword = "123456";
    EXPECT_EQ(m_tester->getPassword(), "123456");
}

TEST_F(UT_Cli7zPlugin, test_errorType)
{
    m_tester->m_eErrorType = ErrorType::ET_NoError;
    EXPECT_EQ(m_tester->errorType(), ErrorType::ET_NoError);
}

TEST_F(UT_Cli7zPlugin, test_setWaitForFinishedSignal)
{
    m_tester->setWaitForFinishedSignal(true);
    EXPECT_EQ(m_tester->m_bWaitForFinished, true);
}

TEST_F(UT_Cli7zPlugin, test_getPermissions_001)
{
    mode_t perm = 0;
    QFileDevice::Permissions pers = QFileDevice::Permissions();
    pers |= (QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
    EXPECT_EQ(m_tester->getPermissions(perm), pers);

}

TEST_F(UT_Cli7zPlugin, test_getPermissions_002)
{
    mode_t perm = 0;
    m_tester->getPermissions(perm);
    perm = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
    QFileDevice::Permissions pers = QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser | QFileDevice::ReadGroup | QFileDevice::WriteGroup | QFileDevice::ExeGroup | QFileDevice::ReadOther | QFileDevice::WriteOther | QFileDevice::ExeOther;
    EXPECT_EQ(m_tester->getPermissions(perm), pers);
}
