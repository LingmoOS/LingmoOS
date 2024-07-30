/*
    SPDX-FileCopyrightText: 2007 Manolo Valdes <nolis71cu@gmail.com>
    SPDX-FileCopyrightText: 2007 Mark Davies <mark@mcs.vuw.ac.nz>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"
#include "processes_local_p.h"

#include <KLocalizedString>

#include <QSet>

#include <kvm.h>
#include <sched.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/sysctl.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>

namespace KSysGuard
{
class ProcessesLocal::Private
{
public:
    Private()
    {
        kd = kvm_open(NULL, NULL, NULL, KVM_NO_FILES, "kvm_open");
    }
    ~Private()
    {
        kvm_close(kd);
    }
    inline bool readProc(long pid, struct kinfo_proc2 **p, int *num);
    inline void readProcStatus(struct kinfo_proc2 *p, Process *process);
    inline void readProcStat(struct kinfo_proc2 *p, Process *process);
    inline void readProcStatm(struct kinfo_proc2 *p, Process *process);
    inline bool readProcCmdline(struct kinfo_proc2 *p, Process *process);

    kvm_t *kd;
};

#ifndef _SC_NPROCESSORS_ONLN
long int KSysGuard::ProcessesLocal::numberProcessorCores()
{
    int mib[2];
    int ncpu;
    size_t len;

    mib[0] = CTL_HW;
    mib[1] = HW_NCPU;
    len = sizeof(ncpu);

    if (sysctl(mib, 2, &ncpu, &len, NULL, 0) == -1 || !len) {
        return 1;
    }
    return len;
}
#endif

bool ProcessesLocal::Private::readProc(long pid, struct kinfo_proc2 **p, int *num)
{
    int len;
    int op, arg;

    if (pid == 0) {
        op = KERN_PROC_ALL;
        arg = 0;
    } else {
        op = KERN_PROC_PID;
        arg = pid;
    }
    *p = kvm_getproc2(kd, op, arg, sizeof(struct kinfo_proc2), &len);

    if (len < 1) {
        return false;
    }

    if (num != NULL) {
        *num = len;
    }
    return true;
}

void ProcessesLocal::Private::readProcStatus(struct kinfo_proc2 *p, Process *process)
{
    process->setUid(p->p_ruid);
    process->setEuid(p->p_uid);
    process->setGid(p->p_rgid);
    process->setEgid(p->p_gid);
    process->setTracerpid(-1);

    process->setName(QString(p->p_comm ? p->p_comm : "????"));
}

void ProcessesLocal::Private::readProcStat(struct kinfo_proc2 *p, Process *ps)
{
    const char *ttname;
    dev_t dev;

    ps->setUserTime(p->p_uutime_sec * 100 + p->p_uutime_usec / 10000);
    ps->setSysTime(p->p_ustime_sec * 100 + p->p_ustime_usec / 10000);

    ps->setUserUsage(100.0 * ((double)(p->p_pctcpu) / FSCALE));
    ps->setSysUsage(0);

    ps->setNiceLevel(p->p_nice - NZERO);
    ps->setVmSize((p->p_vm_tsize + p->p_vm_dsize + p->p_vm_ssize) * getpagesize());
    ps->setVmRSS(p->p_vm_rssize * getpagesize());

    // "idle","run","sleep","stop","zombie"
    switch (p->p_stat) {
    case LSRUN:
        ps->setStatus(Process::Running);
        break;
    case LSSLEEP:
        ps->setStatus(Process::Sleeping);
        break;
    case LSSTOP:
        ps->setStatus(Process::Stopped);
        break;
    case LSZOMB:
        ps->setStatus(Process::Zombie);
        break;
    case LSONPROC:
        ps->setStatus(Process::Running);
        break;
    default:
        ps->setStatus(Process::OtherStatus);
        break;
    }

    dev = p->p_tdev;
    if (dev == NODEV || (ttname = devname(dev, S_IFCHR)) == NULL) {
        ps->setTty(QByteArray());
    } else {
        ps->setTty(QByteArray(ttname));
    }
}

void ProcessesLocal::Private::readProcStatm(struct kinfo_proc2 *p, Process *process)
{
    // TODO

    //     unsigned long shared;
    //     process->vmURSS = process->vmRSS - (shared * sysconf(_SC_PAGESIZE) / 1024);
    process->setVmURSS(-1);
}

bool ProcessesLocal::Private::readProcCmdline(struct kinfo_proc2 *p, Process *process)
{
    char **argv;

    if ((argv = kvm_getargv2(kd, p, 256)) == NULL) {
        return false;
    }

    QString command = QString("");

    while (*argv) {
        command += *argv;
        command += " ";
        argv++;
    }
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
    struct kinfo_proc2 *p;
    if (d->readProc(pid, &p, 0)) {
        ppid = p->p_ppid;
    }
    return ppid;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    struct kinfo_proc2 *p;
    if (!d->readProc(pid, &p, NULL)) {
        return false;
    }
    d->readProcStat(p, process);
    d->readProcStatus(p, process);
    d->readProcStatm(p, process);
    if (!d->readProcCmdline(p, process)) {
        return false;
    }

    return true;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    int len;
    int num;
    struct kinfo_proc2 *p;

    d->readProc(0, &p, &len);

    for (num = 0; num < len; num++) {
        long pid = p[num].p_pid;
        long long ppid = p[num].p_ppid;

        // skip all process with parent id = 0 but init
        if (ppid <= 0 && pid != 1) {
            continue;
        }
        pids.insert(pid);
    }
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
    sysctlbyname("hw.physmem", &Total, &len, NULL, 0);
    return Total /= 1024;
}

ProcessesLocal::~ProcessesLocal()
{
    delete d;
}

}
