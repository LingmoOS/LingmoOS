// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <gtest/gtest.h>
#include <QTest>
#include <QRect>
#include <QTimer>
#include <unistd.h>
#include "../src/process_tree.h"


using namespace testing;

class ProcessTreeTest:public testing::Test{

    //typedef std::map<int, proc_t> StoredProcType;
public:
    ProcessTree *processTree;
    virtual void SetUp() override{
        std::cout << "start RecordProcessTest" << std::endl;
        processTree = new ProcessTree;
    }

    virtual void TearDown() override{
        delete processTree;
        std::cout << "end RecordProcessTest" << std::endl;
    }
};

TEST_F(ProcessTreeTest, recordVideo)
{
    PROCTAB *proc = openproc(
                PROC_FILLMEM |          // memory status: read information from /proc/#pid/statm
                PROC_FILLSTAT |         // cpu status: read information from /proc/#pid/stat
                PROC_FILLUSR            // user status: resolve user ids to names via /etc/passwd
                );
    static proc_t proc_info;
    memset(&proc_info, 0, sizeof(proc_t));

    std::map<int, proc_t> processes;
    while (readproc(proc, &proc_info) != nullptr) {
        processes[proc_info.tid] = proc_info;
    }
    closeproc(proc);

    processTree->scanProcesses(processes);
    pid_t pid = getpid();
    qDebug() << "pid pid pid"<< pid;
    int id = processTree->getAllChildPids(pid).size();
    qDebug() <<" id id id " <<id;
}
