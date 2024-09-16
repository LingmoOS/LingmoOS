// Copyright (C) 2011 ~ 2018 Deepin, Inc.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESSTREE_H
#define PROCESSTREE_H

#include <QMap>
#include <QObject>
#include <proc/readproc.h>
#include <proc/sysinfo.h>

class ProcessTree : public QObject
{
    Q_OBJECT

    typedef std::map<int, proc_t> StoredProcType;

    struct Process {
        int parentPid;
        QList<int> childProcesses;
    };

public:
    ProcessTree();
    ~ProcessTree();

    QList<int> getAllChildPids(int pid);
    void getChildPids(int pid);
    //void printNode(int pid);
    //void printTree();
    void scanProcesses(StoredProcType processes);

private:
    QList<int> childrenPids;
    QMap<int, Process> *processMap;
    int rootPid;
};

#endif
