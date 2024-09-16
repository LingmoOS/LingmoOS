// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process.h"
#include "common/common.h"
#include "process/private/process_p.h"
//gtest
#include "stub.h"
#include <gtest/gtest.h>
//Qt
#include <QIcon>
#include <QApplication>
//system
#include <fcntl.h>
#include <unistd.h>

using namespace core::process;
using namespace common::alloc;
static QString m_Sresult;
/***************************************STUB begin*********************************************/
int stub_readStat_open1()
{
    m_Sresult = "open";
    return 1;
}
int stub_readStat_open2()
{
    m_Sresult = "open failed";
    return -1;
}
ssize_t stub_readStat_read1()
{
    m_Sresult = "read";
    return 1;
}
int stub_readStat_close()
{
    m_Sresult = "close";
    return 1;
}
ssize_t stub_readStat_read2()
{
    m_Sresult = "read failed";
    return -1;
}
FILE *stub_readCmdline_fopen()
{
    m_Sresult = "fopen failed";
    return nullptr;
}
uDir *stub_readSockInodes_opendir()
{
    m_Sresult = "diropen failed";
    return nullptr;
}
/***************************************STUB end**********************************************/
class UT_Process : public ::testing::Test
{
public:
    UT_Process() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {

        m_tester = new Process(getpid());
    }

    virtual void TearDown()
    {
        if (m_tester) {
            delete m_tester;
            m_tester = nullptr;
        }
    }

protected:
    Process *m_tester;
};

TEST_F(UT_Process, initTest)
{

}

TEST_F(UT_Process, test_isValid_001)
{
    bool valid = m_tester->isValid();
    EXPECT_FALSE(valid);
}

TEST_F(UT_Process, test_pid_001)
{
    m_tester->pid();
}

TEST_F(UT_Process, test_ppid_001)
{
    pid_t ppid = m_tester->ppid();
    EXPECT_EQ(ppid, m_tester->d->ppid);
}

TEST_F(UT_Process, test_utime_001)
{
    qulonglong utime = m_tester->utime();
    EXPECT_EQ(utime, m_tester->d->utime);
}

TEST_F(UT_Process, test_stime_001)
{
    qulonglong stime = m_tester->stime();
    EXPECT_EQ(stime, m_tester->d->stime);
}

TEST_F(UT_Process, test_name_001)
{
    QString name = m_tester->name();
    EXPECT_EQ(name, m_tester->d->name);
}

TEST_F(UT_Process, test_setName_001)
{
    QString name = "aaa";
    m_tester->setName(name);
    EXPECT_EQ(m_tester->d->name, name);
}

TEST_F(UT_Process, test_displayName_001)
{
    QString displayName = m_tester->displayName();
    EXPECT_EQ(m_tester->d->proc_name.displayName(), displayName);
}

TEST_F(UT_Process, test_icon_001)
{
    qint64 iconKey = m_tester->icon().cacheKey();
    EXPECT_EQ(m_tester->d->proc_icon.icon().cacheKey(), iconKey);
}

TEST_F(UT_Process, test_cpu_001)
{
    qreal cpu = m_tester->cpu(); struct uCloseDir {
        void operator()(DIR *dir)
        {
            closedir(dir);
        }
    };

    auto *sample = m_tester->d->cpuUsageSample->recentSample();
    if (sample)
        EXPECT_EQ(cpu, sample->data);
    else
        EXPECT_EQ(cpu, {});
}

TEST_F(UT_Process, test_memory_001)
{
    qulonglong memory = m_tester->memory();
    qulonglong expect = m_tester->d->rss - m_tester->d->shm;
    EXPECT_EQ(memory, expect);
}

TEST_F(UT_Process, test_vtrmemory_001)
{
    qulonglong vtrmemory = m_tester->vtrmemory();
    EXPECT_EQ(vtrmemory, m_tester->d->vmsize);
}

TEST_F(UT_Process, test_sharememory_001)
{
    qulonglong sharememory = m_tester->sharememory();
    EXPECT_EQ(sharememory, m_tester->d->shm);
}

TEST_F(UT_Process, test_priority_001)
{
    int priority = m_tester->priority();
    EXPECT_EQ(priority, m_tester->d->nice);
}

TEST_F(UT_Process, test_setPriority_001)
{
    int priority = 1;
    m_tester->setPriority(priority);
    EXPECT_EQ(1, m_tester->d->nice);
}

TEST_F(UT_Process, test_state_001)
{
    char state = m_tester->state();
    EXPECT_EQ(state, m_tester->d->state);
}

TEST_F(UT_Process, test_setState_001)
{
    char state = 'a';
    m_tester->setState(state);
    EXPECT_EQ(state, m_tester->d->state);
}

TEST_F(UT_Process, test_cmdline_001)
{
    QByteArrayList cmdline = m_tester->cmdline();
    EXPECT_EQ(cmdline, m_tester->d->cmdline);
}

TEST_F(UT_Process, test_cmdlineString_001)
{
    QString cmdlineString = m_tester->cmdlineString();
    QString expect = QUrl::fromPercentEncoding(m_tester->d->cmdline.join(' '));
    EXPECT_EQ(cmdlineString, expect);
}

TEST_F(UT_Process, test_environ_001)
{
    QHash<QString, QString> environ = m_tester->environ();

    EXPECT_EQ(environ, m_tester->d->environ);
}

TEST_F(UT_Process, test_uid_001)
{
    uid_t uid = m_tester->uid();

    EXPECT_EQ(uid, m_tester->d->uid);
}

TEST_F(UT_Process, test_userName_001)
{
    QString userName = m_tester->userName();

    EXPECT_EQ(userName, m_tester->d->usrerName);
}

TEST_F(UT_Process, test_gid_001)
{
    gid_t gid = m_tester->gid();

    EXPECT_EQ(gid, m_tester->d->gid);
}

TEST_F(UT_Process, test_groupName_001)
{
    QString groupName = m_tester->groupName();
    QString experct = SysInfo::groupName(m_tester->d->gid);
    EXPECT_EQ(groupName, experct);
}

TEST_F(UT_Process, test_readBps_001)
{
    qreal readBps = m_tester->readBps();
    auto *sample = m_tester->d->diskIOSpeedSample->recentSample();
    if (sample)
        EXPECT_EQ(readBps, sample->data.inBps);
    else
        EXPECT_EQ(readBps, {});

}

TEST_F(UT_Process, test_writeBps_001)
{
    qreal writeBps = m_tester->writeBps();
    auto *sample = m_tester->d->diskIOSpeedSample->recentSample();
    if (sample)
        EXPECT_EQ(writeBps, sample->data.outBps);
    else
        EXPECT_EQ(writeBps, {});
}

TEST_F(UT_Process, test_cancelledWriteBytes_001)
{
    qulonglong cancelledWriteBytes = m_tester->cancelledWriteBytes();
    EXPECT_EQ(cancelledWriteBytes, m_tester->d->cancelled_write_bytes);
}

TEST_F(UT_Process, test_recvBps_001)
{
    qreal recvBps = m_tester->recvBps();
    auto *sample = m_tester->d->networkBandwidthSample->recentSample();
    if (sample)
        EXPECT_EQ(recvBps, sample->data.inBps);
    else
        EXPECT_EQ(recvBps, 0);
}

TEST_F(UT_Process, test_sentBps_001)
{
    qreal sentBps = m_tester->sentBps();
    auto *sample = m_tester->d->networkBandwidthSample->recentSample();
    if (sample)
        EXPECT_EQ(sentBps, sample->data.outBps);
    else
        EXPECT_EQ(sentBps, 0);
}

TEST_F(UT_Process, test_recvBytes_001)
{
    qreal recvBytes = m_tester->recvBytes();
    auto *sample = m_tester->d->networkIOSample->recentSample();
    if (sample)
        EXPECT_EQ(recvBytes, sample->data.inBytes);
    else
        EXPECT_EQ(recvBytes, 0);
}

TEST_F(UT_Process, test_sentBytes_001)
{
    qreal sentBytes = m_tester->sentBytes();
    auto *sample = m_tester->d->networkIOSample->recentSample();
    if (sample)
        EXPECT_EQ(sentBytes, sample->data.outBytes);
    else
        EXPECT_EQ(sentBytes, 0);
}

TEST_F(UT_Process, test_appType_001)
{
    int appType = m_tester->appType();

    EXPECT_EQ(appType, m_tester->d->apptype);
}

TEST_F(UT_Process, test_setAppType_001)
{
    int appType = 1;
    m_tester->setAppType(appType);
    EXPECT_EQ(appType, m_tester->d->apptype);
}

TEST_F(UT_Process, test_procuptime_001)
{
    timeval procuptime = m_tester->procuptime();

    EXPECT_EQ(procuptime.tv_sec, m_tester->d->uptime.tv_sec);
    EXPECT_EQ(procuptime.tv_usec, m_tester->d->uptime.tv_usec);
}

TEST_F(UT_Process, test_getPriorityName_001)
{
    int prio = kVeryHighPriority;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "Very high"));
}

TEST_F(UT_Process, test_getPriorityName_002)
{
    int prio = kHighPriority;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "High"));
}

TEST_F(UT_Process, test_getPriorityName_003)
{
    int prio = kNormalPriority;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "Normal"));
}

TEST_F(UT_Process, test_getPriorityName_004)
{
    int prio = kLowPriority;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "Low"));
}

TEST_F(UT_Process, test_getPriorityName_005)
{
    int prio = kVeryLowPriority;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "Very low"));
}

TEST_F(UT_Process, test_getPriorityName_006)
{
    int prio = 5;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "Custom"));
}

TEST_F(UT_Process, test_getPriorityName_007)
{
    int prio = 50;
    QString PriorityName = getPriorityName(prio);
    EXPECT_EQ(PriorityName, QApplication::translate("Process.Priority", "Invalid"));
}

TEST_F(UT_Process, test_getProcessPriorityStub_001)
{
    int prio = 0;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(kNormalPriority, ProcessPriority);
}

TEST_F(UT_Process, test_getProcessPriorityStub_002)
{
    int prio = kVeryHighPriority;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(prio, ProcessPriority);
}

TEST_F(UT_Process, test_getProcessPriorityStub_003)
{
    int prio = kHighPriority;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(prio, ProcessPriority);
}

TEST_F(UT_Process, test_getProcessPriorityStub_004)
{
    int prio = kLowPriority;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(prio, ProcessPriority);
}

TEST_F(UT_Process, test_getProcessPriorityStub_005)
{
    int prio = kVeryLowPriority;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(prio, ProcessPriority);
}

TEST_F(UT_Process, test_getProcessPriorityStub_006)
{
    int prio = 5;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(kCustomPriority, ProcessPriority);
}

TEST_F(UT_Process, test_getProcessPriorityStub_007)
{
    int prio = 50;
    ProcessPriority ProcessPriority = getProcessPriorityStub(prio);
    EXPECT_EQ(kInvalidPriority, ProcessPriority);
}

TEST_F(UT_Process, test_readStat_001)
{
    Stub b1;
    b1.set(open, stub_readStat_open1);
    Stub b2;
    b2.set(read, stub_readStat_read1);
    Stub b3;
    b3.set(close, stub_readStat_close);
    m_tester->readStat();

}

TEST_F(UT_Process, test_readStat_002)
{
    Stub b1;
    b1.set(open, stub_readStat_open2);
    m_tester->readStat();

    EXPECT_TRUE(m_Sresult == "open failed");
}

TEST_F(UT_Process, test_readStat_003)
{
    Stub b1;
    b1.set(open, stub_readStat_open1);
    Stub b2;
    b2.set(read, stub_readStat_read2);
    Stub b3;
    b3.set(close, stub_readStat_close);
    m_tester->readStat();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readStat_004)
{
    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readStat();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readCmdline_001)
{
    Stub b1;
    b1.set(fopen, stub_readCmdline_fopen);
    m_tester->readCmdline();

    EXPECT_TRUE(m_Sresult == "fopen failed");
}

TEST_F(UT_Process, test_readCmdline_002)
{
    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readCmdline();

    EXPECT_TRUE(m_Sresult == "fopen failed");
}

TEST_F(UT_Process, test_readEnviron_001)
{
    Stub b1;
    b1.set(open, stub_readStat_open2);
    m_tester->readEnviron();

    EXPECT_TRUE(m_Sresult == "open failed");
}

TEST_F(UT_Process, test_readEnviron_002)
{
    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readEnviron();

    EXPECT_TRUE(m_Sresult == "open failed");
}

TEST_F(UT_Process, test_readSchedStat_001)
{
    Stub b1;
    b1.set(open, stub_readStat_open1);
    Stub b2;
    b2.set(read, stub_readStat_read1);
    Stub b3;
    b3.set(close, stub_readStat_close);
    m_tester->readSchedStat();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readSchedStat_002)
{
    Stub b1;
    b1.set(open, stub_readStat_open2);
    m_tester->readSchedStat();

    EXPECT_TRUE(m_Sresult == "open failed");
}

TEST_F(UT_Process, test_readSchedStat_003)
{
    Stub b1;
    b1.set(open, stub_readStat_open1);
    Stub b2;
    b2.set(read, stub_readStat_read2);
    Stub b3;
    b3.set(close, stub_readStat_close);
    m_tester->readSchedStat();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readSchedStat_004)
{
    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readSchedStat();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readStatus_001)
{
    Stub b1;
    b1.set(fopen, stub_readCmdline_fopen);
    m_tester->readStatus();

    EXPECT_TRUE(m_Sresult == "fopen failed");
}

TEST_F(UT_Process, test_readStatus_002)
{
    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readStatus();

    EXPECT_TRUE(m_Sresult == "fopen failed");
}

TEST_F(UT_Process, test_readStatm_001)
{
    Stub b1;
    b1.set(open, stub_readStat_open1);
    Stub b2;
    b2.set(read, stub_readStat_read1);
    Stub b3;
    b3.set(close, stub_readStat_close);
    m_tester->readStatm();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readStatm_002)
{
    Stub b1;
    b1.set(open, stub_readStat_open2);
    m_tester->readStatm();

    EXPECT_TRUE(m_Sresult == "open failed");
}

TEST_F(UT_Process, test_readStatm_003)
{
    Stub b1;
    b1.set(open, stub_readStat_open1);
    Stub b2;
    b2.set(read, stub_readStat_read2);
    Stub b3;
    b3.set(close, stub_readStat_close);
    m_tester->readStatm();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readStatm_004)
{
    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readStatm();

//    EXPECT_TRUE(m_Sresult=="close");
}

TEST_F(UT_Process, test_readIO_001)
{
    Stub b1;
    b1.set(fopen, stub_readCmdline_fopen);
    m_tester->readIO();

    EXPECT_TRUE(m_Sresult == "fopen failed");
}

TEST_F(UT_Process, test_readIO_002)
{

    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readIO();

    EXPECT_TRUE(m_Sresult == "fopen failed");
}

TEST_F(UT_Process, test_readSockInodes_001)
{
    Stub b1;
    b1.set(opendir, stub_readSockInodes_opendir);
    m_tester->readSockInodes();

    EXPECT_TRUE(m_Sresult == "diropen failed");
}

TEST_F(UT_Process, test_readSockInodes_002)
{

    pid_t pid = getpid();
    m_tester->d->pid = pid;
    m_tester->readSockInodes();

}

TEST_F(UT_Process, test_readProcessInfo_001)
{

    m_tester->readProcessInfo();

}
