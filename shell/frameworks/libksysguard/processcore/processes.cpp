/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "processes.h"
#include "processes_atop_p.h"
#include "processes_base_p.h"
#include "processes_local_p.h"
#include "processes_remote_p.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QMutableSetIterator>
#include <QSet>
#include <QVariantMap>

// for sysconf
#include <unistd.h>

/* if porting to an OS without signal.h  please #define SIGTERM to something */
#include <signal.h>

namespace KSysGuard
{
class Q_DECL_HIDDEN Processes::Private
{
public:
    Private(Processes *q_ptr)
    {
        mFakeProcess.setParent(&mFakeProcess);
        mAbstractProcesses = nullptr;
        mHistoricProcesses = nullptr;
        mIsLocalHost = true;
        mProcesses.insert(-1, &mFakeProcess);
        mElapsedTimeMilliSeconds = 0;
        mUpdateFlags = {};
        mUsingHistoricalData = false;
        mLastError = Error::NoError;
        q = q_ptr;
    }
    ~Private();
    void markProcessesAsEnded(long pid);

    QSet<long> mToBeProcessed;
    QSet<long> mEndedProcesses; ///< Processes that have finished

    QHash<long, Process *> mProcesses; ///< This must include mFakeProcess at pid -1
    QList<Process *> mListProcesses; ///< A list of the processes.  Does not include mFakeProcesses
    Process mFakeProcess; ///< A fake process with pid -1 just so that even init points to a parent

    AbstractProcesses *mAbstractProcesses; ///< The OS specific code to get the process information
    ProcessesATop *mHistoricProcesses; ///< A way to get historic information about processes
    bool mIsLocalHost; ///< Whether this is localhost or not

    QElapsedTimer mLastUpdated; ///< This is the time we last updated.  Used to calculate cpu usage.
    long mElapsedTimeMilliSeconds; ///< The number of milliseconds  (1000ths of a second) that passed since the last update

    Processes::UpdateFlags mUpdateFlags;
    bool mUsingHistoricalData; ///< Whether to return historical data for updateProcess() etc
    Processes *q;

    Error mLastError;
};

Processes::Private::~Private()
{
    Q_FOREACH (Process *process, mProcesses) {
        if (process != &mFakeProcess) {
            delete process;
        }
    }
    mProcesses.clear();
    mListProcesses.clear();
    delete mAbstractProcesses;
    mAbstractProcesses = nullptr;
    delete mHistoricProcesses;
    mHistoricProcesses = nullptr;
}

Processes::Processes(const QString &host, QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    qRegisterMetaType<KSysGuard::Process::Updates>();

    if (host.isEmpty()) {
        d->mAbstractProcesses = new ProcessesLocal();
    } else {
        ProcessesRemote *remote = new ProcessesRemote(host);
        d->mAbstractProcesses = remote;
        connect(remote, &ProcessesRemote::runCommand, this, &Processes::runCommand);
    }
    d->mIsLocalHost = host.isEmpty();
    connect(d->mAbstractProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
    connect(d->mAbstractProcesses, &AbstractProcesses::processUpdated, this, &Processes::processUpdated);
}

Processes::~Processes()
{
    delete d;
}

Processes::Error Processes::lastError() const
{
    return d->mLastError;
}
Process *Processes::getProcess(long pid) const
{
    return d->mProcesses.value(pid);
}

const QList<Process *> &Processes::getAllProcesses() const
{
    return d->mListProcesses;
}

int Processes::processCount() const
{
    return d->mListProcesses.count();
}

bool Processes::updateProcess(Process *ps, long ppid)
{
    Process *parent = d->mProcesses.value(ppid, &d->mFakeProcess);
    Q_ASSERT(parent); // even init has a non-null parent - the mFakeProcess

    if (ps->parent() != parent) {
        Q_EMIT beginMoveProcess(ps, parent /*new parent*/);
        // Processes has been reparented
        Process *p = ps;
        do {
            p = p->parent();
            p->numChildren() -= (ps->numChildren() + 1);
        } while (p->pid() != -1);
        Q_ASSERT(ps != parent);
        ps->parent()->children().removeAll(ps);
        ps->setParent(parent); // the parent has changed
        parent->children().append(ps);
        p = ps;
        do {
            p = p->parent();
            p->numChildren() += (ps->numChildren() + 1);
        } while (p->pid() != -1);
        Q_EMIT endMoveProcess();
        Q_ASSERT(ps != parent);
        ps->setParent(parent);
    }

    ps->setParentPid(ppid);

    bool success = updateProcessInfo(ps);
    Q_EMIT processChanged(ps, false);

    return success;
}

bool Processes::updateProcessInfo(Process *ps)
{
    // Now we can actually get the process info
    qlonglong oldUserTime = ps->userTime();
    qlonglong oldSysTime = ps->sysTime();

    qlonglong oldIoCharactersRead = 0;
    qlonglong oldIoCharactersWritten = 0;
    qlonglong oldIoReadSyscalls = 0;
    qlonglong oldIoWriteSyscalls = 0;
    qlonglong oldIoCharactersActuallyRead = 0;
    qlonglong oldIoCharactersActuallyWritten = 0;

    if (d->mUpdateFlags.testFlag(Processes::IOStatistics)) {
        oldIoCharactersRead = ps->ioCharactersRead();
        oldIoCharactersWritten = ps->ioCharactersWritten();
        oldIoReadSyscalls = ps->ioReadSyscalls();
        oldIoWriteSyscalls = ps->ioWriteSyscalls();
        oldIoCharactersActuallyRead = ps->ioCharactersActuallyRead();
        oldIoCharactersActuallyWritten = ps->ioCharactersActuallyWritten();
    }

    ps->setChanges(Process::Nothing);
    bool success;
    if (d->mUsingHistoricalData) {
        success = d->mHistoricProcesses->updateProcessInfo(ps->pid(), ps);
    } else {
        success = d->mAbstractProcesses->updateProcessInfo(ps->pid(), ps);
    }

    // Now we have the process info.  Calculate the cpu usage and total cpu usage for itself and all its parents
    if (!d->mUsingHistoricalData && d->mElapsedTimeMilliSeconds != 0) { // Update the user usage and sys usage
#ifndef Q_OS_NETBSD
        /* The elapsed time is the d->mElapsedTimeMilliSeconds
         * (which is of the order 2 seconds or so) plus a small
         * correction where we get the amount of time elapsed since
         * we start processing. This is because the processing itself
         * can take a non-trivial amount of time.  */
        int elapsedTime = ps->elapsedTimeMilliSeconds();
        ps->setElapsedTimeMilliSeconds(d->mLastUpdated.elapsed());
        elapsedTime = ps->elapsedTimeMilliSeconds() - elapsedTime + d->mElapsedTimeMilliSeconds;
        if (elapsedTime > 0) {
            ps->setUserUsage((int)(((ps->userTime() - oldUserTime) * 1000.0) / elapsedTime));
            ps->setSysUsage((int)(((ps->sysTime() - oldSysTime) * 1000.0) / elapsedTime));
        }
#endif

        static auto calculateRate = [](qlonglong current, qlonglong previous, int elapsedTime) {
            if (elapsedTime <= 0 || previous <= 0) {
                return 0.0;
            }
            return (current - previous) * 1000.0 / elapsedTime;
        };

        if (d->mUpdateFlags.testFlag(Processes::IOStatistics)) {
            ps->setIoCharactersReadRate(calculateRate(ps->ioCharactersRead(), oldIoCharactersRead, elapsedTime));
            ps->setIoCharactersWrittenRate(calculateRate(ps->ioCharactersWritten(), oldIoCharactersWritten, elapsedTime));
            ps->setIoReadSyscallsRate(calculateRate(ps->ioReadSyscalls(), oldIoReadSyscalls, elapsedTime));
            ps->setIoWriteSyscallsRate(calculateRate(ps->ioWriteSyscalls(), oldIoWriteSyscalls, elapsedTime));
            ps->setIoCharactersActuallyReadRate(calculateRate(ps->ioCharactersActuallyRead(), oldIoCharactersActuallyRead, elapsedTime));
            ps->setIoCharactersActuallyWrittenRate(calculateRate(ps->ioCharactersActuallyWritten(), oldIoCharactersActuallyWritten, elapsedTime));
        } else {
            ps->setIoCharactersReadRate(0);
            ps->setIoCharactersWrittenRate(0);
            ps->setIoReadSyscallsRate(0);
            ps->setIoWriteSyscallsRate(0);
            ps->setIoCharactersActuallyReadRate(0);
            ps->setIoCharactersActuallyWrittenRate(0);
        }
    }
    if (d->mUsingHistoricalData || d->mElapsedTimeMilliSeconds != 0) {
        ps->setTotalUserUsage(ps->userUsage());
        ps->setTotalSysUsage(ps->sysUsage());
        if (ps->userUsage() != 0 || ps->sysUsage() != 0) {
            Process *p = ps->parent();
            while (p->pid() != -1) {
                p->totalUserUsage() += ps->userUsage();
                p->totalSysUsage() += ps->sysUsage();
                Q_EMIT processChanged(p, true);
                p = p->parent();
            }
        }
    }

    return success;
}

bool Processes::addProcess(long pid, long ppid)
{
    Process *parent = d->mProcesses.value(ppid);
    if (!parent) {
        // Under race conditions, the parent could have already quit
        // In this case, attach to top leaf
        parent = &d->mFakeProcess;
        Q_ASSERT(parent); // even init has a non-null parent - the mFakeProcess
    }
    // it's a new process - we need to set it up
    Process *ps = new Process(pid, ppid, parent);

    Q_EMIT beginAddProcess(ps);

    d->mProcesses.insert(pid, ps);

    ps->setIndex(d->mListProcesses.count());
    d->mListProcesses.append(ps);

    ps->parent()->children().append(ps);
    Process *p = ps;
    do {
        Q_ASSERT(p);
        p = p->parent();
        p->numChildren()++;
    } while (p->pid() != -1);
    ps->setParentPid(ppid);

    // Now we can actually get the process info
    bool success = updateProcessInfo(ps);
    Q_EMIT endAddProcess();
    return success;
}

bool Processes::updateOrAddProcess(long pid)
{
    long ppid;
    if (d->mUsingHistoricalData) {
        ppid = d->mHistoricProcesses->getParentPid(pid);
    } else {
        ppid = d->mAbstractProcesses->getParentPid(pid);
    }

    if (ppid == pid) {
        // Shouldn't ever happen
        ppid = -1;
    }

    if (d->mToBeProcessed.contains(ppid)) {
        // Make sure that we update the parent before we update this one.  Just makes things a bit easier.
        d->mToBeProcessed.remove(ppid);
        updateOrAddProcess(ppid);
    }

    Process *ps = d->mProcesses.value(pid);
    if (!ps) {
        return addProcess(pid, ppid);
    } else {
        return updateProcess(ps, ppid);
    }
}

void Processes::updateAllProcesses(long updateDurationMS, Processes::UpdateFlags updateFlags)
{
    d->mUpdateFlags = updateFlags;

    if (d->mUsingHistoricalData || d->mLastUpdated.elapsed() >= updateDurationMS || !d->mLastUpdated.isValid()) {
        d->mElapsedTimeMilliSeconds = d->mLastUpdated.restart();
        if (d->mUsingHistoricalData) {
            d->mHistoricProcesses->updateAllProcesses(d->mUpdateFlags);
        } else {
            d->mAbstractProcesses->updateAllProcesses(d->mUpdateFlags); // For a local machine, this will directly call Processes::processesUpdated()
        }
    }
}

void Processes::processesUpdated()
{
    // First really delete any processes that ended last time
    long pid;
    {
        QSetIterator<long> i(d->mEndedProcesses);
        while (i.hasNext()) {
            pid = i.next();
            deleteProcess(pid);
        }
    }

    if (d->mUsingHistoricalData) {
        d->mToBeProcessed = d->mHistoricProcesses->getAllPids();
    } else {
        d->mToBeProcessed = d->mAbstractProcesses->getAllPids();
    }

    QSet<long> endedProcesses;
    for (Process *p : d->mListProcesses) {
        if (!d->mToBeProcessed.contains(p->pid())) {
            endedProcesses += p->pid();
        }
    }

    {
        QMutableSetIterator<long> i(d->mToBeProcessed);
        while (i.hasNext()) {
            pid = i.next();
            i.remove();
            updateOrAddProcess(pid); // This adds the process or changes an existing one
            i.toFront(); // we can remove entries from this set elsewhere, so our iterator might be invalid.  Reset it back to the start of the set
        }
    }
    {
        QSetIterator<long> i(endedProcesses);
        while (i.hasNext()) {
            // We saw these pids last time, but not this time.  That means we have to mark them for deletion now
            pid = i.next();
            d->markProcessesAsEnded(pid);
        }
        d->mEndedProcesses = endedProcesses;
    }

    Q_EMIT updated();
}

void Processes::processUpdated(long pid, const Process::Updates &changes)
{
    auto process = d->mProcesses.value(pid);
    if (!process) {
        return;
    }

    for (auto entry : changes) {
        switch (entry.first) {
        case Process::VmPSS:
            process->setVmPSS(entry.second.toLongLong());
            break;
        default:
            break;
        }
    }

    Q_EMIT processChanged(process, false);
}

void Processes::Private::markProcessesAsEnded(long pid)
{
    Q_ASSERT(pid >= 0);

    Process *process = mProcesses.value(pid);
    if (!process) {
        return;
    }
    process->setStatus(Process::Ended);
    Q_EMIT q->processChanged(process, false);
}
void Processes::deleteProcess(long pid)
{
    Q_ASSERT(pid >= 0);

    Process *process = d->mProcesses.value(pid);
    if (!process) {
        return;
    }
    Q_FOREACH (Process *it, process->children()) {
        deleteProcess(it->pid());
    }

    Q_EMIT beginRemoveProcess(process);

    d->mProcesses.remove(pid);
    d->mListProcesses.removeAll(process);
    process->parent()->children().removeAll(process);
    Process *p = process;
    do {
        Q_ASSERT(p);
        p = p->parent();
        p->numChildren()--;
    } while (p->pid() != -1);
#ifndef QT_NO_DEBUG
    int i = 0;
#endif
    Q_FOREACH (Process *it, d->mListProcesses) {
        if (it->index() > process->index()) {
            it->setIndex(it->index() - 1);
        }
#ifndef QT_NO_DEBUG
        Q_ASSERT(it->index() == i++);
#endif
    }

    delete process;
    Q_EMIT endRemoveProcess();
}

bool Processes::killProcess(long pid)
{
    return sendSignal(pid, SIGTERM);
}

bool Processes::sendSignal(long pid, int sig)
{
    auto processes = d->mUsingHistoricalData ? d->mHistoricProcesses : d->mAbstractProcesses;
    auto error = processes->sendSignal(pid, sig);
    if (error != NoError) {
        d->mLastError = error;
        return false;
    }
    return true;
}

bool Processes::setNiceness(long pid, int priority)
{
    auto processes = d->mUsingHistoricalData ? d->mHistoricProcesses : d->mAbstractProcesses;
    auto error = processes->setNiceness(pid, priority);
    if (error != NoError) {
        d->mLastError = error;
        return false;
    }
    return true;
}

bool Processes::setScheduler(long pid, KSysGuard::Process::Scheduler priorityClass, int priority)
{
    auto processes = d->mUsingHistoricalData ? d->mHistoricProcesses : d->mAbstractProcesses;
    auto error = processes->setScheduler(pid, priorityClass, priority);
    if (error != NoError) {
        d->mLastError = error;
        return false;
    }
    return true;
}

bool Processes::setIoNiceness(long pid, KSysGuard::Process::IoPriorityClass priorityClass, int priority)
{
    auto processes = d->mUsingHistoricalData ? d->mHistoricProcesses : d->mAbstractProcesses;
    auto error = processes->setIoNiceness(pid, priorityClass, priority);
    if (error != NoError) {
        d->mLastError = error;
        return false;
    }
    return true;
}

bool Processes::supportsIoNiceness()
{
    if (d->mUsingHistoricalData) {
        return d->mHistoricProcesses->supportsIoNiceness();
    }
    return d->mAbstractProcesses->supportsIoNiceness();
}

long long Processes::totalPhysicalMemory()
{
    return d->mAbstractProcesses->totalPhysicalMemory();
}

long Processes::numberProcessorCores()
{
    return d->mAbstractProcesses->numberProcessorCores();
}

void Processes::answerReceived(int id, const QList<QByteArray> &answer)
{
    KSysGuard::ProcessesRemote *processes = qobject_cast<KSysGuard::ProcessesRemote *>(d->mAbstractProcesses);
    if (processes) {
        processes->answerReceived(id, answer);
    }
}

QList<QPair<QDateTime, uint>> Processes::historiesAvailable() const
{
    if (!d->mIsLocalHost) {
        return QList<QPair<QDateTime, uint>>();
    }
    if (!d->mHistoricProcesses) {
        d->mHistoricProcesses = new ProcessesATop();
    }

    return d->mHistoricProcesses->historiesAvailable();
}

void Processes::useCurrentData()
{
    if (d->mUsingHistoricalData) {
        delete d->mHistoricProcesses;
        d->mHistoricProcesses = nullptr;
        connect(d->mAbstractProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
        d->mUsingHistoricalData = false;
    }
}

bool Processes::setViewingTime(const QDateTime &when)
{
    if (!d->mIsLocalHost) {
        d->mLastError = NotSupported;
        return false;
    }
    if (!d->mUsingHistoricalData) {
        if (!d->mHistoricProcesses)
            d->mHistoricProcesses = new ProcessesATop();
        disconnect(d->mAbstractProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
        connect(d->mHistoricProcesses, &AbstractProcesses::processesUpdated, this, &Processes::processesUpdated);
        d->mUsingHistoricalData = true;
    }
    return d->mHistoricProcesses->setViewingTime(when);
}

bool Processes::loadHistoryFile(const QString &filename)
{
    if (!d->mIsLocalHost) {
        d->mLastError = NotSupported;
        return false;
    }
    if (!d->mHistoricProcesses) {
        d->mHistoricProcesses = new ProcessesATop(false);
    }

    return d->mHistoricProcesses->loadHistoryFile(filename);
}

QString Processes::historyFileName() const
{
    if (!d->mIsLocalHost || !d->mHistoricProcesses) {
        return QString();
    }
    return d->mHistoricProcesses->historyFileName();
}
QDateTime Processes::viewingTime() const
{
    if (!d->mIsLocalHost || !d->mHistoricProcesses) {
        return QDateTime();
    }
    return d->mHistoricProcesses->viewingTime();
}

bool Processes::isHistoryAvailable() const
{
    if (!d->mIsLocalHost) {
        return false;
    }
    if (!d->mHistoricProcesses) {
        d->mHistoricProcesses = new ProcessesATop();
    }

    return d->mHistoricProcesses->isHistoryAvailable();
}

}
