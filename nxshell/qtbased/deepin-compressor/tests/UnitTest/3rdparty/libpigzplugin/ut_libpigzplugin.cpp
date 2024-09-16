// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libpigzplugin.h"
#include "gtest/src/stub.h"
#include "queries.h"
#include "datamanager.h"

#include <gtest/gtest.h>

#include <QFileInfo>
#include <QMimeDatabase>
#include <QDir>
#include <QDebug>

Q_DECLARE_METATYPE(KPluginMetaData)

bool g_QProcess_waitForStarted_result = false;

class UT_LibPigzPluginFactory : public QObject, public ::testing::Test
{
public:
    UT_LibPigzPluginFactory(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new LibPigzPluginFactory();
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibPigzPluginFactory *m_tester;
};

class UT_LibPigzPlugin : public QObject, public ::testing::Test
{
public:
    UT_LibPigzPlugin(): m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        QString strFile  = _UTSOURCEDIR;
        strFile += "/test_sources/zip/extract/test.zip";
        KPluginMetaData data;
        QMimeDatabase db;
        QMimeType mimeFromContent = db.mimeTypeForFile(strFile, QMimeDatabase::MatchContent);
        const QVariantList args = {QVariant(strFile),
                                   QVariant().fromValue(data),
                                   QVariant::fromValue(mimeFromContent)
                                  };

        m_tester = new LibPigzPlugin(this, args);
    }

    virtual void TearDown()
    {
        delete m_tester;
    }

protected:
    LibPigzPlugin *m_tester;
};


TEST_F(UT_LibPigzPluginFactory, initTest)
{

}

TEST_F(UT_LibPigzPlugin, initTest)
{

}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_list)
{
    EXPECT_EQ(m_tester->list(), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_testArchive)
{
    EXPECT_EQ(m_tester->testArchive(), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_extractFiles)
{
    EXPECT_EQ(m_tester->testArchive(), PFT_Nomral);
}

void kProcess_start_stub()
{
    return ;
}

bool qProcess_waitForStarted_stub(int)
{
    return g_QProcess_waitForStarted_result;
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_addFiles)
{
    Stub stub;
    stub.set(ADDR(KProcess, start), kProcess_start_stub);
    stub.set(ADDR(QProcess, waitForStarted), qProcess_waitForStarted_stub);
    g_QProcess_waitForStarted_result = true;

    QList<FileEntry> listEntry;
    CompressOptions options;
    FileEntry entry;
    entry.strFullPath = "/home/1.txt";
    listEntry << entry;
    EXPECT_EQ(m_tester->addFiles(listEntry, options), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_moveFiles)
{
    CompressOptions options;
    EXPECT_EQ(m_tester->moveFiles(QList<FileEntry>(), options), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_copyFiles)
{
    CompressOptions options;
    EXPECT_EQ(m_tester->copyFiles(QList<FileEntry>(), options), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_deleteFiles)
{
    EXPECT_EQ(m_tester->deleteFiles(QList<FileEntry>()), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_addComment)
{
    EXPECT_EQ(m_tester->addComment("sssss"), PFT_Nomral);
}

TEST_F(UT_LibPigzPlugin, test_updateArchiveData)
{
    UpdateOptions options;
    EXPECT_EQ(m_tester->updateArchiveData(options), PFT_Nomral);
}

int kill_stub(__pid_t, int)
{
    return 0;
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_pauseOperation)
{
    Stub stub;
    stub.set(kill, kill_stub);
    m_tester->m_childProcessId << 123456 << 234567;
    m_tester->m_processId = 012345;
    m_tester->pauseOperation();
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_continueOperation)
{
    Stub stub;
    stub.set(kill, kill_stub);
    m_tester->m_childProcessId << 123456 << 234567;
    m_tester->m_processId = 012345;
    m_tester->continueOperation();
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_doKill_001)
{
    m_tester->m_process = new KPtyProcess;
    EXPECT_EQ(m_tester->doKill(), true);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_doKill_002)
{
    m_tester->m_process = nullptr;
    EXPECT_EQ(m_tester->doKill(), false);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_handleLine_001)
{
    m_tester->m_process = new KPtyProcess;
    EXPECT_EQ(m_tester->handleLine("No space left on device"), false);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_handleLine_002)
{
    m_tester->m_process = new KPtyProcess;
    EXPECT_EQ(m_tester->handleLine("121"), true);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_killProcess_001)
{
    m_tester->m_process = nullptr;
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_process, nullptr);
}

qint64 kProcess_processId_stub()
{
    return 123456;
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_killProcess_002)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->m_childProcessId << 123456 << 234567;
    Stub stub;
    stub.set(kill, kill_stub);
    stub.set(ADDR(KProcess, processId), kProcess_processId_stub);
    m_tester->killProcess(true);
    EXPECT_EQ(m_tester->m_isProcessKilled, false);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_deleteProcess)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->deleteProcess();
    EXPECT_EQ(m_tester->m_process, nullptr);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_processFinished_001)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->processFinished(0, QProcess::NormalExit);
    EXPECT_EQ(m_tester->m_process, nullptr);
}

TEST_F(UT_LibPigzPlugin, UT_LibPigzPlugin_processFinished_002)
{
    m_tester->m_process = new KPtyProcess;
    m_tester->processFinished(28, QProcess::NormalExit);
    EXPECT_EQ(m_tester->m_process, nullptr);
}
