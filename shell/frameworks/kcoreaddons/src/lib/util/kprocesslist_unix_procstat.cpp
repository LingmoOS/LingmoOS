/*

    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2019 Tobias C. Berner <tcberner@FreeBSD.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#include "kprocesslist.h"
#include "kprocesslist_unix_procstat_p.h"

#include <QDir>
#include <QProcess>

using namespace KProcessList;

// Determine UNIX processes by using the procstat library
KProcessInfoList KProcessList::processInfoList()
{
    KProcessInfoList rc;

    ProcStat pstat;
    if (!pstat) {
        return rc;
    }

    ProcStatProcesses procs(pstat);
    for (const auto &process_info : procs) {
        rc.push_back(process_info);
    }

    return rc;
}

KProcessInfo KProcessList::processInfo(qint64 pid)
{
    KProcessInfoList processInfoList = KProcessList::processInfoList();
    auto testProcessIterator = std::find_if(processInfoList.begin(), processInfoList.end(), [pid](const KProcessList::KProcessInfo &info) {
        return info.pid() == pid;
    });
    return testProcessIterator != processInfoList.end() ? *testProcessIterator : KProcessInfo{};
}
