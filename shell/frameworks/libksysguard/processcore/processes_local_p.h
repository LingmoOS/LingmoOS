/*
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PROCESSES_LOCAL_H_
#define PROCESSES_LOCAL_H_

#include "processes_base_p.h"
#include <unistd.h> //For sysconf

#include <QSet>

namespace KSysGuard
{
class Process;

/**
 * This is the OS specific code to get process information for the local host.
 */
class ProcessesLocal : public AbstractProcesses
{
public:
    ProcessesLocal();
    ~ProcessesLocal() override;
    QSet<long> getAllPids() override;
    long getParentPid(long pid) override;
    bool updateProcessInfo(long pid, Process *process) override;
    Processes::Error sendSignal(long pid, int sig) override;
    Processes::Error setNiceness(long pid, int priority) override;
    Processes::Error setScheduler(long pid, int priorityClass, int priority) override;
    long long totalPhysicalMemory() override;
    Processes::Error setIoNiceness(long pid, int priorityClass, int priority) override;
    bool supportsIoNiceness() override;
    long numberProcessorCores() override
#ifdef _SC_NPROCESSORS_ONLN
    {
        return sysconf(_SC_NPROCESSORS_ONLN);
    } // Should work on any recent posix system
#else
        ;
#endif
    void updateAllProcesses(Processes::UpdateFlags updateFlags) override
    {
        mUpdateFlags = updateFlags;
        emit processesUpdated();
    } // For local machine, there is no delay

private:
    /**
     * You can use this for whatever data you want.
     * Be careful about preserving state in between getParentPid and updateProcessInfo calls
     * if you chose to do that. getParentPid may be called several times
     * for different pids before the relevant updateProcessInfo calls are made.
     * This is because the tree structure has to be sorted out first.
     */
    class Private;
    Private *d;
    Processes::UpdateFlags mUpdateFlags;
};
}
#endif
