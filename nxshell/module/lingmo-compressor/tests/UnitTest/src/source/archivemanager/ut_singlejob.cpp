// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "singlejob.h"
#include "libzipplugin.h"
#include "commonstruct.h"
#include "uitools.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"

#include <gtest/gtest.h>
#include <QTest>

/*******************************函数打桩************************************/
bool libzipPlugin_doKill_stub()
{
    return true;
}

//bool qThread_isRunning_stub()
//{
//    return true;
//}

void qThread_requestInterruption_stub()
{
    return;
}

/*******************************函数打桩************************************/
// 测试SingleJobThread
class UT_SingleJobThread : public ::testing::Test
{
public:
    UT_SingleJobThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_pJob = new LoadJob(m_pInterface, nullptr);
        m_tester = new SingleJobThread(m_pJob);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_pJob;
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    LoadJob *m_pJob;
    SingleJobThread *m_tester;
};

TEST_F(UT_SingleJobThread, initTest)
{

}

TEST_F(UT_SingleJobThread, testrun)
{
    m_tester->run();
}

// 测试LoadJob
class UT_LoadJob : public ::testing::Test
{
public:
    UT_LoadJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new LoadJob(m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    LoadJob *m_tester;
};

TEST_F(UT_LoadJob, initTest)
{

}

TEST_F(UT_LoadJob, test_start_001)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    m_tester->m_pInterface->m_bWaitForFinished = true;
    m_tester->start();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_LoadJob, test_start_002)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    m_tester->m_pInterface->m_bWaitForFinished = false;
    m_tester->start();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_LoadJob, test_start_003)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    m_pInterface = nullptr;
    m_tester->start();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Error);
}

TEST_F(UT_LoadJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_LoadJob, test_doPause)
{
    m_tester->doPause();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_LoadJob, test_doContinue)
{
    m_tester->doContinue();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_LoadJob, test_getdptr)
{
    EXPECT_EQ(m_tester->getdptr(), m_tester->d);
}

TEST_F(UT_LoadJob, test_doKill_001)
{
    SAFE_DELETE_ELE(m_tester->m_pInterface);
    m_pInterface = nullptr;
    EXPECT_EQ(m_tester->doKill(), false);
}

TEST_F(UT_LoadJob, test_doKill_002)
{
    Stub stub;
    typedef bool (*fptr)(LibzipPlugin *);
    fptr A_foo = (fptr)(&LibzipPlugin::doKill);   //获取虚函数地址
    stub.set(A_foo, libzipPlugin_doKill_stub);
    EXPECT_EQ(m_tester->doKill(), true);
}

//TEST_F(UT_LoadJob, test_doKill_003)
//{
//    Stub stub;
//    CommonStub::stub_QThread_isRunning(stub, true);
////    stub.set(ADDR(QThread, isRunning), qThread_isRunning_stub);
//    stub.set(ADDR(QThread, requestInterruption), qThread_requestInterruption_stub);
//    stub.set(ADDR(QThread, wait), qThread_wait_stub);
//    EXPECT_EQ(m_tester->doKill(), true);
//}

TEST_F(UT_LoadJob, test_finishJob)
{
    m_tester->finishJob();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_LoadJob, testkill)
{
//    m_tester->kill();
//    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Cancel);
//    EXPECT_EQ(m_tester->m_eErrorType, ET_UserCancelOpertion);
}


// 测试AddJob
class UT_AddJob : public ::testing::Test
{
public:
    UT_AddJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new AddJob(QList<FileEntry>(), m_pInterface, CompressOptions(), nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    AddJob *m_tester;
};

TEST_F(UT_AddJob, initTest)
{

}

TEST_F(UT_AddJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}


// 测试AddJob
class UT_CreateJob : public ::testing::Test
{
public:
    UT_CreateJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new CreateJob(QList<FileEntry>(), m_pInterface, CompressOptions(), nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    CreateJob *m_tester;
};

TEST_F(UT_CreateJob, initTest)
{

}

TEST_F(UT_CreateJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_CreateJob, test_doKill)
{
    EXPECT_EQ(m_tester->doKill(), true);
}

TEST_F(UT_CreateJob, test_cleanCompressFileCancel_001)
{
    m_tester->m_stCompressOptions.bSplit = true;
    m_tester->cleanCompressFileCancel();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_CreateJob, test_cleanCompressFileCancel_002)
{
    m_tester->m_stCompressOptions.bSplit = false;
    m_tester->cleanCompressFileCancel();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}


// 测试ExtractJob
class UT_ExtractJob : public ::testing::Test
{
public:
    UT_ExtractJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new ExtractJob(QList<FileEntry>(), m_pInterface, ExtractionOptions(), nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    ExtractJob *m_tester;
};

TEST_F(UT_ExtractJob, initTest)
{

}

TEST_F(UT_ExtractJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Error);
}


// 测试DeleteJob
class UT_DeleteJob : public ::testing::Test
{
public:
    UT_DeleteJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new DeleteJob(QList<FileEntry>(), m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    DeleteJob *m_tester;
};

TEST_F(UT_DeleteJob, initTest)
{

}

TEST_F(UT_DeleteJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Error);
}


// 测试OpenJob
class UT_OpenJob : public ::testing::Test
{
public:
    UT_OpenJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new OpenJob(FileEntry(), "", "", m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    OpenJob *m_tester;
};

TEST_F(UT_OpenJob, initTest)
{

}

TEST_F(UT_OpenJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Error);
}

TEST_F(UT_OpenJob, test_slotFinished)
{
    Stub stub;
    CommonStub::stub_ProcessOpenThread_start(stub);
    m_tester->slotFinished(PFT_Nomral);
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Nomral);
}


// 测试UpdateJob
class UT_UpdateJob : public ::testing::Test
{
public:
    UT_UpdateJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new UpdateJob(UpdateOptions(), m_pInterface, nullptr);
    }

    virtual void TearDown()
    {
        SAFE_DELETE_ELE(m_pInterface);
        delete m_tester;
    }

protected:
    LibzipPlugin *m_pInterface = nullptr;
    UpdateJob *m_tester;
};

TEST_F(UT_UpdateJob, initTest)
{

}

TEST_F(UT_UpdateJob, test_start)
{
    Stub stub;
    CommonStub::stub_QThread_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    m_tester->start();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_UpdateJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}


// 测试CommentJob
class UT_CommentJob : public ::testing::Test
{
public:
    UT_CommentJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        LibzipPlugin *pInterface = new LibzipPlugin(m_tester, QVariantList());
        m_tester = new CommentJob("", pInterface, nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CommentJob *m_tester;
};

TEST_F(UT_CommentJob, initTest)
{

}

TEST_F(UT_CommentJob, test_doWork)
{
    m_tester->doWork();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_CommentJob, test_doPause)
{
    m_tester->doPause();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_CommentJob, test_doContinue)
{
    m_tester->doContinue();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}

TEST_F(UT_CommentJob, test_doCancel)
{
    m_tester->doCancel();
    EXPECT_NE(m_tester->m_pInterface, nullptr);
}


// 测试ConvertJob
class UT_ConvertJob : public ::testing::Test
{
public:
    UT_ConvertJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ConvertJob("", "./", "", nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    ConvertJob *m_tester;
};

TEST_F(UT_ConvertJob, initTest)
{

}

TEST_F(UT_ConvertJob, test_start)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    EXPECT_EQ(m_tester->m_pIface, pIface);
    EXPECT_EQ(m_tester->m_iStepNo, 0);
}

TEST_F(UT_ConvertJob, testslotHandleExtractFinished)
{
////    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(nullptr, QVariantList());
//    Stub stub;
////    CommonStub::stub_UiTools_createInterface(stub, pIface);
//    JobStub::stub_CreateJob_doWork(stub);
//    JobStub::stub_ExtractJob_doWork(stub);
//    JobStub::stub_SingleJob_start(stub);
//    m_tester->start();
//    m_tester->m_pExtractJob->m_eFinishedType = PluginFinishType::PFT_Nomral;
////    ReadOnlyArchiveInterface *pIface1 = new LibzipPlugin(nullptr, QVariantList());
//    Stub stub1;
////    CommonStub::stub_UiTools_createInterface(stub1, pIface1);
//    m_tester->slotHandleExtractFinished();
//    SAFE_DELETE_ELE(m_tester->m_pIface);
//    m_tester->m_pExtractJob->m_eFinishedType = PFT_Cancel;
//    m_tester->slotHandleExtractFinished();
//    SAFE_DELETE_ELE(m_tester->m_pIface);
//    m_tester->m_pExtractJob->m_eFinishedType = PFT_Error;
//    m_tester->slotHandleExtractFinished();
//    SAFE_DELETE_ELE(m_tester->m_pIface);
}

TEST_F(UT_ConvertJob, test_doPause)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    m_tester->doPause();
    EXPECT_NE(m_tester->m_pIface, nullptr);
}

TEST_F(UT_ConvertJob, test_doContinue)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    m_tester->doContinue();
    EXPECT_NE(m_tester->m_pIface, nullptr);
}

TEST_F(UT_ConvertJob, test_doKill)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(nullptr, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_ExtractJob_doWork(stub);
    m_tester->start();
    m_tester->doKill();
    EXPECT_NE(m_tester->m_pIface, nullptr);
}


// 测试StepExtractJob
class UT_StepExtractJob : public ::testing::Test
{
public:
    UT_StepExtractJob(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new StepExtractJob("", ExtractionOptions(), nullptr);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    StepExtractJob *m_tester;
};

TEST_F(UT_StepExtractJob, initTest)
{

}

TEST_F(UT_StepExtractJob, test_start)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    EXPECT_EQ(m_tester->m_pIface, pIface);
    EXPECT_EQ(m_tester->m_iStepNo, 0);
}

TEST_F(UT_StepExtractJob, test_slotHandleExtractFinished_001)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pExtractJob->m_eFinishedType = PluginFinishType::PFT_Nomral;
    ReadOnlyArchiveInterface *pIface1 = new LibzipPlugin(m_tester, QVariantList());
    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, pIface1);
    m_tester->slotHandleExtractFinished();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Nomral);
}

TEST_F(UT_StepExtractJob, test_slotHandleExtractFinished_002)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pExtractJob->m_eFinishedType = PluginFinishType::PFT_Nomral;
    ReadOnlyArchiveInterface *pIface1 = new LibzipPlugin(m_tester, QVariantList());
    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, pIface1);
    m_tester->m_pExtractJob->m_eFinishedType = PFT_Cancel;
    m_tester->slotHandleExtractFinished();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Cancel);
}

TEST_F(UT_StepExtractJob, test_slotHandleExtractFinished_003)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pExtractJob->m_eFinishedType = PluginFinishType::PFT_Nomral;
    ReadOnlyArchiveInterface *pIface1 = new LibzipPlugin(m_tester, QVariantList());
    Stub stub1;
    CommonStub::stub_UiTools_createInterface(stub1, pIface1);
    m_tester->m_pExtractJob->m_eFinishedType = PFT_Error;
    m_tester->slotHandleExtractFinished();
    EXPECT_EQ(m_tester->m_eFinishedType, PFT_Error);
}

TEST_F(UT_StepExtractJob, test_doKill)
{
    ReadOnlyArchiveInterface *pIface = new LibzipPlugin(m_tester, QVariantList());
    Stub stub;
    CommonStub::stub_UiTools_createInterface(stub, pIface);
    JobStub::stub_SingleJob_start(stub);
    m_tester->start();
    m_tester->m_pExtractJob2 = new ExtractJob(QList<FileEntry>(), pIface, ExtractionOptions(), m_tester);
    EXPECT_EQ(m_tester->doKill(), true);
}


