// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "batchjob.h"
#include "singlejob.h"
#include "uitools.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/
bool batchExtractJob_addExtractItem_stub(const QFileInfo &)
{
    return true;
}
/*******************************函数打桩************************************/

// 测试BatchExtractJob
class UT_BatchExtractJob : public ::testing::Test
{
public:
    UT_BatchExtractJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new BatchExtractJob;
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    BatchExtractJob *m_tester;
};

TEST_F(UT_BatchExtractJob, initTest)
{

}

TEST_F(UT_BatchExtractJob, test_addSubjob_001)
{
    EXPECT_EQ(m_tester->addSubjob(nullptr), false);
}

TEST_F(UT_BatchExtractJob, test_addSubjob_002)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    EXPECT_EQ(m_tester->addSubjob(pJob), true);
}

TEST_F(UT_BatchExtractJob, test_removeSubjob_001)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    EXPECT_EQ(m_tester->removeSubjob(pJob), true);
}

TEST_F(UT_BatchExtractJob, test_removeSubjob_002)
{
    EXPECT_EQ(m_tester->removeSubjob(nullptr), false);
}

TEST_F(UT_BatchExtractJob, test_hasSubjobs)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    EXPECT_EQ(m_tester->hasSubjobs(), true);
}

TEST_F(UT_BatchExtractJob, test_subjobs)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    EXPECT_EQ(m_tester->subjobs(), m_tester->m_listSubjobs);
}

TEST_F(UT_BatchExtractJob, test_clearSubjobs)
{
    ExtractJob *pJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(pJob);
    m_tester->clearSubjobs();
    EXPECT_EQ(m_tester->m_listSubjobs.isEmpty(), true);
}

TEST_F(UT_BatchExtractJob, test_doPause)
{
    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->doPause();
    EXPECT_NE(m_tester->m_pCurJob, nullptr);
}

TEST_F(UT_BatchExtractJob, test_doContinue)
{
    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->doContinue();
    EXPECT_NE(m_tester->m_pCurJob, nullptr);
}

TEST_F(UT_BatchExtractJob, test_start_001)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    EXPECT_EQ(m_tester->m_iCurArchiveIndex, 0);
}

TEST_F(UT_BatchExtractJob, test_start_002)
{
//    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
//    m_tester->addSubjob(m_tester->m_pCurJob);
//    m_tester->start();

//    EXPECT_EQ(m_tester->m_iCurArchiveIndex, 0);
}

TEST_F(UT_BatchExtractJob, test_setExtractPath)
{
    m_tester->setExtractPath("/home/Desktop");
    EXPECT_EQ(m_tester->m_strExtractPath, "/home/Desktop");
}

TEST_F(UT_BatchExtractJob, test_setArchiveFiles)
{
    Stub stub;
    stub.set(ADDR(BatchExtractJob, addExtractItem), batchExtractJob_addExtractItem_stub);
    bool b = m_tester->setArchiveFiles(QStringList() << "1.zip" << "2.zip");
    EXPECT_EQ(b, true);
}

TEST_F(UT_BatchExtractJob, test_addExtractItem_001)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);

    EXPECT_EQ(m_tester->addExtractItem(QFileInfo("1.zip")), true);
}

TEST_F(UT_BatchExtractJob, test_addExtractItem_002)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    EXPECT_EQ(m_tester->addExtractItem(QFileInfo("1.tar.7z")), true);
}

TEST_F(UT_BatchExtractJob, test_slotHandleSingleJobProgress)
{
    m_tester->m_listFiles << "1.txt";
    m_tester->m_qBatchTotalSize = 1024;
    m_tester->m_iCurArchiveIndex = 0;
    m_tester->slotHandleSingleJobProgress(50);
}

TEST_F(UT_BatchExtractJob, test_slotHandleSingleJobCurFileName)
{
    m_tester->slotHandleSingleJobCurFileName("1.txt");
}

TEST_F(UT_BatchExtractJob, test_slotHandleSingleJobFinished_001)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);

    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);

    m_tester->m_pCurJob->m_eFinishedType = PFT_Error;
    m_tester->slotHandleSingleJobFinished();
}

TEST_F(UT_BatchExtractJob, test_slotHandleSingleJobFinished_002)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);

    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);

    m_tester->m_pCurJob->m_eFinishedType = PFT_Nomral;
    m_tester->slotHandleSingleJobFinished();
}

TEST_F(UT_BatchExtractJob, test_slotHandleSingleJobFinished_003)
{
    Stub stub;
    JobStub::stub_SingleJob_start(stub);

    m_tester->m_pCurJob = new ExtractJob(QList<FileEntry>(), nullptr, ExtractionOptions());
    m_tester->addSubjob(m_tester->m_pCurJob);
    m_tester->m_pCurJob->m_eFinishedType = PFT_Nomral;
    m_tester->m_iCurArchiveIndex = 0;
    m_tester->m_listFiles << "1.txt" << "2.txt";
    m_tester->slotHandleSingleJobFinished();
}

TEST_F(UT_BatchExtractJob, test_doKill)
{
    EXPECT_EQ(m_tester->doKill(), false);
}
