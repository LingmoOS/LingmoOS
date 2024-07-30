/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROCESSES_H_
#define PROCESSES_H_

#include "process.h"
#include <QHash>
#include <QObject>
#include <QVariant>

#include "processcore_export.h"

namespace KSysGuard
{
/**
 * This class retrieves the processes currently running in an OS independent way.
 *
 * To use, do something like:
 *
 * \code
 *   #include "processes.h>
 *   #include "process.h>
 *
 *   KSysGuard::Processes *processes = new KSysGuard::Processes()
 *   QHash<long, Process *> processlist = processes->getProcesses();
 *   foreach( Process * process, processlist) {
 *     kDebug() << "Process with pid " << process->pid() << " is called " << process->name;
 *   }
 *   delete processes;
 *   processes = NULL;
 * \endcode
 *
 * @author John Tapsell <tapsell@kde.org>
 */
#ifdef Q_WS_WIN
class Processes : public QObject
#else
class PROCESSCORE_EXPORT Processes : public QObject
#endif
{
    Q_OBJECT

public:
    Processes(const QString &hostname = QString(), QObject *parent = nullptr);
    ~Processes() override;
    enum UpdateFlag {
        StandardInformation = 1,
        IOStatistics = 2,
        XMemory = 4,
        Smaps = 8,
    };
    Q_DECLARE_FLAGS(UpdateFlags, UpdateFlag)

    enum Error { Unknown = 0, InvalidPid, InvalidParameter, InsufficientPermissions, ProcessDoesNotExistOrZombie, NotSupported, NoError };

    /**
     *  Update all the process information.  After calling this, /proc or equivalent is scanned and
     *  the signals processChanged, etc  are emitted.
     *
     *  Set updateDuration to whatever time period that you update, in milliseconds.
     *  For example, if you update every 2000ms, set this to 2000.  That way it won't update
     *  more often than needed.
     */
    void updateAllProcesses(long updateDurationMS = 0, Processes::UpdateFlags updateFlags = {});

    /**
     *  Return information for one specific process.  Call getProcess(0) to get the
     *  fake process used as the top most parent for all processes.
     *  This doesn't fetch any new information and so returns almost instantly.
     *  Call updateAllProcesses() to actually fetch the process information.
     */
    Process *getProcess(long pid) const;

    /**
     *  Get the error code for the last command that failed.
     */
    Error lastError() const;

    /**
     *  Kill the specified process.  You may not have the privilege to kill the process.
     *  The process may also chose to ignore the command.  Send the SIGKILL signal to kill
     *  the process immediately.  You may lose any unsaved data.
     *
     *  @returns Successful or not in killing the process
     */
    bool killProcess(long pid);

    /**
     *  Send the specified named POSIX signal to the process given.
     *
     *  For example, to indicate for process 324 to STOP do:
     *  \code
     *    #include <signals.h>
     *     ...
     *
     *    KSysGuard::Processes::sendSignal(23, SIGSTOP);
     *  \endcode
     *
     */
    bool sendSignal(long pid, int sig);

    /**
     *  Set the priority for a process.  This is from 19 (very nice, lowest priority) to
     *    -20 (highest priority).  The default value for a process is 0.
     *
     *  @return false if you do not have permission to set the priority
     */
    bool setNiceness(long pid, int priority);

    /**
     *  Set the scheduler for a process.  This is defined according to POSIX.1-2001
     *  See "man sched_setscheduler" for more information.
     *
     *  @p priorityClass One of SCHED_FIFO, SCHED_RR, SCHED_OTHER, and SCHED_BATCH
     *  @p priority Set to 0 for SCHED_OTHER and SCHED_BATCH.  Between 1 and 99 for SCHED_FIFO and SCHED_RR
     *  @return false if you do not have permission to set the priority
     */
    bool setScheduler(long pid, KSysGuard::Process::Scheduler priorityClass, int priority);

    /**
     *  Set the io priority for a process.  This is from 7 (very nice, lowest io priority) to
     *  0 (highest priority).  The default value is determined as: io_nice = (cpu_nice + 20) / 5.
     *
     *  @return false if you do not have permission to set the priority
     */
    bool setIoNiceness(long pid, KSysGuard::Process::IoPriorityClass priorityClass, int priority);

    /**
     *  Returns true if ionice is supported on this system
     */
    bool supportsIoNiceness();

    /**
     *  Return the internal pointer of all the processes.  The order of the processes
     *  is guaranteed to never change.  Call updateAllProcesses() first to actually
     *  update the information.
     */
    const QList<Process *> &getAllProcesses() const;

    /**
     *  Return the number of processes.  Call updateAllProcesses() to actually
     *  update the information.
     *
     *  This is equivalent to getAllProcesses().count()
     */
    int processCount() const;

    /**
     *  Return the total amount of physical memory in KB.  This is fast (just a system call)
     *  Returns 0 on error
     */
    long long totalPhysicalMemory();

    /**
     *  Return the number of processor cores enabled.
     *  (A system can disable processors.  Disabled processors are not counted here).
     *  This is fast (just a system call) */
    long numberProcessorCores();

    /** Update/add process for given pid immediately */
    bool updateOrAddProcess(long pid);

    /** Whether we can get historic process and system data */
    bool isHistoryAvailable() const;

    /** Stop using historical data and use the most recent up-to-date data */
    void useCurrentData();

    /** Return a list of end times and intervals for all the available history */
    QList<QPair<QDateTime, uint>> historiesAvailable() const;

    /** Use historical process data closest to the given date-time.
     *  Returns false if it is outside the range available or there is a problem
     *  getting the data. */
    bool setViewingTime(const QDateTime &when);
    QDateTime viewingTime() const;
    bool loadHistoryFile(const QString &filename);
    QString historyFileName() const;

public Q_SLOTS:
    /** The abstract processes has updated its list of processes */
    void processesUpdated();
    void processUpdated(long pid, const Process::Updates &changes);

Q_SIGNALS:
    /** The data for a process has changed.
     *  if @p onlyTotalCpu is set, only the total cpu usage has been updated.
     *  process->changes  contains a bit field indicating what has changed since the last time this was emitted
     *  for this process
     */
    void processChanged(KSysGuard::Process *process, bool onlyTotalCpu);

    /**
     *  This indicates we are about to add a process in the model.
     *  The process already has the pid, ppid and tree_parent set up.
     */
    void beginAddProcess(KSysGuard::Process *process);

    /**
     *  We have finished inserting a process
     */
    void endAddProcess();
    /**
     *  This indicates we are about to remove a process in the model.  Emit the appropriate signals
     */

    void beginRemoveProcess(KSysGuard::Process *process);

    /**
     *  We have finished removing a process
     */
    void endRemoveProcess();

    /**
     *  This indicates we are about move a process from one parent to another.
     */
    void beginMoveProcess(KSysGuard::Process *process, KSysGuard::Process *new_parent);

    /**
     *  We have finished moving the process
     */
    void endMoveProcess();

    void updated();

protected:
    class Private;
    Private *d;

private:
    inline void deleteProcess(long pid);
    bool updateProcess(Process *process, long ppid);
    bool updateProcessInfo(Process *ps);
    bool addProcess(long pid, long ppid);

Q_SIGNALS:
    /** For a remote machine, we rely on being able to communicate with ksysguardd.
     *  This must be dealt with by the program including this widget.  It must listen to our
     *  'runCommand' signal, and run the given command, with the given id. */
    void runCommand(const QString &command, int id);

public:
    /** For a remote machine, we rely on being able to communicate with ksysguardd.
     *  The programming using this must call this slot when an answer is received from ksysguardd,
     *  in response to a runCommand request.  The id identifies the answer */
    void answerReceived(int id, const QList<QByteArray> &answer);
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Processes::UpdateFlags)
}

#endif
