/*

    This file is part of the KDE Frameworks

    SPDX-FileCopyrightText: 2019 Tobias C. Berner <tcberner@FreeBSD.org>

    SPDX-License-Identifier: LGPL-2.1-only
*/

#pragma once

#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>

#include <sys/queue.h> // Must be included before libprocstat.h, otherwise this fails to build on FreeBSD

#include <libprocstat.h>

namespace KProcessList
{
struct ProcStat {
public:
    struct procstat *pstat;
    ProcStat()
    {
        pstat = procstat_open_sysctl();
    }

    ~ProcStat()
    {
        procstat_close(pstat);
    }

    operator bool() const
    {
        return pstat;
    }
};

struct ProcStatProcesses {
private:
    ProcStat &parent;
    unsigned int proc_count;
    struct kinfo_proc *procs;

public:
    ProcStatProcesses(ProcStat &pstat)
        : parent(pstat)
    {
        procs = procstat_getprocs(parent.pstat, KERN_PROC_PROC, 0, &proc_count);
    }

    ~ProcStatProcesses()
    {
        if (procs) {
            procstat_freeprocs(parent.pstat, procs);
        }
    }

    operator bool() const
    {
        return procs && proc_count > 0;
    }

    unsigned int count() const
    {
        return proc_count;
    }

    class ProcessIterator
    {
    private:
        const ProcStatProcesses &processes;
        unsigned int pos;

    public:
        ProcessIterator(const ProcStatProcesses &processes, unsigned int pos)
            : processes(processes)
            , pos(pos){};

        bool operator!=(const ProcessIterator &other) const
        {
            return pos != other.pos;
        }

        ProcessIterator &operator++()
        {
            if (pos < processes.count()) {
                ++pos;
            }
            return *this;
        }

        const KProcessInfo operator*()
        {
            QStringList command_line;
            QString command;
            struct kinfo_proc *proc = &processes.procs[pos];

            // Don't use procstat_getpathname() because:
            // - it can fail, and then it spams a warning to stderr
            // - it produces a full path, e.g. /tmp/kde/build/kcoreaddons/bin/kprocesslisttest
            //   rather than the command-name, so it fails in tests that check for
            //   a process name only.
            command = QString::fromLocal8Bit(proc->ki_comm);

            char **args;
            args = procstat_getargv(processes.parent.pstat, proc, 0);
            if (args) {
                for (int i = 0; args[i] != nullptr; i++) {
                    command_line << QString::fromLocal8Bit(args[i]);
                }
            }

            pid_t pid = proc->ki_pid;
            QString user = QString::fromLocal8Bit(proc->ki_login);
            return KProcessInfo(pid, command_line.join(QString::fromLocal8Bit(" ")), command, user);
        }
    };

    ProcessIterator begin() const
    {
        return ProcessIterator(*this, 0);
    }
    ProcessIterator end() const
    {
        return ProcessIterator(*this, this->count());
    }
};
}
