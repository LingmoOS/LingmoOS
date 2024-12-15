// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "archivemanager.h"
#include "libzipplugin.h"
#include "batchjob.h"
#include "singlejob.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

bool g_batchExtractJob_setArchiveFiles_result = false;
/*******************************函数打桩************************************/
bool batchExtractJob_setArchiveFiles_stub(const QStringList &)
{
    return g_batchExtractJob_setArchiveFiles_result;
}
/*******************************函数打桩************************************/

// 测试TypeLabel
class UT_ArchiveManager : public ::testing::Test
{
public:
    UT_ArchiveManager(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ArchiveManager;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ArchiveManager *m_tester;
};

TEST_F(UT_ArchiveManager, initTest)
{

}

TEST_F(UT_ArchiveManager, test_get_instance)
{
    EXPECT_EQ(m_tester->get_instance(), m_tester->m_instance);
}

TEST_F(UT_ArchiveManager, test_destory_instance)
{
    m_tester->destory_instance();
    EXPECT_EQ(nullptr, m_tester->m_instance);
}

TEST_F(UT_ArchiveManager, test_createArchive_001)
{
    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->createArchive(QList<FileEntry>(), "/home/Desktop", CompressOptions(), UiTools::APT_Auto), true);
    delete m_tester->m_pTempInterface;
    m_tester->m_pTempInterface = nullptr;
}

TEST_F(UT_ArchiveManager, test_createArchive_002)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->createArchive(QList<FileEntry>(), "/home/Desktop", CompressOptions(), UiTools::APT_Auto), false);
}

TEST_F(UT_ArchiveManager, test_loadArchive_001)
{
    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->loadArchive("1.zip", UiTools::APT_Auto), true);
}

TEST_F(UT_ArchiveManager, test_loadArchive_002)
{
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->loadArchive("1.zip", UiTools::APT_Auto), false);
}

TEST_F(UT_ArchiveManager, test_addFiles_001)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->addFiles("1.zip", QList<FileEntry>(), CompressOptions()), true);
    delete m_tester->m_pTempInterface;
    m_tester->m_pTempInterface = nullptr;
}

TEST_F(UT_ArchiveManager, test_addFiles_002)
{
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->addFiles("1.zip", QList<FileEntry>(), CompressOptions()), false);
}

TEST_F(UT_ArchiveManager, test_extractFiles_001)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    JobStub::stub_SingleJob_start(stub);
    JobStub::stub_StepExtractJob_start(stub);

    SAFE_DELETE_ELE(m_tester->m_pInterface);
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    ExtractionOptions option;
    option.bTar_7z = false;
    EXPECT_EQ(m_tester->extractFiles("1.zip", QList<FileEntry>(), option, UiTools::APT_Auto), true);
}

TEST_F(UT_ArchiveManager, test_extractFiles_002)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);
    JobStub::stub_StepExtractJob_start(stub);
    ExtractionOptions option;
    option.bTar_7z = true;
    EXPECT_EQ(m_tester->extractFiles("1.zip", QList<FileEntry>(), option, UiTools::APT_Auto), true);
}

TEST_F(UT_ArchiveManager, test_extractFiles_003)
{
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    JobStub::stub_SingleJob_start(stub);
    JobStub::stub_StepExtractJob_start(stub);
    ExtractionOptions option;
    option.bTar_7z = true;
    EXPECT_EQ(m_tester->extractFiles("1.zip", QList<FileEntry>(), option, UiTools::APT_Auto), false);
}

TEST_F(UT_ArchiveManager, test_extractFiles2Path_001)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->extractFiles2Path("1.zip", QList<FileEntry>(), ExtractionOptions()), true);
}

TEST_F(UT_ArchiveManager, test_extractFiles2Path_002)
{
    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, nullptr);
    EXPECT_EQ(m_tester->extractFiles2Path("1.zip", QList<FileEntry>(), ExtractionOptions()), false);
}

TEST_F(UT_ArchiveManager, test_deleteFiles_001)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->deleteFiles("1.zip", QList<FileEntry>()), true);
}

TEST_F(UT_ArchiveManager, test_deleteFiles_002)
{
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->deleteFiles("1.zip", QList<FileEntry>()), false);
}

TEST_F(UT_ArchiveManager, test_batchExtractFiles_001)
{
    g_batchExtractJob_setArchiveFiles_result = true;
    Stub stub;
    stub.set(ADDR(BatchExtractJob, setArchiveFiles), batchExtractJob_setArchiveFiles_stub);
    EXPECT_EQ(m_tester->batchExtractFiles(QStringList(), ""), true);

}

TEST_F(UT_ArchiveManager, test_batchExtractFiles_002)
{
    g_batchExtractJob_setArchiveFiles_result = false;
    Stub stub;
    stub.set(ADDR(BatchExtractJob, setArchiveFiles), batchExtractJob_setArchiveFiles_stub);
    EXPECT_EQ(m_tester->batchExtractFiles(QStringList(), ""), false);
}

TEST_F(UT_ArchiveManager, test_openFile_001)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->openFile("1.zip", FileEntry(), "", ""), true);
}

TEST_F(UT_ArchiveManager, test_openFile_002)
{
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->openFile("1.zip", FileEntry(), "", ""), false);
}

TEST_F(UT_ArchiveManager, test_updateArchiveCacheData_001)
{
    Stub stub;
    JobStub::stub_UpdateJob_start(stub);

    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    EXPECT_EQ(m_tester->updateArchiveCacheData(UpdateOptions()), true);
}

TEST_F(UT_ArchiveManager, test_updateArchiveCacheData_002)
{
    Stub stub;
    JobStub::stub_UpdateJob_start(stub);

    EXPECT_EQ(m_tester->updateArchiveCacheData(UpdateOptions()), false);
}

TEST_F(UT_ArchiveManager, test_updateArchiveComment_001)
{
    LibzipPlugin *pInterface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pInterface);
    JobStub::stub_SingleJob_start(stub);

    EXPECT_EQ(m_tester->updateArchiveComment("1.zip", ""), true);
}

TEST_F(UT_ArchiveManager, test_updateArchiveComment_002)
{
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->updateArchiveComment("1.zip", ""), false);
}

TEST_F(UT_ArchiveManager, test_convertArchive)
{
    Stub stub;
    JobStub::stub_ConvertJob_start(stub);

    EXPECT_EQ(m_tester->convertArchive("", "", ""), true);
}

TEST_F(UT_ArchiveManager, test_pauseOperation_001)
{
    Stub stub;
    JobStub::stub_SingleJob_doPause(stub, true);

    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    EXPECT_EQ(m_tester->pauseOperation(), true);
}

TEST_F(UT_ArchiveManager, test_pauseOperation_002)
{
    EXPECT_EQ(m_tester->pauseOperation(), false);
}

TEST_F(UT_ArchiveManager, test_continueOperation_001)
{
    Stub stub;
    JobStub::stub_SingleJob_doContinue(stub, true);

    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    EXPECT_EQ(m_tester->continueOperation(), true);
}

TEST_F(UT_ArchiveManager, test_continueOperation_002)
{
    EXPECT_EQ(m_tester->continueOperation(), false);
}

TEST_F(UT_ArchiveManager, test_cancelOperation_001)
{
    Stub stub;
    JobStub::stub_ArchiveJob_kill(stub);

    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    EXPECT_EQ(m_tester->cancelOperation(), true);
}

TEST_F(UT_ArchiveManager, test_cancelOperation_002)
{
    EXPECT_EQ(m_tester->cancelOperation(), false);
}

TEST_F(UT_ArchiveManager, test_getCurFilePassword_001)
{
    m_tester->m_pInterface = new LibzipPlugin(nullptr, QVariantList());
    m_tester->m_pInterface->m_strPassword = "123";
    EXPECT_EQ(m_tester->getCurFilePassword(), "123");
}

TEST_F(UT_ArchiveManager, test_getCurFilePassword_002)
{
    EXPECT_EQ(m_tester->getCurFilePassword(), "");
}

TEST_F(UT_ArchiveManager, test_slotJobFinished)
{
    m_tester->m_pArchiveJob = new LoadJob(nullptr, nullptr);
    m_tester->slotJobFinished();
    EXPECT_EQ(m_tester->m_pArchiveJob, nullptr);
}
