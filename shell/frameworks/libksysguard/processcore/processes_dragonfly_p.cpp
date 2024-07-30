/*
    SPDX-FileCopyrightText: 2007 Manolo Valdes <nolis71cu@gmail.com>
    SPDX-FileCopyrightText: 2010 Alex Hornung <ahornung@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"
#include "processes_local_p.h"

#include <KLocalizedString>

#include <QSet>

#include <err.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/resourcevar.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>

#define PP(pp, field) ((pp)->kp_##field)
#define LP(pp, field) ((pp)->kp_lwp.kl_##field)
#define VP(pp, field) ((pp)->kp_vm_##field)

namespace KSysGuard
{
class ProcessesLocal::Private
{
public:
    Private()
    {
    }
    ~Private()
    {
    }
    inline bool readProc(long pid, struct kinfo_proc *p);
    inline void readProcStatus(struct kinfo_proc *p, Process *process);
    inline void readProcStat(struct kinfo_proc *p, Process *process);
    inline void readProcStatm(struct kinfo_proc *p, Process *process);
    inline bool readProcCmdline(long pid, Process *process);
};

bool ProcessesLocal::Private::readProc(long pid, struct kinfo_proc *p)
{
    int mib[4];
    size_t len;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_PID;
    mib[3] = pid;

    len = sizeof(struct kinfo_proc);
    if (sysctl(mib, 4, p, &len, NULL, 0) == -1 || !len) {
        return false;
    }
    return true;
}

void ProcessesLocal::Private::readProcStatus(struct kinfo_proc *p, Process *process)
{
    process->setUid(0);
    process->setGid(0);
    process->setTracerpid(-1);

    process->setEuid(PP(p, uid));
    process->setUid(PP(p, ruid));
    process->setEgid(PP(p, svgid));
    process->setGid(PP(p, rgid));
    process->setName(QString(PP(p, comm)));
}

void ProcessesLocal::Private::readProcStat(struct kinfo_proc *p, Process *ps)
{
    ps->setUserTime(LP(p, uticks) / 10000);
    ps->setSysTime((LP(p, sticks) + LP(p, iticks)) / 10000);
    ps->setNiceLevel(PP(p, nice));
    ps->setVmSize(VP(p, map_size) / 1024); /* convert to KiB */
    ps->setVmRSS(VP(p, prssize) * getpagesize() / 1024); /* convert to KiB */

    // "idle","run","sleep","stop","zombie"
    switch (LP(p, stat)) {
    case LSRUN:
        ps->setStatus(Process::Running);
        break;
    case LSSLEEP:
        ps->setStatus(Process::Sleeping);
        break;
    case LSSTOP:
        ps->setStatus(Process::Stopped);
        break;
    default:
        ps->setStatus(Process::OtherStatus);
        break;
    }
    if (PP(p, stat) == SZOMB) {
        ps->setStatus(Process::Zombie);
    }
}

void ProcessesLocal::Private::readProcStatm(struct kinfo_proc *p, Process *process)
{
    process->setVmURSS(-1);
}

bool ProcessesLocal::Private::readProcCmdline(long pid, Process *process)
{
    int mib[4];
    size_t buflen = 256;
    char buf[256];

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ARGS;
    mib[3] = pid;

    if (sysctl(mib, 4, buf, &buflen, NULL, 0) == -1 || (buflen == 0)) {
        return false;
    }
    QString command = QString(buf);

    // cmdline separates parameters with the NULL character
    command.replace('\0', ' ');
    process->setCommand(command.trimmed());

    return true;
}

ProcessesLocal::ProcessesLocal()
    : d(new Private())
{
}

long ProcessesLocal::getParentPid(long pid)
{
    long long ppid = -1;
    struct kinfo_proc p;

    if (d->readProc(pid, &p)) {
        ppid = PP(&p, ppid);
    }

    return ppid;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    struct kinfo_proc p;

    if (!d->readProc(pid, &p)) {
        return false;
    }

    d->readProcStat(&p, process);
    d->readProcStatus(&p, process);
    d->readProcStatm(&p, process);
    if (!d->readProcCmdline(pid, process)) {
        return false;
    }

    return true;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    int mib[3];
    size_t len;
    size_t num;
    struct kinfo_proc *p;

    mib[0] = CTL_KERN;
    mib[1] = KERN_PROC;
    mib[2] = KERN_PROC_ALL;
    if (sysctl(mib, 3, NULL, &len, NULL, 0) == -1) {
        return pids;
    }
    if ((p = (kinfo_proc *)malloc(len)) == NULL) {
        return pids;
    }
    if (sysctl(mib, 3, p, &len, NULL, 0) == -1) {
        free(p);
        return pids;
    }

    for (num = 0; num < len / sizeof(struct kinfo_proc); num++) {
        long pid = PP((&p[num]), pid);
        long long ppid = PP((&p[num]), ppid);

        // skip all process with parent id = 0 but init
        if (ppid <= 0 && pid != 1) {
            continue;
        }
        pids.insert(pid);
    }
    free(p);
    return pids;
}

Processes::Error ProcessesLocal::sendSignal(long pid, int sig)
{
    if (kill((pid_t)pid, sig)) {
        // Kill failed
        return Processes::Unknown;
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setNiceness(long pid, int priority)
{
    if (setpriority(PRIO_PROCESS, pid, priority)) {
        // set niceness failed
        return Processes::Unknown;
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setScheduler(long pid, int priorityClass, int priority)
{
    if (priorityClass == KSysGuard::Process::Other || priorityClass == KSysGuard::Process::Batch) {
        priority = 0;
    }
    if (pid <= 0) {
        return Processes::InvalidPid; // check the parameters
    }
    struct sched_param params;
    params.sched_priority = priority;
    bool success;
    switch (priorityClass) {
    case (KSysGuard::Process::Other):
        success = (sched_setscheduler(pid, SCHED_OTHER, &params) == 0);
        break;
    case (KSysGuard::Process::RoundRobin):
        success = (sched_setscheduler(pid, SCHED_RR, &params) == 0);
        break;
    case (KSysGuard::Process::Fifo):
        success = (sched_setscheduler(pid, SCHED_FIFO, &params) == 0);
        break;
#ifdef SCHED_BATCH
    case (KSysGuard::Process::Batch):
        success = (sched_setscheduler(pid, SCHED_BATCH, &params) == 0);
        break;
#endif
    }
    if (success) {
        return Processes::NoError;
    }
    return Processes::Unknown;
}

Processes::Error ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority)
{
    return Processes::NotSupported; // Not yet supported
}

bool ProcessesLocal::supportsIoNiceness()
{
    return false;
}

long long ProcessesLocal::totalPhysicalMemory()
{
    size_t Total;
    size_t len;

    len = sizeof(Total);
    if (sysctlbyname("hw.physmem", &Total, &len, NULL, 0) == -1) {
        return 0;
    }

    Total *= getpagesize() / 1024;
    return Total;
}

ProcessesLocal::~ProcessesLocal()
{
    delete d;
}

}
