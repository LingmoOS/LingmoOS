// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROCESS_SET_H
#define PROCESS_SET_H

#include "process.h"
#include "common/common.h"

#include <QMap>

#include <dirent.h>

using namespace common::alloc;

// class Settings;
namespace core {
namespace process {

enum FilterType { kFilterApps,
                  kFilterCurrentUser,
                  kNoFilter
                };

struct RecentProcStage {
    qulonglong ptime = 0;
    qulonglong read_bytes = 0; // disk read bytes
    qulonglong write_bytes = 0; // disk write bytes
    qulonglong cancelled_write_bytes = 0;
    timeval uptime = {0, 0};
};

class ProcessSet
{
public:
    explicit ProcessSet();
    ProcessSet(const ProcessSet &other);
    ~ProcessSet() = default;

    const Process getProcessById(pid_t pid) const;
    QList<pid_t> getPIDList() const;
    void removeProcess(pid_t pid);
    void updateProcessState(pid_t pid, char state);
    void updateProcessPriority(pid_t pid, int priority);
    std::weak_ptr<RecentProcStage> getRecentProcStage(pid_t pid) const;

    void refresh();

private:
    void scanProcess();
    void mergeSubProcNetIO(pid_t ppid, qreal &recvBps, qreal &sendBps);
    void mergeSubProcCpu(pid_t ppid, qreal &cpu);

    class Iterator
    {
    public:
        Iterator();

        bool hasNext();
        Process next();

    private:
        void advance();

        uDir m_dir {};
        struct dirent *m_dirent {};
    };

private:
    // Settings *m_settings = nullptr;
    QMap<pid_t, Process> m_simpleSet;
    QMap<pid_t, Process> m_set;
    QMap<pid_t, std::shared_ptr<RecentProcStage>> m_recentProcStage {};

    QMap<pid_t, pid_t> m_pidCtoPMapping {}; // child to parent pid mapping
    QMultiMap<pid_t, pid_t> m_pidPtoCMapping {}; // parent to child pid mapping
    QList<pid_t> m_prePid;
    QList<pid_t> m_curPid;
    QList<pid_t> m_pidMyApps;

    friend class Iterator;
};

} // namespace process
} // namespace core

#endif // PROCESS_SET_H
