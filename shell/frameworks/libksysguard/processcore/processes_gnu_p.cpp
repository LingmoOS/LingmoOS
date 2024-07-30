/*
    SPDX-FileCopyrightText: 2009 Pino Toscano <pino@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "process.h"
#include "processes_local_p.h"

#ifdef __GNUC__
#warning Totally bogus ProcessesLocal implementation
#endif

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
};

ProcessesLocal::ProcessesLocal()
    : d(0)
{
}

ProcessesLocal::~ProcessesLocal()
{
    delete d;
}

long ProcessesLocal::getParentPid(long pid)
{
    long ppid = -1;
    return ppid;
}

bool ProcessesLocal::updateProcessInfo(long pid, Process *process)
{
    return false;
}

QSet<long> ProcessesLocal::getAllPids()
{
    QSet<long> pids;
    return pids;
}

Processes::Error ProcessesLocal::sendSignal(long pid, int sig)
{
    return Processes::NotSupported;
}

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
    return Processes::NotSupported;
}

bool ProcessesLocal::supportsIoNiceness()
{
    return false;
}

long long ProcessesLocal::totalPhysicalMemory()
{
    long long memory = 0;
    return memory;
}

}
