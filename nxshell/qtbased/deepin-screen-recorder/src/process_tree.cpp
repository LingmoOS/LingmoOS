// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "process_tree.h"
#include <QDebug>

ProcessTree::ProcessTree()
{
    processMap = new QMap<int, Process>();

    rootPid = INT_MAX;
}

ProcessTree::~ProcessTree()
{
    childrenPids.clear();

    for (auto v : processMap->values()) {
        v.childProcesses.clear();
    }

    processMap->clear();
    delete processMap;
}

void ProcessTree::scanProcesses(StoredProcType processes)
{
    for (auto &i : processes) {
        int ppid = (&i.second)->ppid;
        int pid = (&i.second)->tid;

        if (!processMap->contains(pid)) {
            Process process;

            process.parentPid = ppid;

            (*processMap)[pid] = process;
        }

        if (!processMap->contains(ppid)) {
            Process process;

            process.parentPid = -1;
            process.childProcesses.append(pid);

            (*processMap)[ppid] = process;
        } else {
            (*processMap)[ppid].childProcesses.append(pid);
        }

        if (pid < rootPid) {
            rootPid = pid;
        }
    }
}

QList<int> ProcessTree::getAllChildPids(int pid)
{
    childrenPids.clear();

    getChildPids(pid);

    return childrenPids;
}

void ProcessTree::getChildPids(int pid)
{
    if (processMap->contains(pid)) {
        for (int childPid : ((*processMap)[pid].childProcesses)) {
            childrenPids.append(childPid);

            getChildPids(childPid);
        }
    }
}
/*
 * never used
void ProcessTree::printNode(int pid)
{
    qDebug() << "* Node " << pid;

    if (processMap->contains(pid)) {
        qDebug() << "### Child " << (*processMap)[pid].childProcesses;

        for (int childPid : ((*processMap)[pid].childProcesses)) {
            printNode(childPid);
        }
    }
}
void ProcessTree::printTree()
{
    printNode(rootPid);
}
*/
