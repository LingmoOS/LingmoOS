// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "logapplicationparsethread.h"
#include "logfileparser.h"
#include "structdef.h"
#include "dbusproxy/dldbushandler.h"

#include <stub.h>

#include <QDebug>
#include <QDir>
#include <QFileInfo>

#include <gtest/gtest.h>

QFileInfoList stub_entryInfoList(QDir::Filters filters, QDir::SortFlags sort)
{
    QFileInfo fileInfo("test.txt");
    QList<QFileInfo> m_list {fileInfo};
    return m_list;
}

QStringList stub_split(QChar sep, QString::SplitBehavior behavior,
                       Qt::CaseSensitivity cs)
{
    return QStringList() << "t.em,s[.a]da[],m[.s]d,s[t.][,"
                         << "jsd[sa[dds.,]"
                         << "ds[ds..,/n"
                         << "test2.[df,f";
}

QString stub_readAppLog(const QString &filePath)
{
    Q_UNUSED(filePath);
    return "2021-04-06 13:29:32 install code:amd64 <none> 1.55.0-1617120720";
}

QStringList stub_getAppFileInfo(const QString &flag)
{
    Q_UNUSED(flag);
    return QStringList() << "test";
}

QByteArray stub_readAllStandardOutput(){
    return "2021-03-10, 11:33:23.48.9 [Warning] [                                                         0] QFSFileEngine::open: No file name specified";
}

class LogApplicationParseThread_UT : public testing::Test
{
public:
    //添加日志
    static void SetUpTestCase()
    {
        qDebug() << "SetUpTestCase" << endl;
    }
    static void TearDownTestCase()
    {
        qDebug() << "TearDownTestCase" << endl;
    }
    void SetUp() //TEST跑之前会执行SetUp
    {
        m_logAppThread = new LogApplicationParseThread();
        qDebug() << "SetUp" << endl;
    }
    void TearDown() //TEST跑完之后会执行TearDown
    {
        delete m_logAppThread;
    }
    LogApplicationParseThread *m_logAppThread;
};

TEST_F(LogApplicationParseThread_UT, UT_InitProccess_001){
    m_logAppThread->initProccess();
    EXPECT_NE(m_logAppThread, nullptr);
    EXPECT_NE(m_logAppThread->m_process, nullptr);
}

TEST_F(LogApplicationParseThread_UT, UT_OnProcFinished_001){
    m_logAppThread->onProcFinished(1);
    EXPECT_NE(m_logAppThread, nullptr);
}

TEST_F(LogApplicationParseThread_UT, UT_SetParam_001){
    APP_FILTERS appfilter;
    APP_FILTERSList appfilters;
    appfilters << appfilter;
    m_logAppThread->setFilters(appfilters);
    EXPECT_NE(m_logAppThread, nullptr);
    EXPECT_EQ(m_logAppThread->m_AppFiler.path, appfilter.path);
}

TEST_F(LogApplicationParseThread_UT, UT_DoWork_001)
{
    Stub stub;
    stub.set((QList<QFileInfo>(QDir::*)(QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList), stub_entryInfoList);
    stub.set(ADDR(DLDBusHandler, getFileInfo), stub_getAppFileInfo);
    stub.set(ADDR(DLDBusHandler, readLog), stub_readAppLog);
    stub.set(ADDR(QProcess, readAllStandardOutput), stub_readAllStandardOutput);
    EXPECT_NE(m_logAppThread, nullptr);
    m_logAppThread->m_AppFiler.path = "";
    m_logAppThread->doWork();
    m_logAppThread->m_AppFiler.path = "test";
    m_logAppThread->doWork();
    m_logAppThread->m_AppFiler.lvlFilter=-1;
    m_logAppThread->doWork();
    EXPECT_NE(m_logAppThread->m_process, nullptr);
    EXPECT_EQ(m_logAppThread->m_appList.count(),1);
}

TEST_F(LogApplicationParseThread_UT, stopProcess_UT)
{
    Stub stub;
    stub.set((QList<QFileInfo>(QDir::*)(QDir::Filters, QDir::SortFlags) const)ADDR(QDir, entryInfoList), stub_entryInfoList);
    stub.set((QStringList(QString::*)(QChar, QString::SplitBehavior, Qt::CaseSensitivity) const)ADDR(QString, split), stub_split);
    stub.set(ADDR(DLDBusHandler, getFileInfo), stub_getAppFileInfo);
    stub.set(ADDR(DLDBusHandler, readLog), stub_readAppLog);
    m_logAppThread->stopProccess();
    EXPECT_EQ(m_logAppThread->m_process, nullptr);

}

TEST_F(LogApplicationParseThread_UT, UT_GetIndex_001){
    int index= m_logAppThread->getIndex();
    EXPECT_EQ(index, 6);
}
