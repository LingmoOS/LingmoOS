// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "calculatesizethread.h"
#include "ut_commonstub.h"

#include "gtest/src/stub.h"
#include <gtest/gtest.h>
#include <QDir>

/*******************************函数打桩************************************/



/*******************************单元测试************************************/
// 测试CompressPage
class UT_CalculateSizeThread : public ::testing::Test
{
public:
    UT_CalculateSizeThread(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QStringList m_files{ "a.txt", "b.txt", "c.txt" }; // 添加的源文件数据（首层）
        QString m_strArchiveFullPath("/a/b/c/");       // 压缩包全路径（首层）

        FileEntry a;
        QList<FileEntry> m_listAddEntry{a};  // 添加的entry数据
        CompressOptions m_stOptions;        // 压缩参数
        QList<FileEntry> m_listAllEntry;    // 所有文件数据

        m_tester = new CalculateSizeThread(m_files, m_strArchiveFullPath, m_listAddEntry, m_stOptions);
        m_tester->disconnect();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    CalculateSizeThread *m_tester;
};

TEST_F(UT_CalculateSizeThread, initTest)
{

}

TEST_F(UT_CalculateSizeThread, test_set_thread_stop)
{
    m_tester->set_thread_stop(true);
    EXPECT_EQ(m_tester->m_thread_stop, true);
}

TEST_F(UT_CalculateSizeThread, test_run_001)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    m_tester->run();
    EXPECT_EQ(m_tester->m_qTotalSize, 3);
    EXPECT_EQ(m_tester->m_thread_stop, false);
}

TEST_F(UT_CalculateSizeThread, test_run_002)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    m_tester->run();
    EXPECT_EQ(m_tester->m_thread_stop, true);
}

TEST_F(UT_CalculateSizeThread, test_run_003)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, false);
    m_tester->m_thread_stop = false;
    m_tester->run();
    EXPECT_EQ(m_tester->m_thread_stop, true);
}

TEST_F(UT_CalculateSizeThread, test_run_004)
{
    Stub stub;
    QElapsedTimerStub::stub_QElapsedTimer_start(stub);
    QElapsedTimerStub::stub_QElapsedTimer_elapsed(stub, 1);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);
    m_tester->m_thread_stop = false;
    m_tester->run();
    EXPECT_EQ(m_tester->m_thread_stop, true);
}

TEST_F(UT_CalculateSizeThread, test_ConstructAddOptionsByThread_001)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/a/b/1.txt");
    QFileInfo fi1;
    QFileInfoList filist{fi1};
    QDirStub::stub_QDir_exists(stub, true);
    QDirStub::stub_QDir_entryInfoList(stub, filist);

    m_tester->ConstructAddOptionsByThread("/a/b/");
    EXPECT_EQ(m_tester->m_qTotalSize, 1);
    EXPECT_EQ(m_tester->m_thread_stop, false);
}

TEST_F(UT_CalculateSizeThread, test_ConstructAddOptionsByThread_002)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/a/b/1.txt");
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, true);
    m_tester->ConstructAddOptionsByThread("/a/b/");
    EXPECT_EQ(m_tester->m_thread_stop, false);
}

TEST_F(UT_CalculateSizeThread, test_ConstructAddOptionsByThread_003)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isReadable(stub, true);
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/a/b/1.txt");
    QFileInfoStub::stub_QFileInfo_exists(stub, false);
    QFileInfoStub::stub_QFileInfo_isSymLink(stub, false);
    m_tester->m_thread_stop = false;
    m_tester->ConstructAddOptionsByThread("/a/b/");
    EXPECT_EQ(m_tester->m_thread_stop, false);
}

TEST_F(UT_CalculateSizeThread, test_ConstructAddOptionsByThread_004)
{
    Stub stub;
    QFileInfoStub::stub_QFileInfo_isDir(stub, false);
    QFileInfoStub::stub_QFileInfo_size(stub, 1);
    QFileInfoStub::stub_QFileInfo_fileName(stub, "1.txt");
    QFileInfoStub::stub_QFileInfo_filePath(stub, "/a/b/1.txt");
    QFileInfoStub::stub_QFileInfo_exists(stub, true);
    QFileInfoStub::stub_QFileInfo_isReadable(stub, false);
    m_tester->m_thread_stop = false;
    m_tester->ConstructAddOptionsByThread("/a/b/");
    EXPECT_EQ(m_tester->m_thread_stop, false);
}
