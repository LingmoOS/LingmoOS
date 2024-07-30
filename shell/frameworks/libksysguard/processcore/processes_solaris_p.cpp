/*
    SPDX-FileCopyrightText: 2007 Adriaan de Groot <groot@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

/* Stop <sys/procfs.h> from crapping out on 32-bit architectures. */

#if !defined(_LP64) && _FILE_OFFSET_BITS == 64
#undef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 32
#endif

#include "process.h"
#include "processes_local_p.h"

#include <KLocalizedString>

#include <QSet>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <procfs.h>
#include <pwd.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/proc.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/user.h>
#include <unistd.h>

#define PROCESS_BUFFER_SIZE 512
#define PROCDIR "/proc"

namespace KSysGuard
{
class ProcessesLocal::Private
{
public:
    Private()
    {
        mProcDir = opendir(PROCDIR);
    };
    ~Private(){};
    char mBuf[PROCESS_BUFFER_SIZE + 1]; // used as a buffer to read data into
    DIR *mProcDir;
};

ProcessesLocal::ProcessesLocal()
    : d(new Private())
{
}

long ProcessesLocal::getParentPid(long pid)
{
    long long ppid = -1;
    int fd;
    psinfo_t psinfo;

    snprintf(d->mBuf, PROCESS_BUFFER_SIZE - 1, "%s/%ld/psinfo", PROCDIR, pid);
    if ((fd = open(d->mBuf, O_RDONLY)) < 0) {
        return -1; /* process has terminated in the meantime */
    }

    if (read(fd, &psinfo, sizeof(psinfo_t)) != sizeof(psinfo_t)) {
        close(fd);
        return -1;
    }
    close(fd);
    ppid = psinfo.pr_ppid;

    return ppid;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    int fd, pfd;
    psinfo_t psinfo;
    prusage_t prusage;

    snprintf(d->mBuf, PROCESS_BUFFER_SIZE - 1, "%s/%ld/psinfo", PROCDIR, pid);
    if ((fd = open(d->mBuf, O_RDONLY)) < 0) {
        return false; /* process has terminated in the meantime */
    }

    snprintf(d->mBuf, PROCESS_BUFFER_SIZE - 1, "%s/%ld/usage", PROCDIR, pid);
    if ((pfd = open(d->mBuf, O_RDONLY)) < 0) {
        close(fd);
        return false; /* process has terminated in the meantime */
    }

    process->uid = 0;
    process->gid = 0;
    process->tracerpid = -1;
    process->pid() = pid;

    if (read(fd, &psinfo, sizeof(psinfo_t)) != sizeof(psinfo_t)) {
        close(fd);
        return false;
    }
    close(fd);

    if (read(pfd, &prusage, sizeof(prusage_t)) != sizeof(prusage_t)) {
        close(pfd);
        return false;
    }
    close(pfd);

    process->setUid(psinfo.pr_uid);
    process->setEuid(psinfo.pr_euid);
    process->setGid(psinfo.pr_gid);
    process->setEgid(psinfo.pr_egid);

    switch ((int)psinfo.pr_lwp.pr_state) {
    case SIDL:
    case SWAIT:
    case SSLEEP:
        process->setStatus(Process::Sleeping);
        break;
    case SONPROC:
    case SRUN:
        process->setStatus(Process::Running);
        break;
    case SZOMB:
        process->setStatus(Process::Zombie);
        break;
    case SSTOP:
        process->setStatus(Process::Stopped);
        break;
    default:
        process->setStatus(Process::OtherStatus);
        break;
    }

    process->setVmRSS(psinfo.pr_rssize);
    process->setVmSize(psinfo.pr_size);
    process->setVmURSS(-1);

    if (process->command.isNull()) {
        QString name(psinfo.pr_fname);

        name = name.trimmed();
        if (!name.isEmpty()) {
            name.remove(QRegExp("^[^ ]*/"));
        }
        process->setName(name);
        name = psinfo.pr_fname;
        name.append(psinfo.pr_psargs);
        process->setCommand(name);
    }

    // Approximations, not quite accurate. Needs more changes in ksysguard to map
    // RR and FIFO to current Solaris classes.
    if (strcmp(psinfo.pr_lwp.pr_clname, "TS") == 0 || strcmp(psinfo.pr_lwp.pr_clname, "SYS") == 0 || strcmp(psinfo.pr_lwp.pr_clname, "FSS") == 0) {
        process->setscheduler(KSysGuard::Process::Other);

    } else if (strcmp(psinfo.pr_lwp.pr_clname, "FX") == 0 || strcmp(psinfo.pr_lwp.pr_clname, "RT") == 0) {
        process->setscheduler(KSysGuard::Process::RoundRobin);

    } else if (strcmp(psinfo.pr_lwp.pr_clname, "IA") == 0) {
        process->setscheduler(KSysGuard::Process::Interactive);
    }
    process->setNiceLevel(psinfo.pr_lwp.pr_pri);
    process->setUserTime(prusage.pr_utime.tv_sec * 100 + prusage.pr_utime.tv_nsec / 10000000.0);
    process->setSysTime(prusage.pr_stime.tv_sec * 100 + prusage.pr_stime.tv_nsec / 10000000.0);
    return false;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    long pid;

    if (d->mProcDir == NULL) {
        return pids; // There's not much we can do without /proc
    }
    struct dirent *entry;
    rewinddir(d->mProcDir);
    while ((entry = readdir(d->mProcDir)))
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            pid = atol(entry->d_name);
            // Skip all processes with parent id = 0 except init
            if (pid == 1 || getParentPid(pid) > 0) {
                pids.insert(pid);
            }
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

/*
 *
 */
Processes::Error ProcessesLocal::setNiceness(long pid, int priority)
{
    return Processes::NotSupported;
}

Processes::Error ProcessesLocal::setScheduler(long pid, int priorityClass, int priority)
{
    return Processes::NotSupported;
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
    long long memory = ((long long)sysconf(_SC_PHYS_PAGES)) * (sysconf(_SC_PAGESIZE) / 1024);
    if (memory > 0) {
        return memory;
    }
    return 0;
}

ProcessesLocal::~ProcessesLocal()
{
    delete d;
}

}
