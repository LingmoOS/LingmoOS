/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"
#include <KLocalizedString>

namespace KSysGuard
{
class ProcessPrivate
{
public:
    long pid;
    long parent_pid;
    Process *parent;
    QString login;
    qlonglong uid;
    qlonglong euid;
    qlonglong suid;
    qlonglong fsuid;
    qlonglong gid;
    qlonglong egid;
    qlonglong sgid;
    qlonglong fsgid;
    qlonglong tracerpid;
    QByteArray tty;
    qlonglong userTime;
    qlonglong sysTime;
    qlonglong startTime;
    int userUsage;
    int sysUsage;
    int totalUserUsage;
    int totalSysUsage;
    unsigned long numChildren;
    int niceLevel;
    Process::Scheduler scheduler;
    Process::IoPriorityClass ioPriorityClass;
    int ioniceLevel;
    qlonglong vmSize;
    qlonglong vmRSS;
    qlonglong vmURSS;
    qlonglong vmPSS;
    qlonglong vmSizeChange;
    qlonglong vmRSSChange;
    qlonglong vmURSSChange;
    qlonglong vmPSSChange;
    unsigned long pixmapBytes;
    bool hasManagedGuiWindow;
    QString name;
    QString command;
    Process::ProcessStatus status;
    qlonglong ioCharactersRead;
    qlonglong ioCharactersWritten;
    qlonglong ioReadSyscalls;
    qlonglong ioWriteSyscalls;
    qlonglong ioCharactersActuallyRead;
    qlonglong ioCharactersActuallyWritten;
    long ioCharactersReadRate;
    long ioCharactersWrittenRate;
    long ioReadSyscallsRate;
    long ioWriteSyscallsRate;
    long ioCharactersActuallyReadRate;
    long ioCharactersActuallyWrittenRate;
    int numThreads;
    QList<Process *> children;
    QElapsedTimer timeKillWasSent;
    int index;
    Process::Changes changes;
    int elapsedTimeMilliSeconds;
    int noNewPrivileges;
    QString cGroup;
    QString macContext;
};

Process::Process()
    : d(new ProcessPrivate())
{
    clear();
}

Process::Process(qlonglong _pid, qlonglong _ppid, Process *_parent)
    : d(new ProcessPrivate())
{
    clear();
    d->pid = _pid;
    d->parent_pid = _ppid;
    d->parent = _parent;
}

Process::~Process()
{
    delete d;
}

QString Process::niceLevelAsString() const
{
    // Just some rough heuristic to map a number to how nice it is
    if (d->niceLevel == 0) {
        return i18nc("Process Niceness", "Normal");
    }
    if (d->niceLevel >= 10) {
        return i18nc("Process Niceness", "Very low priority");
    }
    if (d->niceLevel > 0) {
        return i18nc("Process Niceness", "Low priority");
    }
    if (d->niceLevel <= -10) {
        return i18nc("Process Niceness", "Very high priority");
    }
    if (d->niceLevel < 0) {
        return i18nc("Process Niceness", "High priority");
    }
    return QString(); // impossible;
}

QString Process::ioniceLevelAsString() const
{
    // Just some rough heuristic to map a number to how nice it is
    if (d->ioniceLevel == 4) {
        return i18nc("Process Niceness", "Normal");
    }
    if (d->ioniceLevel >= 6) {
        return i18nc("Process Niceness", "Very low priority");
    }
    if (d->ioniceLevel > 4) {
        return i18nc("Process Niceness", "Low priority");
    }
    if (d->ioniceLevel <= 2) {
        return i18nc("Process Niceness", "Very high priority");
    }
    if (d->ioniceLevel < 4) {
        return i18nc("Process Niceness", "High priority");
    }
    return QString(); // impossible;
}

QString Process::ioPriorityClassAsString() const
{
    switch (d->ioPriorityClass) {
    case None:
        return i18nc("Priority Class", "None");
    case RealTime:
        return i18nc("Priority Class", "Real Time");
    case BestEffort:
        return i18nc("Priority Class", "Best Effort");
    case Idle:
        return i18nc("Priority Class", "Idle");
    default:
        return i18nc("Priority Class", "Unknown");
    }
}

QString Process::translatedStatus() const
{
    switch (d->status) {
    case Running:
        return i18nc("process status", "running");
    case Sleeping:
        return i18nc("process status", "sleeping");
    case DiskSleep:
        return i18nc("process status", "disk sleep");
    case Zombie:
        return i18nc("process status", "zombie");
    case Stopped:
        return i18nc("process status", "stopped");
    case Paging:
        return i18nc("process status", "paging");
    case Ended:
        return i18nc("process status", "finished");
    default:
        return i18nc("process status", "unknown");
    }
}

QString Process::schedulerAsString() const
{
    switch (d->scheduler) {
    case Fifo:
        return i18nc("Scheduler", "FIFO");
    case RoundRobin:
        return i18nc("Scheduler", "Round Robin");
    case Interactive:
        return i18nc("Scheduler", "Interactive");
    case Batch:
        return i18nc("Scheduler", "Batch");
    case SchedulerIdle:
        return i18nc("Scheduler", "Idle");
    default:
        return QString();
    }
}

void Process::clear()
{
    d->pid = -1;
    d->parent_pid = -1;
    d->parent = nullptr;
    d->uid = 0;
    d->gid = -1;
    d->suid = d->euid = d->fsuid = -1;
    d->sgid = d->egid = d->fsgid = -1;
    d->tracerpid = -1;
    d->userTime = 0;
    d->sysTime = 0;
    d->startTime = 0;
    d->userUsage = 0;
    d->sysUsage = 0;
    d->totalUserUsage = 0;
    d->totalSysUsage = 0;
    d->numChildren = 0;
    d->niceLevel = 0;
    d->vmSize = 0;
    d->vmRSS = 0;
    d->vmURSS = 0;
    d->vmPSS = 0;
    d->vmSizeChange = 0;
    d->vmRSSChange = 0;
    d->vmURSSChange = 0;
    d->vmPSSChange = 0;
    d->pixmapBytes = 0;
    d->hasManagedGuiWindow = false;
    d->status = OtherStatus;
    d->ioPriorityClass = None;
    d->ioniceLevel = -1;
    d->scheduler = Other;
    d->ioCharactersRead = 0;
    d->ioCharactersWritten = 0;
    d->ioReadSyscalls = 0;
    d->ioWriteSyscalls = 0;
    d->ioCharactersActuallyRead = 0;
    d->ioCharactersActuallyWritten = 0;
    d->ioCharactersReadRate = 0;
    d->ioCharactersWrittenRate = 0;
    d->ioReadSyscallsRate = 0;
    d->ioWriteSyscallsRate = 0;
    d->ioCharactersActuallyReadRate = 0;
    d->ioCharactersActuallyWrittenRate = 0;

    d->elapsedTimeMilliSeconds = 0;
    d->numThreads = 0;
    d->changes = Process::Nothing;
}

long int Process::pid() const
{
    return d->pid;
}

long int Process::parentPid() const
{
    return d->parent_pid;
}

Process *Process::parent() const
{
    return d->parent;
}

QString Process::login() const
{
    return d->login;
}

qlonglong Process::uid() const
{
    return d->uid;
}

qlonglong Process::euid() const
{
    return d->euid;
}

qlonglong Process::suid() const
{
    return d->suid;
}

qlonglong Process::fsuid() const
{
    return d->fsuid;
}

qlonglong Process::gid() const
{
    return d->gid;
}

qlonglong Process::egid() const
{
    return d->egid;
}

qlonglong Process::sgid() const
{
    return d->sgid;
}

qlonglong Process::fsgid() const
{
    return d->fsgid;
}

qlonglong Process::tracerpid() const
{
    return d->tracerpid;
}

QByteArray Process::tty() const
{
    return d->tty;
}

qlonglong Process::userTime() const
{
    return d->userTime;
}

qlonglong Process::sysTime() const
{
    return d->sysTime;
}

qlonglong Process::startTime() const
{
    return d->startTime;
}

int Process::noNewPrivileges() const
{
    return d->noNewPrivileges;
}

int Process::userUsage() const
{
    return d->userUsage;
}

int Process::sysUsage() const
{
    return d->sysUsage;
}

int &Process::totalUserUsage() const
{
    return d->totalUserUsage;
}

int &Process::totalSysUsage() const
{
    return d->totalSysUsage;
}

long unsigned &Process::numChildren() const
{
    return d->numChildren;
}

int Process::niceLevel() const
{
    return d->niceLevel;
}

Process::Scheduler Process::scheduler() const
{
    return d->scheduler;
}

Process::IoPriorityClass Process::ioPriorityClass() const
{
    return d->ioPriorityClass;
}

int Process::ioniceLevel() const
{
    return d->ioniceLevel;
}

qlonglong Process::vmSize() const
{
    return d->vmSize;
}

qlonglong Process::vmRSS() const
{
    return d->vmRSS;
}

qlonglong Process::vmURSS() const
{
    return d->vmURSS;
}

qlonglong Process::vmPSS() const
{
    return d->vmPSS;
}

qlonglong &Process::vmSizeChange() const
{
    return d->vmSizeChange;
}

qlonglong &Process::vmRSSChange() const
{
    return d->vmRSSChange;
}

qlonglong &Process::vmURSSChange() const
{
    return d->vmURSSChange;
}

qlonglong Process::vmPSSChange() const
{
    return d->vmPSSChange;
}

unsigned long &Process::pixmapBytes() const
{
    return d->pixmapBytes;
}

bool &Process::hasManagedGuiWindow() const
{
    return d->hasManagedGuiWindow;
}

QString Process::name() const
{
    return d->name;
}

QString &Process::command() const
{
    return d->command;
}

Process::ProcessStatus Process::status() const
{
    return d->status;
}

qlonglong Process::ioCharactersRead() const
{
    return d->ioCharactersRead;
}

qlonglong Process::ioCharactersWritten() const
{
    return d->ioCharactersWritten;
}

qlonglong Process::ioReadSyscalls() const
{
    return d->ioReadSyscalls;
}

qlonglong Process::ioWriteSyscalls() const
{
    return d->ioWriteSyscalls;
}

qlonglong Process::ioCharactersActuallyRead() const
{
    return d->ioCharactersActuallyRead;
}

qlonglong Process::ioCharactersActuallyWritten() const
{
    return d->ioCharactersActuallyWritten;
}

long int Process::ioCharactersReadRate() const
{
    return d->ioCharactersReadRate;
}

long int Process::ioCharactersWrittenRate() const
{
    return d->ioCharactersWrittenRate;
}

long int Process::ioReadSyscallsRate() const
{
    return d->ioReadSyscallsRate;
}

long int Process::ioWriteSyscallsRate() const
{
    return d->ioWriteSyscallsRate;
}

long int Process::ioCharactersActuallyReadRate() const
{
    return d->ioCharactersActuallyReadRate;
}

long int Process::ioCharactersActuallyWrittenRate() const
{
    return d->ioCharactersActuallyWrittenRate;
}

int Process::numThreads() const
{
    return d->numThreads;
}

QList<Process *> &Process::children() const
{
    return d->children;
}

QElapsedTimer Process::timeKillWasSent() const
{
    return d->timeKillWasSent;
}

int Process::index() const
{
    return d->index;
}

Process::Changes Process::changes() const
{
    return d->changes;
}

int Process::elapsedTimeMilliSeconds() const
{
    return d->elapsedTimeMilliSeconds;
}

QString Process::cGroup() const
{
    return d->cGroup;
}

QString Process::macContext() const
{
    return d->macContext;
}

void Process::setParentPid(long int parent_pid)
{
    d->parent_pid = parent_pid;
}

void Process::setParent(Process *parent)
{
    d->parent = parent;
}

void Process::setLogin(const QString &login)
{
    if (d->login == login) {
        return;
    }
    d->login = login;
    d->changes |= Process::Login;
}

void Process::setUid(qlonglong uid)
{
    if (d->uid == uid) {
        return;
    }
    d->uid = uid;
    d->changes |= Process::Uids;
}

void Process::setEuid(qlonglong euid)
{
    if (d->euid == euid) {
        return;
    }
    d->euid = euid;
    d->changes |= Process::Uids;
}

void Process::setSuid(qlonglong suid)
{
    if (d->suid == suid) {
        return;
    }
    d->suid = suid;
    d->changes |= Process::Uids;
}

void Process::setFsuid(qlonglong fsuid)
{
    if (d->fsuid == fsuid) {
        return;
    }
    d->fsuid = fsuid;
    d->changes |= Process::Uids;
}

void Process::setGid(qlonglong gid)
{
    if (d->gid == gid) {
        return;
    }
    d->gid = gid;
    d->changes |= Process::Gids;
}

void Process::setEgid(qlonglong egid)
{
    if (d->egid == egid) {
        return;
    }
    d->egid = egid;
    d->changes |= Process::Gids;
}

void Process::setSgid(qlonglong sgid)
{
    if (d->sgid == sgid) {
        return;
    }
    d->sgid = sgid;
    d->changes |= Process::Gids;
}

void Process::setFsgid(qlonglong fsgid)
{
    if (d->fsgid == fsgid) {
        return;
    }
    d->fsgid = fsgid;
    d->changes |= Process::Gids;
}

void Process::setTracerpid(qlonglong tracerpid)
{
    if (d->tracerpid == tracerpid) {
        return;
    }
    d->tracerpid = tracerpid;
    d->changes |= Process::Tracerpid;
}

void Process::setTty(const QByteArray &tty)
{
    if (d->tty == tty) {
        return;
    }
    d->tty = tty;
    d->changes |= Process::Tty;
}

void Process::setUserTime(qlonglong userTime)
{
    d->userTime = userTime;
}

void Process::setSysTime(qlonglong sysTime)
{
    d->sysTime = sysTime;
}

void Process::setStartTime(qlonglong startTime)
{
    d->startTime = startTime;
}

void Process::setNoNewPrivileges(int number)
{
    if (d->noNewPrivileges == number) {
        return;
    }
    d->noNewPrivileges = number;
    d->changes |= Process::Status;
}

void Process::setUserUsage(int _userUsage)
{
    if (d->userUsage == _userUsage) {
        return;
    }
    d->userUsage = _userUsage;
    d->changes |= Process::Usage;
}

void Process::setSysUsage(int _sysUsage)
{
    if (d->sysUsage == _sysUsage) {
        return;
    }
    d->sysUsage = _sysUsage;
    d->changes |= Process::Usage;
}

void Process::setTotalUserUsage(int _totalUserUsage)
{
    if (d->totalUserUsage == _totalUserUsage) {
        return;
    }
    d->totalUserUsage = _totalUserUsage;
    d->changes |= Process::TotalUsage;
}

void Process::setTotalSysUsage(int _totalSysUsage)
{
    if (d->totalSysUsage == _totalSysUsage) {
        return;
    }
    d->totalSysUsage = _totalSysUsage;
    d->changes |= Process::TotalUsage;
}

void Process::setNiceLevel(int _niceLevel)
{
    if (d->niceLevel == _niceLevel) {
        return;
    }
    d->niceLevel = _niceLevel;
    d->changes |= Process::NiceLevels;
}

void Process::setScheduler(Scheduler _scheduler)
{
    if (d->scheduler == _scheduler) {
        return;
    }
    d->scheduler = _scheduler;
    d->changes |= Process::NiceLevels;
}

void Process::setIoPriorityClass(IoPriorityClass _ioPriorityClass)
{
    if (d->ioPriorityClass == _ioPriorityClass) {
        return;
    }
    d->ioPriorityClass = _ioPriorityClass;
    d->changes |= Process::NiceLevels;
}

void Process::setIoniceLevel(int _ioniceLevel)
{
    if (d->ioniceLevel == _ioniceLevel) {
        return;
    }
    d->ioniceLevel = _ioniceLevel;
    d->changes |= Process::NiceLevels;
}

void Process::setVmSize(qlonglong _vmSize)
{
    if (d->vmSizeChange != 0 || d->vmSize != 0) {
        d->vmSizeChange = _vmSize - d->vmSize;
    }
    if (d->vmSize == _vmSize) {
        return;
    }
    d->vmSize = _vmSize;
    d->changes |= Process::VmSize;
}

void Process::setVmRSS(qlonglong _vmRSS)
{
    if (d->vmRSSChange != 0 || d->vmRSS != 0) {
        d->vmRSSChange = _vmRSS - d->vmRSS;
    }
    if (d->vmRSS == _vmRSS) {
        return;
    }
    d->vmRSS = _vmRSS;
    d->changes |= Process::VmRSS;
}

void Process::setVmURSS(qlonglong _vmURSS)
{
    if (d->vmURSSChange != 0 || d->vmURSS != 0) {
        d->vmURSSChange = _vmURSS - d->vmURSS;
    }
    if (d->vmURSS == _vmURSS) {
        return;
    }
    d->vmURSS = _vmURSS;
    d->changes |= Process::VmURSS;
}

void Process::setVmPSS(qlonglong pss)
{
    if (d->vmPSSChange != 0 || d->vmPSS != 0) {
        d->vmPSSChange = pss - d->vmPSS;
    }

    if (d->vmPSS == pss) {
        return;
    }

    d->vmPSS = pss;
    d->changes |= Process::VmPSS;
}

void Process::setName(const QString &_name)
{
    if (d->name == _name) {
        return;
    }
    d->name = _name;
    d->changes |= Process::Name;
}

void Process::setCommand(const QString &_command)
{
    if (d->command == _command) {
        return;
    }
    d->command = _command;
    d->changes |= Process::Command;
}

void Process::setStatus(ProcessStatus _status)
{
    if (d->status == _status) {
        return;
    }
    d->status = _status;
    d->changes |= Process::Status;
}

void Process::setIoCharactersRead(qlonglong number)
{
    if (d->ioCharactersRead == number) {
        return;
    }
    d->ioCharactersRead = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersWritten(qlonglong number)
{
    if (d->ioCharactersWritten == number) {
        return;
    }
    d->ioCharactersWritten = number;
    d->changes |= Process::IO;
}

void Process::setIoReadSyscalls(qlonglong number)
{
    if (d->ioReadSyscalls == number) {
        return;
    }
    d->ioReadSyscalls = number;
    d->changes |= Process::IO;
}

void Process::setIoWriteSyscalls(qlonglong number)
{
    if (d->ioWriteSyscalls == number) {
        return;
    }
    d->ioWriteSyscalls = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersActuallyRead(qlonglong number)
{
    if (d->ioCharactersActuallyRead == number) {
        return;
    }
    d->ioCharactersActuallyRead = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersActuallyWritten(qlonglong number)
{
    if (d->ioCharactersActuallyWritten == number) {
        return;
    }
    d->ioCharactersActuallyWritten = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersReadRate(long number)
{
    if (d->ioCharactersReadRate == number) {
        return;
    }
    d->ioCharactersReadRate = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersWrittenRate(long number)
{
    if (d->ioCharactersWrittenRate == number) {
        return;
    }
    d->ioCharactersWrittenRate = number;
    d->changes |= Process::IO;
}

void Process::setIoReadSyscallsRate(long number)
{
    if (d->ioReadSyscallsRate == number) {
        return;
    }
    d->ioReadSyscallsRate = number;
    d->changes |= Process::IO;
}

void Process::setIoWriteSyscallsRate(long number)
{
    if (d->ioWriteSyscallsRate == number) {
        return;
    }
    d->ioWriteSyscallsRate = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersActuallyReadRate(long number)
{
    if (d->ioCharactersActuallyReadRate == number) {
        return;
    }
    d->ioCharactersActuallyReadRate = number;
    d->changes |= Process::IO;
}

void Process::setIoCharactersActuallyWrittenRate(long number)
{
    if (d->ioCharactersActuallyWrittenRate == number) {
        return;
    }
    d->ioCharactersActuallyWrittenRate = number;
    d->changes |= Process::IO;
}

void Process::setNumThreads(int number)
{
    if (d->numThreads == number) {
        return;
    }
    d->numThreads = number;
    d->changes |= Process::NumThreads;
}

void Process::setIndex(int index)
{
    d->index = index;
}

void Process::setElapsedTimeMilliSeconds(int value)
{
    d->elapsedTimeMilliSeconds = value;
}

void Process::setChanges(KSysGuard::Process::Change changes)
{
    d->changes = changes;
}

void Process::setCGroup(const QString &_cGroup)
{
    if (d->cGroup == _cGroup) {
        return;
    }
    d->cGroup = _cGroup;
    d->changes |= Process::Status;
}

void Process::setMACContext(const QString &_macContext)
{
    if (d->macContext == _macContext) {
        return;
    }
    d->macContext = _macContext;
    d->changes |= Process::Status;
}

}
