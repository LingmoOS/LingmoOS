// Copyright (C) 2019 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

//self
#include "process/process_set.h"
#include "process/process_db.h"
#include "common/common.h"
#include "wm/wm_window_list.h"

//gtest
#include "stub.h"
#include <gtest/gtest.h>

using namespace core::process;
/***************************************STUB begin*********************************************/

/***************************************STUB end**********************************************/
class UT_ProcessSet : public ::testing::Test
{
public:
    UT_ProcessSet() : m_tester(nullptr) {}

public:
    virtual void SetUp()
    {
        m_tester = new ProcessSet();
    }

    virtual void TearDown()
    {
        if(m_tester){
            delete m_tester;
            m_tester = nullptr;
        }

    }

protected:
    ProcessSet *m_tester;

};

TEST_F(UT_ProcessSet, initTest)
{

}

TEST_F(UT_ProcessSet, test_mergeSubProcNetIO_001)
{
    pid_t ppid = getppid();
    qreal recvBps = 100;
    qreal sendBps = 100;
    m_tester->mergeSubProcNetIO(ppid,recvBps,sendBps);

}

TEST_F(UT_ProcessSet, test_mergeSubProcCpu_001)
{
    pid_t ppid = getppid();
    qreal cpu = 0;
    m_tester->mergeSubProcCpu(ppid,cpu);

}

TEST_F(UT_ProcessSet, test_refresh_001)
{
    m_tester->refresh();
}

TEST_F(UT_ProcessSet, test_scanProcess_001)
{
    Process *proc = new Process;
    m_tester->m_set.insert(proc->pid(),*proc);
    m_tester->scanProcess();
    delete proc;
}

TEST_F(UT_ProcessSet, test_hasNext_001)
{
    ProcessSet::Iterator *it = new ProcessSet::Iterator();
    it->hasNext();
    delete it;
}

TEST_F(UT_ProcessSet, test_next_001)
{
    ProcessSet::Iterator *it = new ProcessSet::Iterator();
    it->next();
    delete it;
}

TEST_F(UT_ProcessSet, test_advance_001)
{
    ProcessSet::Iterator *it = new ProcessSet::Iterator();
    it->advance();
    delete it;
}

TEST_F(UT_ProcessSet, test_getRecentProcStage_001)
{
    pid_t pid = getpid();
    m_tester->getRecentProcStage(pid);

}

TEST_F(UT_ProcessSet, test_getProcessById_001)
{
    pid_t pid = getpid();
    m_tester->getProcessById(pid);

}

TEST_F(UT_ProcessSet, test_getPIDList_001)
{
    m_tester->getPIDList();

}

TEST_F(UT_ProcessSet, test_removeProcess_001)
{
    pid_t pid = getpid();
    m_tester->removeProcess(pid);
}

TEST_F(UT_ProcessSet, test_updateProcessState_001)
{
    pid_t pid = getpid();
    m_tester->updateProcessState(pid,'Z');
}

TEST_F(UT_ProcessSet, test_updateProcessPriority_001)
{
    pid_t pid = getpid();
    m_tester->updateProcessPriority(pid,0);
}
