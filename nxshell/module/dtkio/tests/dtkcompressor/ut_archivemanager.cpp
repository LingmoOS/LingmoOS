// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "ut_stub.h"

DCOMPRESSOR_USE_NAMESPACE

class UT_DArchiveManager : public ::testing::Test
{
public:
    UT_DArchiveManager()
        : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = DArchiveManager::get_instance();
    }

    virtual void TearDown()
    {
        m_tester->destory_instance();
    }

protected:
    DArchiveManager *m_tester = nullptr;
};

TEST_F(UT_DArchiveManager, test_createArchive)
{
    Stub stub;
    stub_SingleJob_start(stub);
    stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->createArchive(QList<DArchiveManager::MgrFileEntry>(), "/home/Desktop", DArchiveManager::MgrCompressOptions(), DArchiveManager::APT_Auto), false);
}

TEST_F(UT_DArchiveManager, test_loadArchive)
{
    Stub stub;
    stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->loadArchive("1.zip", DArchiveManager::APT_Auto), false);
}

TEST_F(UT_DArchiveManager, test_addFiles)
{
    Stub stub;
    stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->addFiles("1.zip", QList<DArchiveManager::MgrFileEntry>(), DArchiveManager::MgrCompressOptions()), false);
}

TEST_F(UT_DArchiveManager, test_extractFiles)
{
    Stub stub;
    stub_UiTools_createInterface(stub, nullptr);
    stub_SingleJob_start(stub);
    stub_StepExtractJob_start(stub);
    DArchiveManager::MgrExtractionOptions option;
    option.bTar_7z = true;
    EXPECT_EQ(m_tester->extractFiles("1.zip", QList<DArchiveManager::MgrFileEntry>(), option, DArchiveManager::APT_Auto), false);
}

TEST_F(UT_DArchiveManager, test_extractFiles2Path)
{
    Stub stub1;
    stub_UiTools_createInterface(stub1, nullptr);
    EXPECT_EQ(m_tester->extractFiles2Path("1.zip", QList<DArchiveManager::MgrFileEntry>(), DArchiveManager::MgrExtractionOptions()), false);
}

TEST_F(UT_DArchiveManager, test_deleteFiles)
{
    Stub stub;
    stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->deleteFiles("1.zip", QList<DArchiveManager::MgrFileEntry>()), false);
}

TEST_F(UT_DArchiveManager, test_batchExtractFiles_001)
{
    Stub stub;
    stub_batchExtractJob_setArchiveFiles(stub, true);
    EXPECT_EQ(m_tester->batchExtractFiles(QStringList(), ""), true);
}

TEST_F(UT_DArchiveManager, test_batchExtractFiles_002)
{
    Stub stub;
    stub_batchExtractJob_setArchiveFiles(stub, false);
    EXPECT_EQ(m_tester->batchExtractFiles(QStringList(), ""), false);
}

TEST_F(UT_DArchiveManager, test_openFile)
{
    Stub stub;
    stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->openFile("1.zip", DArchiveManager::MgrFileEntry(), "", ""), false);
}

TEST_F(UT_DArchiveManager, test_updateArchiveCacheData)
{
    Stub stub;
    stub_UpdateJob_start(stub);
    EXPECT_EQ(m_tester->updateArchiveCacheData(DArchiveManager::MgrUpdateOptions()), false);
}

TEST_F(UT_DArchiveManager, test_updateArchiveComment)
{
    Stub stub;
    stub_UiTools_createInterface(stub, nullptr);
    EXPECT_EQ(m_tester->updateArchiveComment("1.zip", ""), false);
}

TEST_F(UT_DArchiveManager, test_convertArchive)
{
    Stub stub;
    stub_ConvertJob_start(stub);

    EXPECT_EQ(m_tester->convertArchive("", "", ""), true);
}

TEST_F(UT_DArchiveManager, test_pauseOperation)
{
    EXPECT_EQ(m_tester->pauseOperation(), false);
}

TEST_F(UT_DArchiveManager, test_continueOperation)
{
    EXPECT_EQ(m_tester->continueOperation(), false);
}

TEST_F(UT_DArchiveManager, test_cancelOperation)
{
    EXPECT_EQ(m_tester->cancelOperation(), false);
}

TEST_F(UT_DArchiveManager, test_getCurFilePassword_002)
{
    EXPECT_EQ(m_tester->getCurFilePassword(), "");
}
