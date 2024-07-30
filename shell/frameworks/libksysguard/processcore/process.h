/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>
    SPDX-FileCopyrightText: 2015 Gregor Mi <codestruct@posteo.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROCESS_H
#define PROCESS_H

#include <QElapsedTimer>
#include <QFlags>
#include <QList>
#include <QTime>
#include <QVariant>

#include "processcore_export.h"

namespace KSysGuard
{
class ProcessPrivate; // forward decl d-ptr

class PROCESSCORE_EXPORT Process
{
public:
    enum ProcessStatus { Running, Sleeping, DiskSleep, Zombie, Stopped, Paging, Ended, OtherStatus = 99 };
    enum IoPriorityClass { None, RealTime, BestEffort, Idle };
    enum Scheduler { Other = 0, Fifo, RoundRobin, Batch, SchedulerIdle, Interactive }; ///< Interactive is Solaris only

    Process();
    Process(qlonglong _pid, qlonglong _ppid, Process *_parent);
    virtual ~Process();

    long pid() const; ///< The system's ID for this process.  1 for init.  -1 for our virtual 'parent of init' process used just for convenience.

    long parentPid() const; ///< The system's ID for the parent of this process.  Set to -1 if it has no parent (e.g. 'init' on Linux).
    void setParentPid(long parent_pid);

    /** A guaranteed NON-NULL pointer for all real processes to the parent process except for the fake process with pid -1.
     *  The Parent's pid is the same value as the parent_pid.  The parent process will be also pointed
     *  to by ProcessModel::mPidToProcess to there is no need to worry about mem management in using parent.
     *  For process without a parent (such as 'init' on Linux, parent will point to a (fake) process with pid -1 to simplify things.
     *  For the fake process, this will point to NULL
     */
    Process *parent() const;
    void setParent(Process *parent);

    QList<Process *> &children() const; // REF, make non-ref later! ///< A list of all the direct children that the process has.  Children of children are not
                                        // listed here, so note that children_pids <= numChildren

    unsigned long &numChildren() const; // REF, make non-ref later!

    QString login() const;
    void setLogin(const QString &login); ///< The user login name.  Only used for processes on remote machines.  Otherwise use uid to get the name

    qlonglong uid() const;
    void setUid(qlonglong uid); ///< The user id that the process is running as

    qlonglong euid() const;
    void setEuid(qlonglong euid); ///< The effective user id that the process is running as

    qlonglong suid() const;
    void setSuid(qlonglong suid); ///< The set user id that the process is running as

    qlonglong fsuid() const;
    void setFsuid(qlonglong fsuid); ///< The file system user id that the process is running as.

    qlonglong gid() const;
    void setGid(qlonglong gid); ///< The process group id that the process is running as

    qlonglong egid() const;
    void setEgid(qlonglong egid); ///< The effective group id that the process is running as

    qlonglong sgid() const;
    void setSgid(qlonglong sgid); ///< The set group id that the process is running as

    qlonglong fsgid() const;
    void setFsgid(qlonglong fsgid); ///< The file system group id that the process is running as

    qlonglong tracerpid() const;
    void setTracerpid(qlonglong tracerpid); ///< If this is being debugged, this is the process that is debugging it, or 0 otherwise

    QByteArray tty() const;
    void setTty(const QByteArray &tty); ///< The name of the tty the process owns

    qlonglong userTime() const;
    void setUserTime(qlonglong userTime); ///< The time, in 100ths of a second, spent in total on user calls. -1 if not known

    qlonglong sysTime() const;
    void setSysTime(qlonglong sysTime); ///< The time, in 100ths of a second, spent in total on system calls.  -1 if not known

    /**
     * the value is expressed in clock ticks (since Linux 2.6; we only handle this case) since system boot
     */
    qlonglong startTime() const;
    void
    setStartTime(qlonglong startTime); /// The time the process started after system boot. Since Linux 2.6, the value is expressed in clock ticks. See man proc.

    int userUsage() const;
    void setUserUsage(int userUsage); ///< Percentage (0 to 100).  It might be more than 100% on multiple cpu core systems

    int sysUsage() const;
    void setSysUsage(int sysUsage); ///< Percentage (0 to 100).  It might be more than 100% on multiple cpu core systems

    int &totalUserUsage() const; // REF, make non-ref later!
    void setTotalUserUsage(int totalUserUsage); ///< Percentage (0 to 100) from the sum of itself and all its children recursively.  If there's no children,
                                                ///< it's equal to userUsage.  It might be more than 100% on multiple cpu core systems

    int &totalSysUsage() const; // REF, make non-ref later!
    void setTotalSysUsage(int totalSysUsage); ///< Percentage (0 to 100) from the sum of itself and all its children recursively. If there's no children, it's
                                              ///< equal to sysUsage. It might be more than 100% on multiple cpu core systems

    int niceLevel() const;
    void setNiceLevel(int niceLevel); ///< If Scheduler = Other, niceLevel is the niceness (-20 to 20) of this process.  A lower number means a higher priority.
                                      ///< Otherwise sched priority (1 to 99)

    Scheduler scheduler() const;
    void setScheduler(Scheduler scheduler); ///< The scheduler this process is running in.  See man sched_getscheduler for more info

    IoPriorityClass ioPriorityClass() const;
    void setIoPriorityClass(IoPriorityClass ioPriorityClass); ///< The IO priority class.  See man ionice for detailed information.

    int ioniceLevel() const;
    void setIoniceLevel(int ioniceLevel); ///< IO Niceness (0 to 7) of this process.  A lower number means a higher io priority.  -1 if not known or not
                                          ///< applicable because ioPriorityClass is Idle or None

    qlonglong vmSize() const;
    void setVmSize(qlonglong vmSize); ///< Virtual memory size in KiloBytes, including memory used, mmap'ed files, graphics memory etc,

    qlonglong vmRSS() const;
    void setVmRSS(qlonglong vmRSS); ///< Physical memory used by the process and its shared libraries.  If the process and libraries are swapped to disk, this
                                    ///< could be as low as 0

    qlonglong vmURSS() const;
    void setVmURSS(qlonglong vmURSS); ///< Physical memory used only by the process, and not counting the code for shared libraries. Set to -1 if unknown

    qlonglong vmPSS() const;
    void setVmPSS(qlonglong vmPSS); ///< Proportional set size, the amount of private physical memory used by the process + the amount of shared memory used
                                    ///< divided over the number of processes using it.

    QString name() const;
    void setName(const QString &name); ///< The name (e.g. "ksysguard", "konversation", "init")

    QString &command() const; // REF, make non-ref later!
    void setCommand(const QString &command); ///< The command the process was launched with

    ProcessStatus status() const;
    void setStatus(ProcessStatus status); ///< Whether the process is running/sleeping/etc

    qlonglong ioCharactersRead() const;
    void setIoCharactersRead(qlonglong number); ///< The number of bytes which this task has caused to be read from storage

    qlonglong ioCharactersWritten() const;
    void setIoCharactersWritten(qlonglong number); ///< The number of bytes which this task has caused, or shall cause to be written to disk.

    qlonglong ioReadSyscalls() const;
    void setIoReadSyscalls(qlonglong number); ///< Number of read I/O operations, i.e. syscalls like read() and pread().

    qlonglong ioWriteSyscalls() const;
    void setIoWriteSyscalls(qlonglong number); ///< Number of write I/O operations, i.e. syscalls like write() and pwrite().

    qlonglong ioCharactersActuallyRead() const;
    void setIoCharactersActuallyRead(qlonglong number); ///< Number of bytes which this process really did cause to be fetched from the storage layer.

    qlonglong ioCharactersActuallyWritten() const;
    void setIoCharactersActuallyWritten(qlonglong number); ///< Attempt to count the number of bytes which this process caused to be sent to the storage layer.

    long ioCharactersReadRate() const;
    void setIoCharactersReadRate(long number); ///< The rate, in bytes per second, which this task has caused to be read from storage

    long ioCharactersWrittenRate() const;
    void setIoCharactersWrittenRate(long number); ///< The rate, in bytes per second, which this task has caused, or shall cause to be written to disk.

    long ioReadSyscallsRate() const;
    void setIoReadSyscallsRate(long number); ///< Number of read I/O operations per second, i.e. syscalls like read() and pread().

    long ioWriteSyscallsRate() const;
    void setIoWriteSyscallsRate(long number); ///< Number of write I/O operations per second, i.e. syscalls like write() and pwrite().

    long ioCharactersActuallyReadRate() const;
    void setIoCharactersActuallyReadRate(long number); ///< Number of bytes per second which this process really did cause to be fetched from the storage layer.

    long ioCharactersActuallyWrittenRate() const;
    void setIoCharactersActuallyWrittenRate(
        long number); ///< Attempt to count the number of bytes per second which this process caused to be sent to the storage layer.

    int numThreads() const; ///< Number of threads that this process has, including the main one.  0 if not known
    void setNumThreads(int number); ///< The number of threads that this process has, including this process.

    int noNewPrivileges() const;
    void setNoNewPrivileges(int number); ///< Linux process flag NoNewPrivileges

    int index() const; ///< Each process has a parent process.  Each sibling has a unique number to identify it under that parent.  This is that number.
    void setIndex(int index);

    qlonglong &vmSizeChange() const; // REF, make non-ref later!  ///< The change in vmSize since last update, in KiB

    qlonglong &vmRSSChange() const; // REF, make non-ref later!   ///< The change in vmRSS since last update, in KiB

    qlonglong &vmURSSChange() const; // REF, make non-ref later!  ///< The change in vmURSS since last update, in KiB

    qlonglong vmPSSChange() const; ///< The change in vmPSS since last update, in KiB.

    unsigned long &pixmapBytes() const; // REF, make non-ref later! ///< The number of bytes used for pixmaps/images and not counted by vmRSS or vmURSS

    bool &hasManagedGuiWindow() const; // REF, make non-ref later!

    QElapsedTimer
    timeKillWasSent() const; ///< This is usually a NULL time.  When trying to kill a process, this is the time that the kill signal was sent to the process.

    QString translatedStatus() const; ///< Returns a translated string of the status. e.g. "Running" etc

    QString niceLevelAsString() const; ///< Returns a simple translated string of the nice priority.  e.g. "Normal", "High", etc

    QString ioniceLevelAsString() const; ///< Returns a simple translated string of the io nice priority.  e.g. "Normal", "High", etc

    QString ioPriorityClassAsString() const; ///< Returns a translated string of the io nice class.  i.e. "None", "Real Time", "Best Effort", "Idle"

    QString schedulerAsString() const; ///< Returns a translated string of the scheduler class.  e.g. "FIFO", "Round Robin", "Batch"

    QString cGroup() const;
    void setCGroup(const QString &cGroup); ///< Linux Control Group (cgroup)

    QString macContext() const;
    void setMACContext(const QString &macContext); ///< Mandatory Access Control (SELinux or AppArmor) Context

    /** This is the number of 1/1000ths of a second since this
     *  particular process was last updated compared to when all the processes
     *  were updated. The purpose is to allow a more fine tracking of the time
     *  a process has been running for.
     *
     *  This is updated in processes.cpp and so shouldn't be touched by the
     *  OS dependent classes.
     */
    int elapsedTimeMilliSeconds() const;
    void setElapsedTimeMilliSeconds(int value);

    /** An enum to keep track of what changed since the last update.  Note that we
     * the maximum we can use is 0x4000, so some of the enums represent multiple variables
     */
    enum Change {
        Nothing = 0x0,
        Uids = 0x1,
        Gids = 0x2,
        Tracerpid = 0x4,
        Tty = 0x8,
        Usage = 0x10,
        TotalUsage = 0x20,
        NiceLevels = 0x40,
        VmSize = 0x80,
        VmRSS = 0x100,
        VmURSS = 0x200,
        Name = 0x400,
        Command = 0x800,
        Status = 0x1000,
        Login = 0x2000,
        IO = 0x4000,
        NumThreads = 0x8000,
        VmPSS = 0x10000,
    };
    Q_DECLARE_FLAGS(Changes, Change)

    Changes changes() const; /**< A QFlags representing what has changed */
    void setChanges(Change changes);

    using Updates = QList<QPair<Change, QVariant>>;

private:
    void clear();

private:
    ProcessPrivate *const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(Process::Changes)
}

Q_DECLARE_METATYPE(KSysGuard::Process::Updates)

#endif
