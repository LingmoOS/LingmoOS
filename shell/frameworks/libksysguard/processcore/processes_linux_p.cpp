/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"
#include "processes_local_p.h"
#include "read_procsmaps_runnable.h"

#include <klocalizedstring.h>

#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QPointer>
#include <QSet>
#include <QTextStream>
#include <QThreadPool>

// for sysconf
#include <unistd.h>
// for kill and setNice
#include <dirent.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/types.h>
// for ionice
#include <asm/unistd.h>
#include <sys/ptrace.h>
// for getsched
#include <sched.h>

#define PROCESS_BUFFER_SIZE 1000

/* For ionice */
extern int sys_ioprio_set(int, int, int);
extern int sys_ioprio_get(int, int);

#define HAVE_IONICE
/* Check if this system has ionice */
#if !defined(SYS_ioprio_get) || !defined(SYS_ioprio_set)
/* All new kernels have SYS_ioprio_get and _set defined, but for the few that do not, here are the definitions */
#if defined(__i386__)
#define __NR_ioprio_set 289
#define __NR_ioprio_get 290
#elif defined(__ppc__) || defined(__powerpc__)
#define __NR_ioprio_set 273
#define __NR_ioprio_get 274
#elif defined(__x86_64__)
#define __NR_ioprio_set 251
#define __NR_ioprio_get 252
#elif defined(__ia64__)
#define __NR_ioprio_set 1274
#define __NR_ioprio_get 1275
#else
#ifdef __GNUC__
#warning "This architecture does not support IONICE.  Disabling ionice feature."
#endif
#undef HAVE_IONICE
#endif
/* Map these to SYS_ioprio_get */
#define SYS_ioprio_get __NR_ioprio_get
#define SYS_ioprio_set __NR_ioprio_set

#endif /* !SYS_ioprio_get */

/* Set up ionice functions */
#ifdef HAVE_IONICE
#define IOPRIO_WHO_PROCESS 1
#define IOPRIO_CLASS_SHIFT 13

/* Expose the kernel calls to userspace via syscall
 * See man ioprio_set  and man ioprio_get   for information on these functions */
static int ioprio_set(int which, int who, int ioprio)
{
    return syscall(SYS_ioprio_set, which, who, ioprio);
}

static int ioprio_get(int which, int who)
{
    return syscall(SYS_ioprio_get, which, who);
}
#endif

namespace KSysGuard
{
class ProcessesLocal::Private
{
public:
    Private()
    {
        mProcDir = opendir("/proc");
    }
    ~Private();
    inline bool readProcStatus(const QString &dir, Process *process);
    inline bool readProcStat(const QString &dir, Process *process);
    inline bool readProcStatm(const QString &dir, Process *process);
    inline bool readProcCmdline(const QString &dir, Process *process);
    inline bool readProcCGroup(const QString &dir, Process *process);
    inline bool readProcAttr(const QString &dir, Process *process);
    inline bool getNiceness(long pid, Process *process);
    inline bool getIOStatistics(const QString &dir, Process *process);
    QFile mFile;
    char mBuffer[PROCESS_BUFFER_SIZE + 1]; // used as a buffer to read data into
    DIR *mProcDir;
    QPointer<ReadProcSmapsRunnable> smapsRunnable;
};

ProcessesLocal::Private::~Private()
{
    closedir(mProcDir);
}

ProcessesLocal::ProcessesLocal()
    : d(new Private())
{
}
bool ProcessesLocal::Private::readProcStatus(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("status"));
    if (!mFile.open(QIODevice::ReadOnly))
        return false; /* process has terminated in the meantime */

    process->setUid(0);
    process->setGid(0);
    process->setTracerpid(-1);
    process->setNumThreads(0);
    process->setNoNewPrivileges(0);

    int size;
    int found = 0; // count how many fields we found
    while ((size = mFile.readLine(mBuffer, sizeof(mBuffer))) > 0) { //-1 indicates an error
        switch (mBuffer[0]) {
        case 'N':
            if ((unsigned int)size > sizeof("Name:") && qstrncmp(mBuffer, "Name:", sizeof("Name:") - 1) == 0) {
                if (process->command().isEmpty()) {
                    process->setName(QString::fromLocal8Bit(mBuffer + sizeof("Name:") - 1, size - sizeof("Name:") + 1).trimmed());
                }
                if (++found == 6) {
                    goto finish;
                }
            } else if ((unsigned int)size > sizeof("NoNewPrivs:") && qstrncmp(mBuffer, "NoNewPrivs:", sizeof("NoNewPrivs:") - 1) == 0) {
                process->setNoNewPrivileges(atol(mBuffer + sizeof("NoNewPrivs:") - 1));
                if (++found == 6) {
                    goto finish;
                }
            }
            break;
        case 'U':
            if ((unsigned int)size > sizeof("Uid:") && qstrncmp(mBuffer, "Uid:", sizeof("Uid:") - 1) == 0) {
                qlonglong uid;
                qlonglong euid;
                qlonglong suid;
                qlonglong fsuid;
                sscanf(mBuffer + sizeof("Uid:") - 1, "%lld %lld %lld %lld", &uid, &euid, &suid, &fsuid);
                process->setUid(uid);
                process->setEuid(euid);
                process->setSuid(suid);
                process->setFsuid(fsuid);
                if (++found == 6) {
                    goto finish;
                }
            }
            break;
        case 'G':
            if ((unsigned int)size > sizeof("Gid:") && qstrncmp(mBuffer, "Gid:", sizeof("Gid:") - 1) == 0) {
                qlonglong gid, egid, sgid, fsgid;
                sscanf(mBuffer + sizeof("Gid:") - 1, "%lld %lld %lld %lld", &gid, &egid, &sgid, &fsgid);
                process->setGid(gid);
                process->setEgid(egid);
                process->setSgid(sgid);
                process->setFsgid(fsgid);
                if (++found == 6) {
                    goto finish;
                }
            }
            break;
        case 'T':
            if ((unsigned int)size > sizeof("TracerPid:") && qstrncmp(mBuffer, "TracerPid:", sizeof("TracerPid:") - 1) == 0) {
                process->setTracerpid(atol(mBuffer + sizeof("TracerPid:") - 1));
                if (process->tracerpid() == 0) {
                    process->setTracerpid(-1);
                }
                if (++found == 6) {
                    goto finish;
                }
            } else if ((unsigned int)size > sizeof("Threads:") && qstrncmp(mBuffer, "Threads:", sizeof("Threads:") - 1) == 0) {
                process->setNumThreads(atol(mBuffer + sizeof("Threads:") - 1));
                if (++found == 6) {
                    goto finish;
                }
            }
            break;
        default:
            break;
        }
    }

finish:
    mFile.close();
    return true;
}

bool ProcessesLocal::Private::readProcCGroup(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("cgroup"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    while (mFile.readLine(mBuffer, sizeof(mBuffer)) > 0) { //-1 indicates an error
        if (mBuffer[0] == '0' && mBuffer[1] == ':' && mBuffer[2] == ':') {
            process->setCGroup(QString::fromLocal8Bit(&mBuffer[3]).trimmed());
            break;
        }
    }
    mFile.close();
    return true;
}

bool ProcessesLocal::Private::readProcAttr(const QString &dir, Process *process)
{
    mFile.setFileName(dir + QStringLiteral("attr/current"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    if (mFile.readLine(mBuffer, sizeof(mBuffer)) > 0) { //-1 indicates an error
        process->setMACContext(QString::fromLocal8Bit(mBuffer).trimmed());
    }
    mFile.close();
    return true;
}

long ProcessesLocal::getParentPid(long pid)
{
    if (pid <= 0) {
        return -1;
    }
    d->mFile.setFileName(QStringLiteral("/proc/") + QString::number(pid) + QStringLiteral("/stat"));
    if (!d->mFile.open(QIODevice::ReadOnly)) {
        return -1; /* process has terminated in the meantime */
    }

    int size; // amount of data read in
    if ((size = d->mFile.readLine(d->mBuffer, sizeof(d->mBuffer))) <= 0) { //-1 indicates nothing read
        d->mFile.close();
        return -1;
    }

    d->mFile.close();
    char *word = d->mBuffer;
    // The command name is the second parameter, and this ends with a closing bracket.  So find the last
    // closing bracket and start from there
    word = strrchr(word, ')');
    if (!word) {
        return -1;
    }
    word++; // Nove to the space after the last ")"
    int current_word = 1;

    while (true) {
        if (word[0] == ' ') {
            if (++current_word == 3) {
                break;
            }
        } else if (word[0] == 0) {
            return -1; // end of data - serious problem
        }
        word++;
    }
    long ppid = atol(++word);
    if (ppid == 0) {
        return -1;
    }
    return ppid;
}

bool ProcessesLocal::Private::readProcStat(const QString &dir, Process *ps)
{
    QString filename = dir + QStringLiteral("stat");
    // As an optimization, if the last file read in was stat, then we already have this info in memory
    if (mFile.fileName() != filename) {
        mFile.setFileName(filename);
        if (!mFile.open(QIODevice::ReadOnly)) {
            return false; /* process has terminated in the meantime */
        }
        if (mFile.readLine(mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
            mFile.close();
            return false;
        }
        mFile.close();
    }

    char *word = mBuffer;
    // The command name is the second parameter, and this ends with a closing bracket.  So find the last
    // closing bracket and start from there
    word = strrchr(word, ')');
    if (!word) {
        return false;
    }
    word++; // Nove to the space after the last ")"
    int current_word = 1; // We've skipped the process ID and now at the end of the command name
    char status = '\0';
    unsigned long long vmSize = 0;
    unsigned long long vmRSS = 0;
    while (current_word < 23) {
        if (word[0] == ' ') {
            ++current_word;
            switch (current_word) {
            case 2: // status
                status = word[1]; // Look at the first letter of the status.
                // We analyze this after the while loop
                break;
            case 6: // ttyNo
            {
                int ttyNo = atoi(word + 1);
                int major = ttyNo >> 8;
                int minor = ttyNo & 0xff;
                switch (major) {
                case 136:
                    ps->setTty(QByteArray("pts/") + QByteArray::number(minor));
                    break;
                case 5:
                    ps->setTty(QByteArray("tty"));
                    break;
                case 4:
                    if (minor < 64) {
                        ps->setTty(QByteArray("tty") + QByteArray::number(minor));
                    } else {
                        ps->setTty(QByteArray("ttyS") + QByteArray::number(minor - 64));
                    }
                    break;
                default:
                    ps->setTty(QByteArray());
                }
            } break;
            case 13: // userTime
                ps->setUserTime(atoll(word + 1));
                break;
            case 14: // sysTime
                ps->setSysTime(atoll(word + 1));
                break;
            case 18: // niceLevel
                ps->setNiceLevel(atoi(word + 1)); /*Or should we use getPriority instead? */
                break;
            case 21: // startTime
                ps->setStartTime(atoll(word + 1));
                break;
            case 22: // vmSize
                vmSize = atoll(word + 1);
                break;
            case 23: // vmRSS
                vmRSS = atoll(word + 1);
                break;
            default:
                break;
            }
        } else if (word[0] == 0) {
            return false; // end of data - serious problem
        }
        word++;
    }

    /* There was a "(ps->vmRss+3) * sysconf(_SC_PAGESIZE)" here in the original ksysguard code.  I have no idea why!  After comparing it to
     *   meminfo and other tools, this means we report the RSS by 12 bytes differently compared to them.  So I'm removing the +3
     *   to be consistent.  NEXT TIME COMMENT STRANGE THINGS LIKE THAT! :-)
     *
     *   Update: I think I now know why - the kernel allocates 3 pages for
     *   tracking information about each the process. This memory isn't
     *   included in vmRSS..*/
    ps->setVmRSS(vmRSS * (sysconf(_SC_PAGESIZE) / 1024)); /*convert to KiB*/
    ps->setVmSize(vmSize / 1024); /* convert to KiB */

    switch (status) {
    case 'R':
        ps->setStatus(Process::Running);
        break;
    case 'S':
        ps->setStatus(Process::Sleeping);
        break;
    case 'D':
        ps->setStatus(Process::DiskSleep);
        break;
    case 'Z':
        ps->setStatus(Process::Zombie);
        break;
    case 'T':
        ps->setStatus(Process::Stopped);
        break;
    case 'W':
        ps->setStatus(Process::Paging);
        break;
    default:
        ps->setStatus(Process::OtherStatus);
        break;
    }
    return true;
}

bool ProcessesLocal::Private::readProcStatm(const QString &dir, Process *process)
{
#ifdef _SC_PAGESIZE
    mFile.setFileName(dir + QStringLiteral("statm"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    if (mFile.readLine(mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
        mFile.close();
        return 0;
    }
    mFile.close();

    int current_word = 0;
    char *word = mBuffer;

    while (true) {
        if (word[0] == ' ') {
            if (++current_word == 2) {
                // number of pages that are shared
                break;
            }
        } else if (word[0] == 0) {
            return false; // end of data - serious problem
        }
        word++;
    }
    long shared = atol(word + 1);

    /* we use the rss - shared  to find the amount of memory just this app uses */
    process->setVmURSS(process->vmRSS() - (shared * sysconf(_SC_PAGESIZE) / 1024));
#else
    process->setVmURSS(0);
#endif
    return true;
}

bool ProcessesLocal::Private::readProcCmdline(const QString &dir, Process *process)
{
    if (!process->command().isNull()) {
        return true; // only parse the cmdline once.  This function takes up 25% of the CPU time :-/
    }
    mFile.setFileName(dir + QStringLiteral("cmdline"));
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }

    QTextStream in(&mFile);
    process->setCommand(in.readAll());

    // cmdline separates parameters with the NULL character
    if (!process->command().isEmpty()) {
        // extract non-truncated name from cmdline
        int zeroIndex = process->command().indexOf(QLatin1Char('\0'));
        int processNameStart = process->command().lastIndexOf(QLatin1Char('/'), zeroIndex);
        if (processNameStart == -1) {
            processNameStart = 0;
        } else {
            processNameStart++;
        }
        QString nameFromCmdLine = process->command().mid(processNameStart, zeroIndex - processNameStart);
        if (nameFromCmdLine.startsWith(process->name())) {
            process->setName(nameFromCmdLine);
        }

        process->command().replace(QLatin1Char('\0'), QLatin1Char(' '));
    }

    mFile.close();
    return true;
}

bool ProcessesLocal::Private::getNiceness(long pid, Process *process)
{
    int sched = sched_getscheduler(pid);
    switch (sched) {
    case (SCHED_OTHER):
        process->setScheduler(KSysGuard::Process::Other);
        break;
    case (SCHED_RR):
        process->setScheduler(KSysGuard::Process::RoundRobin);
        break;
    case (SCHED_FIFO):
        process->setScheduler(KSysGuard::Process::Fifo);
        break;
#ifdef SCHED_IDLE
    case (SCHED_IDLE):
        process->setScheduler(KSysGuard::Process::SchedulerIdle);
        break;
#endif
#ifdef SCHED_BATCH
    case (SCHED_BATCH):
        process->setScheduler(KSysGuard::Process::Batch);
        break;
#endif
    default:
        process->setScheduler(KSysGuard::Process::Other);
    }
    if (sched == SCHED_FIFO || sched == SCHED_RR) {
        struct sched_param param;
        if (sched_getparam(pid, &param) == 0) {
            process->setNiceLevel(param.sched_priority);
        } else {
            process->setNiceLevel(0); // Error getting scheduler parameters.
        }
    }

#ifdef HAVE_IONICE
    int ioprio = ioprio_get(IOPRIO_WHO_PROCESS, pid); /* Returns from 0 to 7 for the iopriority, and -1 if there's an error */
    if (ioprio == -1) {
        process->setIoniceLevel(-1);
        process->setIoPriorityClass(KSysGuard::Process::None);
        return false; /* Error. Just give up. */
    }
    process->setIoniceLevel(ioprio & 0xff); /* Bottom few bits are the priority */
    process->setIoPriorityClass((KSysGuard::Process::IoPriorityClass)(ioprio >> IOPRIO_CLASS_SHIFT)); /* Top few bits are the class */
    return true;
#else
    return false; /* Do nothing, if we do not support this architecture */
#endif
}

bool ProcessesLocal::Private::getIOStatistics(const QString &dir, Process *process)
{
    QString filename = dir + QStringLiteral("io");
    // As an optimization, if the last file read in was io, then we already have this info in memory
    mFile.setFileName(filename);
    if (!mFile.open(QIODevice::ReadOnly)) {
        return false; /* process has terminated in the meantime */
    }
    if (mFile.read(mBuffer, sizeof(mBuffer)) <= 0) { //-1 indicates nothing read
        mFile.close();
        return false;
    }
    mFile.close();

    int current_word = 0; // count from 0
    char *word = mBuffer;
    while (current_word < 6 && word[0] != 0) {
        if (word[0] == ' ') {
            qlonglong number = atoll(word + 1);
            switch (current_word++) {
            case 0: // rchar - characters read
                process->setIoCharactersRead(number);
                break;
            case 1: // wchar - characters written
                process->setIoCharactersWritten(number);
                break;
            case 2: // syscr - read syscall
                process->setIoReadSyscalls(number);
                break;
            case 3: // syscw - write syscall
                process->setIoWriteSyscalls(number);
                break;
            case 4: // read_bytes - bytes actually read from I/O
                process->setIoCharactersActuallyRead(number);
                break;
            case 5: // write_bytes - bytes actually written to I/O
                process->setIoCharactersActuallyWritten(number);
            default:
                break;
            }
        }
        word++;
    }
    return true;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    bool success = true;
    const QString dir = QLatin1String("/proc/") + QString::number(pid) + QLatin1Char('/');

    if (mUpdateFlags.testFlag(Processes::Smaps) && !d->smapsRunnable) {
        d->smapsRunnable = new ReadProcSmapsRunnable{dir};

        connect(d->smapsRunnable, &ReadProcSmapsRunnable::finished, this, [this, pid](qulonglong pss) {
            Q_EMIT processUpdated(pid, {{Process::VmPSS, pss}});
        });

        QThreadPool::globalInstance()->start(d->smapsRunnable);
    }

    if (!d->readProcStat(dir, process)) {
        success = false;
    }
    if (!d->readProcStatus(dir, process)) {
        success = false;
    }
    if (!d->readProcStatm(dir, process)) {
        success = false;
    }
    if (!d->readProcCmdline(dir, process)) {
        success = false;
    }
    if (!d->readProcCGroup(dir, process)) {
        success = false;
    }
    if (!d->readProcAttr(dir, process)) {
        success = false;
    }
    if (!d->getNiceness(pid, process)) {
        success = false;
    }
    if (mUpdateFlags.testFlag(Processes::IOStatistics) && !d->getIOStatistics(dir, process)) {
        success = false;
    }

    return success;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    if (d->mProcDir == nullptr) {
        return pids; // There's not much we can do without /proc
    }
    struct dirent *entry;
    rewinddir(d->mProcDir);
    while ((entry = readdir(d->mProcDir))) {
        if (entry->d_name[0] >= '0' && entry->d_name[0] <= '9') {
            pids.insert(atol(entry->d_name));
        }
    }
    return pids;
}

Processes::Error ProcessesLocal::sendSignal(long pid, int sig)
{
    errno = 0;
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
    if (kill((pid_t)pid, sig)) {
        switch (errno) {
        case ESRCH:
            return Processes::ProcessDoesNotExistOrZombie;
        case EINVAL:
            return Processes::InvalidParameter;
        case EPERM:
            return Processes::InsufficientPermissions;
        }
        // Kill failed
        return Processes::Unknown;
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setNiceness(long pid, int priority)
{
    errno = 0;
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
    auto error = [] {
        switch (errno) {
        case ESRCH:
        case ENOENT:
            return Processes::ProcessDoesNotExistOrZombie;
        case EINVAL:
            return Processes::InvalidParameter;
        case EACCES:
        case EPERM:
            return Processes::InsufficientPermissions;
        default:
            return Processes::Unknown;
        }
    };
    auto threadList{QDir(QString::fromLatin1("/proc/%1/task").arg(pid)).entryList(QDir::NoDotAndDotDot | QDir::Dirs)};
    if (threadList.isEmpty()) {
        return error();
    }
    for (auto entry : threadList) {
        int threadId = entry.toInt();
        if (!threadId) {
            return Processes::InvalidParameter;
        }
        if (setpriority(PRIO_PROCESS, threadId, priority)) {
            return error();
        }
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setScheduler(long pid, int priorityClass, int priority)
{
    errno = 0;
    if (priorityClass == KSysGuard::Process::Other || priorityClass == KSysGuard::Process::Batch || priorityClass == KSysGuard::Process::SchedulerIdle) {
        priority = 0;
    }
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
    struct sched_param params;
    params.sched_priority = priority;
    int policy;
    switch (priorityClass) {
    case (KSysGuard::Process::Other):
        policy = SCHED_OTHER;
        break;
    case (KSysGuard::Process::RoundRobin):
        policy = SCHED_RR;
        break;
    case (KSysGuard::Process::Fifo):
        policy = SCHED_FIFO;
        break;
#ifdef SCHED_IDLE
    case (KSysGuard::Process::SchedulerIdle):
        policy = SCHED_IDLE;
        break;
#endif
#ifdef SCHED_BATCH
    case (KSysGuard::Process::Batch):
        policy = SCHED_BATCH;
        break;
#endif
    default:
        return Processes::NotSupported;
    }

    auto error = [] {
        switch (errno) {
        case ESRCH:
        case ENOENT:
            return Processes::ProcessDoesNotExistOrZombie;
        case EINVAL:
            return Processes::InvalidParameter;
        case EPERM:
            return Processes::InsufficientPermissions;
        default:
            return Processes::Unknown;
        }
    };
    auto threadList{QDir(QString::fromLatin1("/proc/%1/task").arg(pid)).entryList(QDir::NoDotAndDotDot | QDir::Dirs)};
    if (threadList.isEmpty()) {
        return error();
    }
    for (auto entry : threadList) {
        int threadId = entry.toInt();
        if (!threadId) {
            return Processes::InvalidParameter;
        }
        if (sched_setscheduler(threadId, policy, &params) != 0) {
            return error();
        }
    }
    return Processes::NoError;
}

Processes::Error ProcessesLocal::setIoNiceness(long pid, int priorityClass, int priority)
{
    errno = 0;
    if (pid <= 0) {
        return Processes::InvalidPid;
    }
#ifdef HAVE_IONICE
    if (ioprio_set(IOPRIO_WHO_PROCESS, pid, priority | priorityClass << IOPRIO_CLASS_SHIFT) == -1) {
        // set io niceness failed
        switch (errno) {
        case ESRCH:
            return Processes::ProcessDoesNotExistOrZombie;
            break;
        case EINVAL:
            return Processes::InvalidParameter;
        case EPERM:
            return Processes::InsufficientPermissions;
        }
        return Processes::Unknown;
    }
    return Processes::NoError;
#else
    return Processes::NotSupported;
#endif
}

bool ProcessesLocal::supportsIoNiceness()
{
#ifdef HAVE_IONICE
    return true;
#else
    return false;
#endif
}

long long ProcessesLocal::totalPhysicalMemory()
{
    // Try to get the memory via sysconf.  Note the cast to long long to try to avoid a long overflow
    // Should we use sysconf(_SC_PAGESIZE)  or getpagesize()  ?
#ifdef _SC_PHYS_PAGES
    return ((long long)sysconf(_SC_PHYS_PAGES)) * (sysconf(_SC_PAGESIZE) / 1024);
#else
    // This is backup code in case this is not defined.  It should never fail on a linux system.

    d->mFile.setFileName("/proc/meminfo");
    if (!d->mFile.open(QIODevice::ReadOnly)) {
        return 0;
    }

    int size;
    while ((size = d->mFile.readLine(d->mBuffer, sizeof(d->mBuffer))) > 0) { //-1 indicates an error
        switch (d->mBuffer[0]) {
        case 'M':
            if ((unsigned int)size > sizeof("MemTotal:") && qstrncmp(d->mBuffer, "MemTotal:", sizeof("MemTotal:") - 1) == 0) {
                d->mFile.close();
                return atoll(d->mBuffer + sizeof("MemTotal:") - 1);
            }
        }
    }
    return 0; // Not found.  Probably will never happen
#endif
}
ProcessesLocal::~ProcessesLocal()
{
    delete d;
}

}
